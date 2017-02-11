# Limiter of immigration requests based deciding based on provided array of limiters.. if any of the limiters denies immigrations, no request is accepted
class LimitersImmigrationController
  def initialize(limiters, immigratedTasksController)
    @limiters = limiters
    @immigratedTasksController = immigratedTasksController
  end

  def onImmigrationRequest(node, execName, localKey, remoteKey)
    cmd = `python clondike/userspace/blockchain/accept_task.py "#{localKey}" "#{remoteKey}"`
    $log.info("Python returns #{$?.success?}")
    if $?.success?
      return true
    else
      return false
    end
    # maximum = 100
    # @limiters.each { |limiter|
    #   limiterMax = limiter.maximumAcceptCount()
    #   maximum = limiterMax if ( limiterMax < maximum )
    # }
    # return maximum > @immigratedTasksController.immigratedTaskCount
  end
end
