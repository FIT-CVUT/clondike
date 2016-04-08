require_relative "directorApi"

# Class, that interacts with the kernel netlink api
#
# Npm callbacks are processed by registered handlers. If first handler does not
# return any value, we pass the request to the next... if not handler returns
# any result DO_NOT_MIGRATE is returned
class NetlinkConnector
  def initialize (membershipManager, trustManagement, cql3Driver=nil)
    @cql3Driver = cql3Driver
    @trustManagement = trustManagement
    @membershipManager = membershipManager
    @npmHandlers = []
    @exitHandlers = []
    @forkHandlers = []
    @userMessageHandlers = []
    @immigrationHandlers = []
    @immigrationConfirmedHandlers = []
    @migrationFailedHandlers = []
    @migratedHomeHandlers = []
  end

  # Registers a netlink connector instance to native handler
  def self.register(instance)
    begin
      DirectorNetlinkApi.instance.registerNpmCallback(instance, :connectorNpmCallbackFunction)
      DirectorNetlinkApi.instance.registerNpmFullCallback(instance, :connectorNpmFullCallbackFunction)
      DirectorNetlinkApi.instance.registerNodeConnectedCallback(instance, :connectorNodeConnectedCallbackFunction)
      DirectorNetlinkApi.instance.registerNodeDisconnectedCallback(instance, :connectorNodeDisconnectedCallbackFunction)
      DirectorNetlinkApi.instance.registerTaskExittedCallback(instance, :connectorTaskExittedCallbackFunction)
      DirectorNetlinkApi.instance.registerTaskForkedCallback(instance, :connectorTaskForkedCallbackFunction)
      DirectorNetlinkApi.instance.registerImmigrateRequestCallback(instance, :connectorImmigrationRequestCallbackFunction)
      DirectorNetlinkApi.instance.registerImmigrationConfirmedCallback(instance, :connectorImmigrationConfirmedCallbackFunction)
      DirectorNetlinkApi.instance.registerEmigrationFailedCallback(instance, :connectorEmigrationFailedCallbackFunction)
      DirectorNetlinkApi.instance.registerMigratedHomeCallback(instance, :connectorMigratedHomeCallbackFunction)
      DirectorNetlinkApi.instance.registerUserMessageReceivedCallback(instance, :connectorUserMessageReceivedCallbackFunction)
    rescue => err
      puts "#{err.backtrace.join("\n")}"
      raise "Failed to initialize netlink API!!!"
    end
  end

  # Adds a new listener on npm events. Registered as a last listener on events
  def pushNpmHandlers(handler)
    @npmHandlers << handler;
  end

  def connectorNpmCallbackFunction (pid, uid, name, is_guest, jiffies, rusage)
    result = nil
    @npmHandlers.each do |handler|
      result = handler.onExec(pid, uid, name, is_guest, jiffies, nil, nil, rusage)
      break if result
    end
    result = [DirectorNetlinkApi::DO_NOT_MIGRATE] if !result
    $log.info("(Result #{result[0]} target #{result[1]}) for #{name}") if ( result[0] == DirectorNetlinkApi::MIGRATE_BACK || result[0] == DirectorNetlinkApi::MIGRATE)
    result
  end

  def connectorNpmFullCallbackFunction (pid, uid, name, is_guest, jiffies, args, envp)
    result = nil
    @npmHandlers.each do |handler|
      result = handler.onExec(pid, uid, name, is_guest, jiffies, args, envp)
      break if result
    end
    result = [DirectorNetlinkApi::DO_NOT_MIGRATE] if !result
    $log.info("#{result[0]} for #{name}")
    @cql3Driver.createRecord("EMIGRATE", "#{name}:#{pid}:#{jiffies}", @trustManagement.localIdentity.publicKey, @trustManagement.getKey(result[1]), 0, Time.now)
    result
  end

  def pushImmigrationHandler(handler)
    @immigrationHandlers << handler;
  end

  # Pridat do teto zpravy pid pro Cassandra
  def connectorImmigrationRequestCallbackFunction(uid, pid, slotIndex, name, jiffies)
    $log.info("Immigration request for process #{pid} #{name} #{jiffies}")
    result = true
    node = @membershipManager.detachedManagers[slotIndex].coreNode
    @immigrationHandlers.each do |handler|
      result = result && handler.onImmigrationRequest(node, name)
      break if !result
    end
    
    if result
      @cql3Driver.createRecord("IMMIGRATION_REQUEST", "#{name}:#{pid}:#{jiffies}", node.nodeId, @trustManagement.localIdentity.publicKey, 0, Time.now)
    else
      $log.info("Immigration request for process #{name} REJECTED!")
      @cql3Driver.createRecord("IMMIGRATION_REQUEST", "#{name}:#{pid}:#{jiffies}", node.nodeId, @trustManagement.localIdentity.publicKey, 1, Time.now)
    end

    result
  end

  def pushImmigrationConfirmedHandler(handler)
    @immigrationConfirmedHandlers << handler;
  end

  def connectorImmigrationConfirmedCallbackFunction(uid, slotIndex, name, localPid, remotePid, jiffies)
    $log.info("Immigration of process #{name} (#{localPid}, #{remotePid}, #{jiffies}) confirmed.")
    @cql3Driver.createRecord("IMMIGRATION_CONFIRMED", "#{name}:#{remotePid}:#{jiffies}", @membershipManager.detachedManagers[slotIndex].coreNode.nodeId, @trustManagement.localIdentity.publicKey, 0, Time.now)
    @immigrationConfirmedHandlers.each do |handler|
      node = @membershipManager.detachedManagers[slotIndex].coreNode
      handler.onImmigrationConfirmed(node, name, localPid, remotePid)
    end
  end

  def connectorNodeConnectedCallbackFunction (address, slotIndex, authenticationData)
    $log.info("New node tries to connect: #{address} - #{slotIndex} - #{authenticationData}")
    #if ( @membershipManager.coreManager and @membershipManager.canConnect(authenticationData)) then
    puts "Accept"
    @membershipManager.nodeConnected(address, slotIndex)
    return true
    #else
    #  puts "Reject"
    #  return false
    #end
  end

  def connectorNodeDisconnectedCallbackFunction (slotIndex, slotType, reason)
    # Reason: 0 locally requested, 1 remotely requested... not imporant right now..
    $log.info("Node disconnected: #{slotType}/#{slotIndex} Reason: #{reason == 0 ? 'locally requested' : 'remotely requested'}")

    @membershipManager.nodeDisconnected(ManagerSlot.new(slotType, slotIndex))
  end

  def pushUserMessageHandler(handler)
    @userMessageHandlers << handler;
  end

  def connectorUserMessageReceivedCallbackFunction(slotType, slotIndex, messageLength, message)
    $log.info("Received user message on slot: #{slotType}/#{slotIndex} of length #{messageLength}")

    @userMessageHandlers.each do |handler|
      handler.userMessageReceived(ManagerSlot.new(slotType, slotIndex), messageLength, message)
    end
  end

  def connectorSendUserMessage(managerSlot, messageLength, message)
    $log.debug( "Sending user message #{message.inspect} to slot #{managerSlot}")
    if managerSlot.class == ManagerSlot
     DirectorNetlinkApi.instance.sendUserMessage(managerSlot.slotType, managerSlot.slotIndex, messageLength, message)
    else
      $log.warn "NetlinkConnector.rb: We got managerSlot that IS NOT INSTANCE OF CLASS ManagerSlot, #{managerSlot}"
    end
  end

  # Adds a new listener on exit events. Registered as a last listener on events
  def pushExitHandler(handler)
    @exitHandlers << handler;
  end

  def connectorTaskExittedCallbackFunction(pid, exitCode, rusage)
    #puts "Pid #{pid} exitted with code #{exitCode}"
    @exitHandlers.each do |handler|
      handler.onExit(pid, exitCode, rusage)
    end
  end

  def pushForkHandler(handler)
    @forkHandlers << handler;
  end

  def connectorTaskForkedCallbackFunction(pid, parentPid)
    #$log.debug( "Connector Task Forked")
    @forkHandlers.each do |handler|
      handler.onFork(pid, parentPid)
    end
  end

  def pushEmigrationFailedHandler(handler)
    @migrationFailedHandlers << handler;
  end

  # Pridat jiffies a name pro Cassandra
  def connectorEmigrationFailedCallbackFunction(pid, name, jiffies)
    $log.info("Emigration failed for pid #{pid} #{name} #{jiffies}")
    @cql3Driver.createRecord("EMIGRATE_FAILED", "#{name}:#{pid}:#{jiffies}", @trustManagement.localIdentity.publicKey, nil, 1, Time.now)
    
    @migrationFailedHandlers.each do |handler|
      handler.onEmigrationFailed(pid)
    end
  end

  def pushMigratedHomeHandler(handler)
    @migratedHomeHandlers << handler;
  end

  # Pridat jiffies a name pro Cassandra
  def connectorMigratedHomeCallbackFunction(pid, name, jiffies)
    $log.info("Migrated home: #{pid} #{name} #{jiffies}")

    @migratedHomeHandlers.each do |handler|
      handler.onMigratedHome(pid)
    end
  end

  # Starts the processing thread, that listens on incoming messages from kernel
  def startProcessingThread
    @thread = ExceptionAwareThread.new {
      DirectorNetlinkApi.instance.runProcessingLoop
    }
  end

  # Waits till processing thread terminates
  def waitForProcessingThread
    @thread.join if @thread
  end
end
