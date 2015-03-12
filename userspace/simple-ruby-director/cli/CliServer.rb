require 'gserver'
require 'cli/CliInterpreter.rb'

# This class listens on CLI commands and sends them to CliInterpreter
class CliServer < GServer
  def initialize(interpreter, listenPort)
    super(listenPort)
    @interpreter = interpreter
    @audit = true
    @debug = true
  end

  def serve(io)
    loop do
      input = io.gets
      break if !input
      input = input.chop!
      response = @interpreter.interpret(input)
      $log.debug "CliServer: Sending response #{response}"
      io.puts(response)
    end
  end

  def error(detail)
    $log.error "CliServer: #{detail.message} #{detail.backtrace.join("\n")}"
  end
end
