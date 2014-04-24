require 'UserConfiguration'

# Helper class used for target matching in migration strategies
class TargetMatcher
  # Performs the matching
  # Accepts block param.. that calculates quality of the node candidate
  #                     - returns nil if the node cannot be used
  #                     - The higher number the better
  # Return index to array candidateNodes the best quality node or nil
  def TargetMatcher.performMatch(pid, uid, name, candidateNodes)
    target = nil
    targetQuality = nil
    userConfig = UserConfiguration.getConfig(uid)
    candidateNodes.each_index do |index|
      node = candidateNodes[index]
      # $log.debug "Index: #{index} node #{node} #{node ? node.nodeInfo : "No Node"} #{node && node.nodeInfo ? node.nodeInfo.maximumAccept : "No Node-Info"}... #{node ? node.nodeId : "No nodeId"}"
      next if !canMigrateTo(pid, uid, name, node)
      nodeQuality = yield node
      if ( nodeQuality != nil && (targetQuality == nil || nodeQuality > targetQuality) ) then
        targetQuality = nodeQuality
        target = index
      end
    end

    target
  end

  def TargetMatcher.canMigrateTo(pid, uid, name, node)
    userConfig = UserConfiguration.getConfig(uid)
	if node
		if node.nodeInfo
			if node.nodeInfo.maximumAccept < 1
				$log.debug("canMigrateTo failed - maximumAccept = #{node.nodeInfo.maximumAccept}")
			end
			
			if !userConfig.canMigrateTo(name, node.nodeId)
				$log.debug("canMigrateTo failed - userConfig.canMigrateTo failed with name = #{name}")
			end
		else
			$log.debug("canMigrateTo failed - nodeinfo is nil")
		end
	else
		$log.debug("canMigrateTo failed - node nil")
	end
	
    return node && node.nodeInfo && node.nodeInfo.maximumAccept >= 1 && userConfig.canMigrateTo(name, node.nodeId)
  end
end
