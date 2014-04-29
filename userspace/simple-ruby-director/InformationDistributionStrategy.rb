require 'BlockingQueue'

#This class handles propagation and receiving of NodeInfos
class InformationDistributionStrategy
  DISTRIBUTION_PERIOD = 1 # in seconds

  def initialize(nodeInfoProvider, informationConsumer, interconnection)
    @nodeInfoProvider = nodeInfoProvider
    @informationConsumer = informationConsumer
    @interconnection = interconnection
    @interconnection.addReceiveHandler(NodeInfoWithId, NodeInfoWithIdMessageHandler.new(@informationConsumer))
  end

  def start
    @extractThread = ExceptionAwareThread.new() { nodeInfoExtractThread() }
  end

  def waitForFinished
    @extractThread.join
  end

  #Callback method, informing about significant change in a dynamic node info
  def notifyChange(nodeInfo)
    @interconnection.dispatch(nil, nodeInfo)
  end

  private

  #Enqueues dynamic info about the current node to be send (once per second)
  def nodeInfoExtractThread
    while true
      sleep(DISTRIBUTION_PERIOD)
      @interconnection.dispatch(nil, @nodeInfoProvider.getCurrentInfoWithId)
    end
  end
end

class NodeInfoWithIdMessageHandler
  def initialize(informationConsumer)
    @informationConsumer = informationConsumer
  end
  def handle(message)
    @informationConsumer.infoReceived(message)
  end
end
