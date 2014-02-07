class DHTConfig
  K = 1; # count of duplication. Minimal value is 1. For very robust reliable is the recommended value about 20.
  ALPHA = 3; # number of maximum asynchronous/parallel connections (requests to other nodes) for lookup a self NodeID. The minimal value is 1
  SIZE_NODE_ID_SPACE = 2**160 # 160-bit key space: IDs from 0000000000000000000000000000000000000000 to ffffffffffffffffffffffffffffffffffffffff
end
