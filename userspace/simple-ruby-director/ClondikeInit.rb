#!/usr/bin/ruby -w

require 'ConfigurationParser'
require 'Director'
require 'resolv'
require 'logger'
require 'fileutils'

class Clondike
    @localIP = nil
    @simpleRubyDirectorPath = nil

    ###################
    def initialize( configuration )
        @simpleRubyDirectorPath = File.dirname(__FILE__)
        @localIP = getLocalIP( configuration );

        clearCurrentConfig()


        listen( configuration )
        #cmd = "../../scripts/listen.sh #{ip}"
        #$log.debug( "#{cmd}" )
        #`#{cmd}`

       $log.debug( "[OK]\tStarting Clondike complete..\n.................................\n\n" )
    end



    ###################
    def getConfigDirective( configuration, directive, default_value )
        value = configuration.getValue( directive )
        value = default_value if value==nil
        return value
    end

    ###################
    def getLocalIP( configuration )
        interface   = getConfigDirective( configuration, 'interface', 'eth0' )
        command     = "ip -4 addr show | awk '/#{interface}/ { getline; adrs=$2; split(adrs,adr,\"/\"); printf \"%s\", adr[1]; }'"
        $log.debug("#{command}\n")
        localIP     = `#{command}`
        $log.debug("LocalIP: #{localIP}\n")

        # test IP address and if not pass exit with error
        if ( localIP !~ Resolv::IPv4::Regex )
            $log.error( "[ERROR]\tIP address #{localIP} of interface #{interface} was not found or is wrong! Please check the interface.\n" )
            exit 1
        end
        return localIP
    end

    ###################
    def clearCurrentConfig()
        $log.debug( "[..]\tClearing current config" )
        FileUtils.rm_rf( Dir.glob('conf/*') )
        $log.debug( "[OK]\tDeleting current config complete" )
        
        # README file
        File.write( "#{@simpleRubyDirectorPath}/conf/README", 'This directory should be empty before starting Clondike' )

        $log.debug( "[OK]\tClearing current config complete" )
    end

    ###################
    # Mount 9p and proxyfs filesystems
    def mount( configuration, listen_ip )
        # 9p mounting parameters
        $log.debug( "[..]\tCreating mounting files" )
        mounterdir = getConfigDirective( configuration, 'mounterdir', '/clondike/ccn/mounter/' )
        File.write( "#{mounterdir}fs-mount", '9p-global' )
        File.write( "#{mounterdir}fs-mount-device", listen_ip )
        #File.write( "#{mounterdir}fs-mount-options", 'aname=/,uname=root,port=5577' )
        File.write( "#{mounterdir}fs-mount-options", 'aname=/,uname=root,version=9p2000.L' )
        $log.debug( "[OK]\tMounting files created in #{mounterdir}" )

        # Proxyfs
        command = "mount -t proxyfs tcp:#{listen_ip}:1112 /mnt/proxy"
        `#{command}`
    end

    ###################
    def listen( configuration )
        @localIP = getLocalIP( configuration ) if @localIP==nil
        listen_ip   = @localIP
        listen_port = getConfigDirective( configuration, 'listenport', '54321' )
        listen_prot = getConfigDirective( configuration, 'listenprot', 'tcp'   )
        listen_str  = "#{listen_prot}:#{listen_ip}:#{listen_port}"
        $log.debug( "[INFO]\tListenstr: #{listen_str}" )

        # Mount 9p and proxyfs filesystems
        mount( configuration, listen_ip )

        # Listen file
        listen_file = getConfigDirective( configuration, 'listenfile', '/clondike/ccn/listen' )
        if not File.file?( listen_file )
            $log.error( "[ERROR]\tCan't listen on '#{listen_str}', missing listen file '#{listen_file}'" )
            exit 1
        end
        # File.write( listen_file, listen_str ) # Can't write by ruby, have to write by shell
        command = "echo #{listen_str} > #{listen_file}"
        `#{command}`
        $log.info( "[OK]\tStarting listening on #{listen_str}.." )
        $log.debug( "[OK]\tListening complete" )
    end

end

########### Main program #############
begin
    $log = Logger.new("/tmp/simple-ruby-director.log")
#    $log = Logger.new(STDOUT)
    #$log.level = Logger::DEBUG;
    $log.level=Logger::INFO;
    $log.datetime_format = "%Y-%m-%d %H:%M:%S"

    $useProcTrace = false

    configuration = ConfigurationParser.new( File.dirname(__FILE__)+"/clondike.conf" )
    clondike = Clondike.new( configuration )

    # Running director
    director = Director.new( configuration )
    director.start
    $log.debug( "[OK]\tDirector started" )
    director.waitForFinished()

rescue => err
    $log.error "Error in main Director thread: #{err.message} \n#{err.backtrace.join("\n")}"
end






