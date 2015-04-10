#!/usr/bin/ruby

require "directorApi"

#include DirectorNetlinkApi

def npmCallbackFunction(pid, uid, name, is_guest, jiffies, rusage)
	#puts "Hueee we are in callback"
	puts "Test callback called #{pid} name #{name} is_guest #{is_guest}" 
	#args.each { |arg| puts "Arg #{arg}" }
	#envs.each { |env| puts "Env #{env}" }
	binaryName = name.split("/").last
	puts binaryName

	if ( binaryName == "dmesg" )
		[DirectorNetlinkApi::REQUIRE_ENVP]
	else
		[DirectorNetlinkApi::DO_NOT_MIGRATE]
	end
end

def npmFullCallbackFunction(pid, uid, name, is_guest, jiffies, args, envp)
	#puts "Hueee we are in callback"
	puts "Test callback called #{pid} name #{name} is_guest #{is_guest}" 
	args.each { |arg| puts "Arg #{arg}" }
	envp.each { |env| puts "Env #{env}" }

	[DirectorNetlinkApi::DO_NOT_MIGRATE]
end

netlinkThread = Thread.new {
	DirectorNetlinkApi.instance.registerNpmCallback(self,:npmCallbackFunction)
	DirectorNetlinkApi.instance.registerNpmFullCallback(self,:npmFullCallbackFunction)
	DirectorNetlinkApi.instance.runProcessingLoop
}

connectThread = Thread.new {
	#return
	sleep(5)
	
	puts "Zapisuji do /test.txt"
	`echo "tcp:192.168.1.245:54321" > /clondike/pen/connect`
	#`echo "tcp:192.168.96.145:54321" > /test.txt`
	#File.open('/clondike/pen/connect', 'w'){ |file| file.write("tcp:192.168.10.130:54321")}
	puts "Koncim"
}

puts "Waiting for netlink thread end"

netlinkThread.join
connectThread.join
