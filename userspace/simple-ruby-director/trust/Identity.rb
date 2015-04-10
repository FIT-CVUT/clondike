require "openssl"
require "base64"
require "trust/Certificates.rb"
require "trust/CertificateStore.rb"
require 'trust/Permission.rb'
require 'PersistentIdSequence.rb'
require 'delegate.rb'
require "set"

class RSAConfig
  BITS=1024
end

# Class representing all identity related aspects of node
class Identity
  # Private key of the node
  attr_reader :privateKey
  # Public key of the node
  attr_reader :publicKey
  # Certificate corresponding to public key
  attr_reader :certificate
  # Store with all certificates known by this identity
  attr_reader :certificateStore
  # Directory where to store all certificates
  attr_reader :directory

  # Loads an identity data from a specified path
  # Returns nil in case identity data cannot be loaded at that path
  def self.loadIfExists(directory, distributionStrategy)
    return nil if ( !identityExists(directory) )
    identity = Identity.new(directory, distributionStrategy, nil, nil, nil)
    identity.load()
    return identity
  end

  # Saves an identity data at a specified path
  def save()
    saveKey(publicKey,"#{@directory}/public.pem")
    saveKey(privateKey,"#{@directory}/private.pem")
    certificate.save("#{@directory}/identity.cert")
    # TODO: Save remaining certs
  end

  def load()
    @publicKey = loadKey("#{directory}/public.pem")
    @privateKey = loadKey("#{directory}/private.pem")
    @certificate = loadCertificate("#{directory}/identity.cert")
    @certificateStore.load()
  end

  # Creates and returns a new identity data
  def self.create(directory, distributionStrategy)
    newKey = RSAKeyTools.generate(RSAConfig::BITS)

    Identity.new(directory, distributionStrategy, newKey, newKey.public_key, NodeCertificate.new(0, newKey.public_key))
  end

  def issueAliasCertificate(nodeAlias, nodePublicKey)
    certificate = AliasCertificate.new(nextSequenceNumber(), @publicKey, nodeAlias, nodePublicKey)
    certificate.sign(privateKey)
    @certificateStore.insertNewCertificate(certificate, true)
    return certificate
  end

  def issueGroupMembershipCertificate(groupName, member)
    certificate = GroupMembershipCertificate.new(nextSequenceNumber(), @publicKey, groupName, member)
    certificate.sign(privateKey)
    @certificateStore.insertNewCertificate(certificate, true)
    return certificate
  end

  # Permissions can be either a set of permissions or a single permission
  def issueAuthorizationCertificate(authorizee, authorizedGroupName, target, targetGroupName, permissions, canDelegate)
    # Wrap permissions into a collection, if required
    if ( !permissions.kind_of?(Set))
      permissionsSet = Set.new
      permissionsSet.add(permissions)
      permissions = permissionsSet
    end

    certificate = AuthorizationCertificate.new(nextSequenceNumber(), @publicKey, authorizee, authorizedGroupName, target, targetGroupName, permissions, canDelegate)
    certificate.sign(privateKey)
    @certificateStore.insertNewCertificate(certificate, true)
    return certificate
  end

  def revokeCertificate(certificateToRevoke)
    #puts "REVOKING #{certificateToRevoke.issuer} .... #{@publicKey}"
    certificate = RevocationCertificate.new(nextSequenceNumber(), @publicKey, certificateToRevoke.issuer, certificateToRevoke.id)
    certificate.sign(privateKey)
    @certificateStore.insertNewCertificate(certificate, true)
    return certificate
  end

  # Should be called whever we recieve a new certificate from some other node
  def receiveCertificate(certificate)
    # TODO: Should we always persist? There are some certificates that shall not be persisted!
    @certificateStore.insertNewCertificate(certificate, true)
  end

  # Signs arbitrary (string) data and returns a signature
  def sign(dataToSign)
    @privateKey.sign(@digest, dataToSign)
  end

  def verify(signedBy, signature, data)
    return signedBy.verify(@digest, signature, data)
  end

  def decrypt(encryptedData)
    @privateKey.private_decrypt(encryptedData)
  end

  protected

  def initialize(directory, distributionStrategy, privateKey, publicKey, certificate)
    @directory = directory
    @privateKey = privateKey
    @publicKey = publicKey
    @certificate = certificate

    @certificateStore = CertificateStore.new(self, distributionStrategy, @directory)

    @digest=OpenSSL::Digest::SHA1.new

    # Sequence used for certificate ID generation
    @sequenceId = PersistentIdSequence.new("#{directory}/seq")
  end

  # Returns true, if identity files exists at specified directory
  def self.identityExists(directory)
    # TODO: We now check presence of sequence only. It'd be better to perform more checks..
    return File.exists?("#{directory}/seq")
  end

  def nextSequenceNumber
    @sequenceId.nextId
  end

  def saveKey(key,file)
    output = File.new(file, "w")
    output.puts key.full_to_s
    output.close
  end

  def loadKey(file)
    OpenSSL::PKey::RSA.new(File.read(file))
  end
