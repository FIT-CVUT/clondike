require 'FilesystemConnector'
require 'FilesystemNodeBuilder'
require 'MockNetlinkConnector'
require 'NetlinkConnector'
require 'NodeRepository'
require 'NodeInfoProvider'
require 'NodeInfoConsumer'
require 'Manager'
require 'MembershipManager'
require 'ManagerMonitor'
require 'InformationDistributionStrategy'
require 'LoadBalancer'
require 'RandomBalancingStrategy'
require 'CpuLoadBalancingStrategy'
require 'QuantityLoadBalancingStrategy'
require 'RoundRobinBalancingStrategy'
require 'SignificanceTracingFilter'
require 'ExecDumper'
require 'ExecutionTimeTracer'
require 'TaskRepository'
require 'CacheFSController'
require 'Classifications'
require 'ImmigratedTasksController'
require 'logger'
require 'trust/Identity.rb'
require 'trust/TrustManagement.rb'
require 'cli/CliServer.rb'
require 'trust/TrustCliHandlers.rb'
require 'trust/TrustCliParsers.rb'
require 'trust/CertificatesDistributionStrategy.rb'

require 'measure/MeasurementDirector.rb'
require 'measure/MeasurementPlanParser.rb'
require 'measure/MeasureCliParsers.rb'
require 'measure/MeasureCliHandlers.rb'

require 'Interconnection.rb'
require 'ProcTrace.rb'

require 'TestMakeAcceptLimiter'
require 'TaskNameBasedAcceptLimiter'
require 'MeasurementAcceptLimiter'
require 'LimitersImmigrationController'
require 'ExecutionTimePredictor'
#require "xray/thread_dump_signal_handler"


