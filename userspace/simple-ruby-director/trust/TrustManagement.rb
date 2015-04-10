require 'openssl'
require 'monitor'
require 'trust/Session.rb'
require 'trust/TrustInMemoryDataStore.rb'
require 'trust/TrustMessages.rb'
require 'trust/AuthenticationDispatcher.rb'

# Main class representing a trust management system.
class TrustManagement
  attr_reader :dataStore
  attr_reader :localIdentity
  attr_reader :interconnection
  attr_reader :authenticationDispatcher

  def initialize(localIdentity, interconnection)
    # Identity object of local node
    @localIdentity = localIdentity
    @dataStore = TrustInMemoryDataStore.newDataStore(localIdentity)
    @interconnection = interconnection
    @interconnection.addReceiveHandler(CertificateMessage, CertificateHandler.new(self)) if interconnection

    @authenticationDispatcher = AuthenticationDispatcher.new(localIdentity, interconnection, self)

    # Maps nodeId -> public key of that node as known from periodic broadcast
    @keyMap = {}
    @keyMap.extend(MonitorMixin)

    #@localIdentity.certificateStore.changeListener.addListener(BroadcastCertificateChangeListener.new(@interconnection))
  end

  # Called by a "client" when he wants to establish session with a remote node.
  # This method pre-establishes secret needed to authenticate.
  # If succeeded, returns authentication data to be used by the client
  def authenticate(nodeId, publicKey)
    begin
      proof = @authenticationDispatcher.prepareSession(@idProvider.getCurrentId(), nodeId, publicKey)
      if !proof then
        $log.debug("Authentication negotiation has failed (Peer: #{publicKey})!")
        return nil
      end
      return Session.new(publicKey, "#{proof}")
    rescue TimeoutException
      $log.debug("Authentication request time-outed")
      return nil
    end
  end

  # Called by server to verify if a login attempt is valid
  # If the public key is allowed to connect, create and return a new session
  # otherwise nil is returned
  def verifyAuthentication(authenticationData)
    return nil if !authenticationData
    proof = authenticationData

    requestValid = @authenticationDispatcher.checkNewSessionRequest(proof)
    if ( !requestValid ) then
      $log.debug("Authentication request rejected! (Proof: #{proof})")
      return nil
    end
    #TODO Fill remote session! Get it back from checkNewSessionRequest?
    Session.new(nil, proof)
  end

  def registerKey(nodeId, publicKey)
    @keyMap.synchronize {
      @keyMap[nodeId] = publicKey
    }
  end

  def getKey(nodeId)
    @keyMap.synchronize {
      return @keyMap[nodeId] if @keyMap.has_key?(nodeId)
    }
    return nil
  end

  def convertKeyToPEMString(key)
    return nil if key.nil? 
    return key.to_pem
  end

  def convertPEMStringToKey(pem)
    return nil if pem.nil?
    return OpenSSL::PKey::RSA.new(pem)
  end


  def registerIdProvider(idProvider)
    @idProvider = idProvider
  end

  # Method that checks permission to perform operation on object
  # session param represents a remote node session with this node
  # type params specifies type of object being access (aka "fs" for file system)
  def checkPermissionForSession(session, type, operation, object)
    # TODO: Handle anonymous nodes rules
    remoteEntity = @dataStore.entities[session.remoteNode]
    return false if !remoteEntity

    return checkPermission(remoteEntity, type, operation, object)
  end

  def checkPermission(remoteEntity, type, operation, object)
    permission = parsePermissionLine("#{type} #{operation} #{object}")
    localEntity = @dataStore.entities[@localIdentity.publicKey]
    return checkEntityPermission(localEntity, remoteEntity, permission)
  end

  # Signs an arbitrary data and returns the signature
  def sign(dataToSign)
    return @localIdentity.sign(dataToSign)
  end

  # Check of signature.
  # @param signedBy - Public key of the entity that signed the data
  def verifySignature(data, signature, signedBy)
    res=@localIdentity.verify(signedBy, signature, data)
    #res = signedBy.verify(@digest, signature, data)
    #puts "Verify #{res}"
    return res
  end

  # Returns true or false
  # If a confirmation is just now proccessing, THIS METHOD WILL WAIT for the result of the confirmation
  def isVerified?(nodeId)
    session = getSession(nodeId)
    return false if session.nil?
    session.waitForConfirmationState()
    return (session.confirmed == true ? true : false)
  end

  def getSession(nodeId)
    key = getKey(nodeId).to_pem
    return nil if key.nil?
    cliNegs = @authenticationDispatcher.clientNegotiations
    srvNegs = @authenticationDispatcher.serverNegotiations
    return cliNegs[key] if (cliNegs.has_key?(key) && cliNegs[key].confirmed == true)
    return srvNegs[key] if (srvNegs.has_key?(key) && srvNegs[key].confirmed == true)
    return cliNegs[key] if cliNegs.has_key?(key)
    return srvNegs[key] if srvNegs.has_key?(key)
    return nil
  end

  private

  def checkEntityPermission(localEntity, entity, permission)
    return true if ( checkEntityPermissionNoRecurse(localEntity, entity, permission) )

    localEntity.containedIn.each { |containingEntity|
      #puts "CHECKING CONTAINER ENTITY #{containingEntity}"
      return true if ( checkEntityPermission(containingEntity, entity, permission))
    }

    return false
  end

  def checkEntityPermissionNoRecurse(localEntity, entity, permission)
    grantedPermissions = Set.new
    requestedPermissions = Set.new
    entity.authorizationsWithDelegations.each { |authorization|
      #puts "AUTHORIZATION: #{authorization}"
      # TODO: Detect if the authorization is for one of local groups
      grantedPermissions.merge(authorization.permissions) if ( authorization.target == localEntity)
    }
    requestedPermissions.add(permission)

    return true if ( areAllPermissionImplied(grantedPermissions, requestedPermissions) )

    entity.containedIn.each { |containingEntity|
      #puts "CHECKING CONTAINER ENTITY #{containingEntity}"
      return true if ( checkEntityPermission(localEntity, containingEntity, permission))
    }

    return false
  end
end

class CertificateHandler
  def initialize(trustManagement)
    @trustManagement = trustManagement
  end

  # Handles CertificateMessage
  def handle(message)
    $log.debug("Received certificate.")

    if ( message.certificate.issuer == @trustManagement.localIdentity.publicKey) then
      $log.debug("Ignoring my own certificate.")
      return
    end

    # TODO: Handle persists flag correctly!
    @trustManagement.localIdentity.certificateStore.insertNewCertificate(message.certificate, true)
    # Immediately sents ack.. the cert should be persisted by now..
    #@trustMaganegemt.interconnection.dispatch(message.certificate.issuer, CertificateAckMessage.new(@trustManagement.localIdentity.publicKey, message.certificate.issuer, message.certificate.id))
  end
end

# HACKY certificate listener. Broadcast all certificates to everybody
# TODO: Implement real distribution strategies
class BroadcastCertificateChangeListener
  def initialize(interconnect)
    @interconnect = interconnect
  end

  def becomeValid(certificate)
    message = CertificateMessage.new(certificate)
    # Broadcast to everybody
    $log.debug("Broadcasting message #{@interconnect != nil}")
    @interconnect.dispatch(nil, message) if @interconnect != nil
  end

  def becomeInvalid(certificate)
  end
end

class PublicKeyNodeIdResolver
  def initialize(trustManagement)
    @identity = trustManagement.localIdentity.publicKey.to_pem
  end

  def getCurrentId()
    @identity
  end

  def requiredIdentityClass()
    return String.class
  end
end