end

class RSAKeyTools
  def self.generate(number)
    rsaKey = RSAPublicKey.new(nil)
    rsaKey.__setobj__(OpenSSL::PKey::RSA.new(number))
    return rsaKey
  end

  def self.load(stringKey)
    OpenSSL::PKey::RSA.new(RSAKeyTools.decorateKey(stringKey))
  end

  def self.undecorateKey(key)
    key.to_s.gsub(/-----[ A-Z]*-----/,"").gsub(/\n/,"")
  end

  # Key could be decorated yet or non-decorated
  def self.decorateKey(key)
    res = RSAKeyTools.undecorateKey(key)

    # Fragile assumption: undecorated string of *public* RSAConfig::BITS key has fix length
    if res.length == RSAKeyTools.getPublicKeyLength()
      beginString = RSAKeyTools.getPublicKeyHeader()
      endString = RSAKeyTools.getPublicKeyFooter()
    else
      beginString = RSAKeyTools.getPrivateKeyHeader()
      endString = RSAKeyTools.getPrivateKeyFooter()
    end
    "#{beginString}\n#{RSAKeyTools.splitBase64(res)}\n#{endString}"
  end

  def self.splitBase64(string)
    r = []
    len = 64
    start = 0
    while(start <= string.length) do
      r << string[start...start+len]
      start += len
    end
    return r.join("\n")
  end

  def self.unsplitBase64(string)
    string.gsub(/\n/,"")
  end

  def self.getPublicKeyLength
    @@publicKeyLength = RSAKeyTools.undecorateKey(OpenSSL::PKey::RSA.new(RSAConfig::BITS).public_key).length unless defined? @@publicKeyLength
    return @@publicKeyLength
  end
  def self.getPublicKeyHeader
    @@publicKeyHeader = OpenSSL::PKey::RSA.new(RSAConfig::BITS).public_key.to_s.gsub(/^([- A-Z]*)\n.*/m,'\1') unless defined? @@publicKeyHeader
    return @@publicKeyHeader #=> Ruby1.8.7 "-----BEGIN PUBLIC KEY-----", Ruby2.0 "-----BEGIN RSA PUBLIC KEY-----"
  end
  def self.getPublicKeyFooter
    @@publicKeyFooter = OpenSSL::PKey::RSA.new(RSAConfig::BITS).public_key.to_s.gsub(/.*\n([- A-Z]*)\n/m,'\1') unless defined? @@publicKeyFooter
    return @@publicKeyFooter #=> Ruby1.8.7 "-----END PUBLIC KEY-----", Ruby2.0 "-----END RSA PUBLIC KEY-----"
  end
  def self.getPrivateKeyHeader
    @@privateKeyHeader = OpenSSL::PKey::RSA.new(RSAConfig::BITS).to_s.gsub(/^([- A-Z]*)\n.*/m,'\1') unless defined? @@privateKeyHeader
    return @@privateKeyHeader #=> "-----BEGIN RSA PRIVATE KEY-----"
  end
  def self.getPrivateKeyFooter
    @@privateKeyFooter = OpenSSL::PKey::RSA.new(RSAConfig::BITS).to_s.gsub(/.*\n([- A-Z]*)\n/m,'\1') unless defined? @@privateKeyFooter
    return @@privateKeyFooter #=> "-----END RSA PRIVATE KEY-----"
  end
end

class RSAPublicKey < SimpleDelegator
  def initialize(decoratedKeyText)
    super(decoratedKeyText != nil ? OpenSSL::PKey::RSA.new(RSAKeyTools.decorateKey(decoratedKeyText)) : nil)
  end

  def undecorated_to_s
    res = __getobj__.to_s
    RSAKeyTools.undecorateKey(res)
  end

  def public_key
    RSAPublicKey.new(super.public_key.to_s)
  end

  def marshal_dump()
    return undecorated_to_s
  end

  def marshal_load(var)
    rsaKey = OpenSSL::PKey::RSA.new(RSAKeyTools.decorateKey(var))
    __setobj__(rsaKey)
  end

  def hash()
    return undecorated_to_s.hash
  end

  # Short-cut method with hardcoded digest algorithm
  def verifySignature(signature, data)
    verify(OpenSSL::Digest::SHA1.new, signature, data)
  end

  def ==(other)
    return false if !other.kind_of?(RSAPublicKey)
    return undecorated_to_s == other.undecorated_to_s
  end

  def toShortHashString()
    return OpenSSL::Digest::SHA1.new(undecorated_to_s)
  end

  def full_to_s
    return __getobj__.to_s
  end

  def to_s
    return toShortHashString().to_s
  end

  def eql?(other)
    self == other
  end
end
