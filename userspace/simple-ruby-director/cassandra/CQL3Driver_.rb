
require 'cassandra'

cluster = Cassandra.connect

keyspace = 'system'


session = cluster.connect(keyspace)

session.execute('DROP KEYSPACE KEYSPACE_NAME')
puts 'Deleted clondike'

keyspace_definition = <<-KEYSPACE_CQL
  create keyspace if not exists clondike
  with replication = {
    'class': 'SimpleStrategy',
    'replication_factor': 3
  }
KEYSPACE_CQL

#session.execute(keyspace_definition)

