require 'socket'

class String
  def startsWith(prefix)
    self.match(/^#{prefix}/) ? true : false
  end

  def endsWith(sufix)
    self.match(/#{sufix}$/) ? true : false
  end
end

class Hash
  # Returns single element contained in the hash. Raises error if there is different count of elements than 1
  def singleElement
    raise "Invalid element count: #{size}" if ( size != 1 )
    return to_a[0]
  end
end

class ExceptionAwareThread < Thread
  def initialize(*args)
    super do
      begin
        yield(*args)
      rescue => err
        $log.error "Error in thread: #{err.message} \n#{err.backtrace.join("\n")}"
      end
    end
  end
end

class TimeoutException<RuntimeError
  attr_reader :text

  def initialize(text)
    @text = text
  end

  def to_s
    "#{text}"
  end
end

def formattedDuration(startTime, endTime)
  millisDiff = 1000*(endTime.to_f - startTime.to_f)
  millis = millisDiff%1000
  secondsDiff = millisDiff/1000
  seconds = secondsDiff%60
  minutes = secondsDiff/60
  return sprintf("%02d:%02d.%04d", minutes, seconds, millis)
end

def timedExecution(name)
  startTime = Time.now.to_f
  result = yield
  endTime = Time.now.to_f

  $log.debug "Execution of '#{name}' took: #{1000*(endTime - startTime)} ms"
  return result
end

class TimingProxy
  instance_methods.each { |m| undef_method m unless m =~ /(^__|^send$|^object_id$)/ }

  def initialize(object)
    @object = object
  end

  protected
  def method_missing(name, *args, &block)
    timedExecution(name) {
      @object.send(name, *args, &block)
    }
  end
end

def showBacktrace
  begin
    5/0
  rescue => exception
    e = exception.backtrace
    e.shift # => Util.rb:80:in `/'
    e.shift # => Util.rb:80:in `showBacktrace'
    puts e
  end
end