#This is the main class that is running on the Clondike cluster node
#It controls both Core node and Detached nodes
#This class should be started AFTER core and/or detached node kernel managers are started!
class Director	
        CONF_DIR = "conf"
	LOG_DIR = '/var/log/director'
    
	attr_reader :nodeRepository	
	
	def initialize
		begin Dir.mkdir(Director::LOG_DIR) rescue Errno::EEXIST end
		@filesystemConnector = FilesystemConnector.new
                #acceptLimiter = TestMakeAcceptLimiter.new();
		  		  		
		@immigratedTasksController = ImmigratedTasksController.new(@filesystemConnector, false)
		acceptLimiter = TaskNameBasedAcceptLimiter.new(["make", "mandel", "test-nosleep", "test-sleep", "test-busy"], @immigratedTasksController)
		@measurementLimiter = MeasurementAcceptLimiter.new()
		@immigrationController = LimitersImmigrationController.new([acceptLimiter, @measurementLimiter], @immigratedTasksController)

		@interconnection = Interconnection.new(InterconnectionUDPMessageDispatcher.new(), CONF_DIR)
		initializeTrust()

		#idResolver = IpBasedNodeIdResolver.new
		@idResolver = PublicKeyNodeIdResolver.new(@trustManagement)
                @nodeInfoProvider = NodeInfoProvider.new(@idResolver, @immigratedTasksController)
		@trustManagement.registerIdProvider(@idResolver)
                currentNode = CurrentNode.createCurrentNode(@nodeInfoProvider)

		$log.info("Starting director on node with id #{currentNode.id}")    

		@nodeRepository = NodeRepository.new(currentNode)                                		
                @membershipManager = MembershipManager.new(@filesystemConnector, @nodeRepository, @trustManagement)
		@managerMonitor = ManagerMonitor.new(@interconnection, @membershipManager, @nodeRepository, @filesystemConnector)
                @taskRepository = TaskRepository.new(@nodeRepository, @membershipManager)
		@taskRepository.addExecClassificator(CompileNameClassificator.new())
		# Classify all "mandel" (mandelbrot calc) tasks as long term migrateable tasks
		@taskRepository.addExecClassificator(ExecNameConfigurableClassificator.new("mandel", [MigrateableLongTermTaskClassification.new, MasterTaskClassification.new]))
		@immigratedTasksController.registerTaskRepository(@taskRepository)
		
		predictor = ExecutionTimePredictor.new(CONF_DIR, false)
		
                #balancingStrategy = RandomBalancingStrategy.new(@nodeRepository, @membershipManager)
                #balancingStrategy = CpuLoadBalancingStrategy.new(@nodeRepository, @membershipManager)
		#balancingStrategy = RoundRobinBalancingStrategy.new(@nodeRepository, @membershipManager)		
                balancingStrategy = QuantityLoadBalancingStrategy.new(@nodeRepository, @membershipManager, @taskRepository, predictor)
		balancingStrategy.startDebuggingToFile("LoadBalancer.log")
                @loadBalancer = LoadBalancer.new(balancingStrategy, @taskRepository, @filesystemConnector)		
                @nodeInfoConsumer = NodeInfoConsumer.new(@nodeRepository, @idResolver.getCurrentId)
                @nodeInfoConsumer.registerNewNodeListener(@membershipManager)		
                @informationDistributionStrategy = InformationDistributionStrategy.new(@nodeInfoProvider, @nodeInfoConsumer)
                @nodeInfoProvider.addListener(SignificanceTracingFilter.new(@informationDistributionStrategy))
                @nodeInfoProvider.addListener(currentNode)
                @nodeInfoProvider.addLimiter(acceptLimiter)
		@nodeInfoProvider.addLimiter(@measurementLimiter)
		@nodeInfoProvider.registerLocalTaskCountProvider(balancingStrategy)
                
                #@taskRepository.registerListener(ExecutionTimeTracer.new)
                @taskRepository.registerListener(balancingStrategy)
                @taskRepository.registerListener(acceptLimiter)
		@taskRepository.registerListener(predictor)
                @loadBalancer.registerMigrationListener(@taskRepository)		
                   
		#@cacheFSController = CacheFSController.new(@interconnection)
		
		#initializeMeasurements()
                #initializeCliServer()
	end

	# Starts director processing
	def start
                $log.debug @nodeInfoProvider.getCurrentInfoWithId.to_s
                $log.debug @nodeInfoProvider.getCurrentStaticInfo.to_s            

		#Start kernel listening thread
                begin                  
                    #@netlinkConnector = TimingProxy.new(NetlinkConnector.new(@membershipManager))
		    @netlinkConnector = NetlinkConnector.new(@membershipManager)
		    NetlinkConnector.register(@netlinkConnector)
                rescue => err
                    $log.warn "Creating mock netlink connector as a real connector cannot be created! Problem with creation of the real connector:\n #{err.backtrace.join("\n")}"
                    @netlinkConnector = MockNetlinkConnector.new(@membershipManager)                  
                end		
		procTrace = ProcTrace.new(['/usr/bin/make','/usr/bin/gcc'])
                @netlinkConnector.pushNpmHandlers(@taskRepository)
                @netlinkConnector.pushNpmHandlers(procTrace) if $useProcTrace
                @netlinkConnector.pushNpmHandlers(ExecDumper.new())
                @netlinkConnector.pushNpmHandlers(@loadBalancer)

		@netlinkConnector.pushExitHandler(@taskRepository)
		@netlinkConnector.pushExitHandler(@immigratedTasksController)		
                @netlinkConnector.pushExitHandler(procTrace) if $useProcTrace
		
		@netlinkConnector.pushForkHandler(@immigratedTasksController)
		@netlinkConnector.pushForkHandler(@taskRepository)
		@netlinkConnector.pushForkHandler(@immigratedTasksController)		
		@netlinkConnector.pushForkHandler(procTrace) if $useProcTrace
		
		@netlinkConnector.pushUserMessageHandler(@interconnection)
		
		#@netlinkConnector.pushImmigrationHandler(@cacheFSController)
		@netlinkConnector.pushImmigrationHandler(@immigrationController)
		
		@netlinkConnector.pushImmigrationConfirmedHandler(@immigratedTasksController)
		
		@netlinkConnector.pushMigratedHomeHandler(@taskRepository)
		@netlinkConnector.pushEmigrationFailedHandler(@taskRepository)
		@netlinkConnector.startProcessingThread                                
		
		@loadBalancer.registerMigrationListener(procTrace) if $useProcTrace
                
                #Start notification thread
                @nodeInfoProvider.startNotifyThread
                
                @informationDistributionStrategy.start  
                @interconnection.start(@trustManagement, @netlinkConnector, @idResolver)
		@managerMonitor.start()
	end

	# Waits, till director and all its threads terminates
	def waitForFinished
		@netlinkConnector.waitForProcessingThread
                @nodeInfoProvider.waitForNotifyThread
                @informationDistributionStrategy.waitForFinished
	end
        
private
        def initializeTrust
	      distributionStrategy = BroadcastCertificateDistributionStrategy.new(@interconnection)	
              @identity = Identity.loadIfExists(CONF_DIR, distributionStrategy)
              if ( !@identity )
                  @identity = Identity.create(CONF_DIR, distributionStrategy)      
                  @identity.save()                                    
              end
              @trustManagement = TrustManagement.new(@identity, @interconnection)
        end
	
	def initializeMeasurements()
	      @measurementDirector = MeasurementDirector.new(@nodeInfoProvider.getCurrentId, @interconnection, @measurementLimiter) 
	      @nodeInfoConsumer.registerUpdateListener(@measurementDirector)
	      @nodeInfoProvider.addListener(@measurementDirector)
	end
        
        def initializeCliServer
            parser = CliParser.new            
            registerAllTrustParsers(parser)
	    registerAllMeasurementParsers(parser)
	    registerAllCcfsParsers(parser);
            interpreter = CliInterpreter.new(parser)
            registerAllTrustHandler(@trustManagement, interpreter)
	    registerAllMeasureHandlers(MeasurementPlanParser.new(@nodeRepository), @measurementDirector, interpreter)
	    registerAllCcfsHandlers(@cacheFSController, @interconnection, interpreter)
            server = CliServer.new(interpreter, 4223)
            server.start            
        end
end

$log = Logger.new(STDOUT)
$log.level = Logger::DEBUG;

$useProcTrace = false
    
director = Director.new
director.start
director.waitForFinished
