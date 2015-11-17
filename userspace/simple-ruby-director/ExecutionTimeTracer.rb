require 'logger'

# Listener pluggable to TaskRepository, that can trace execution times of individual tasks
class ExecutionTimeTracer
  def newTask(task)
    # Nothing to be done
  end 

  def taskExit(task, exitCode)
    $taskLog = Logger.new('/var/log/clondike-taskLog')
    $taskLog.level = Logger::DEBUG
    $taskLog.datetime_format = "%Y-%m-%d %H:%M:%S"
    
    #$taskLog.info("------## Class: #{task.class}")

    endTime = Time.now.to_f
    $taskLog.debug("
    Task: #{task.name} (#{task.pid})
        #{task.args.join(' ')}
    Duration:       #{endTime-task.startTime} seconds.
    Exit code:      #{exitCode}.
    Execution node: #{task.executionNode}
    ")
  end 
end

