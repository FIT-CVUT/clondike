require 'BlockingQueue'

#This class handles propagation and receiving of NodeInfos
class InformationDistributionStrategy
  def initialize(nodeInfoProvider, informationConsumer, interconnection)
    @nodeInfoProvider = nodeInfoProvider
    @informationConsumer = informationConsumer
    @interconnection = interconnection
    @interconnection.addReceiveHandler(NodeInfoWithId, InformationDistributionStrategyMessageHandler.new(@informationConsumer))
  end

  def start
    @extractThread = Thread.new() { nodeInfoExtractThread() }
  end

  def waitForFinished
    @extractThread.join
  end

  #Callback method, informing about significant change in a dynamic node info
  def notifyChange(nodeInfo)
    @interconnection.dispatch(nil, nodeInfo) # broadcast to all nodes
    #@sendQueue.enqueue(nodeInfo)
  end
  private
  #Enqueues dynamic info about the current node to be send (once per second)
  def nodeInfoExtractThread
    while true
      sleep(1)
      @interconnection.dispatch(nil, sendElement) # broadcast to all nodes
    end
  end
end

class InformationDistributionStrategyMessageHandler
  def initialize(informationConsumer)
    @informationConsumer = informationConsumer
  end
  def handleFrom(message, from)
    @informationConsumer.infoReceived(from.ipAddress, message)
  end
end

