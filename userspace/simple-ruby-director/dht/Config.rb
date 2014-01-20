class DHTConfig
  K = 3; # count of duplication
  ALPHA = 3; # count of nodes for request to lookup (find node request)
  SIZE_NODE_ID_SPACE = 2**160 # 160-bit key space: IDs from 0000000000000000000000000000000000000000 to ffffffffffffffffffffffffffffffffffffffff
end
