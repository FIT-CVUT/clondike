require 'cassandra'


class CQL3Driver

    records = {}

    @session = nil    
    @tasks = nil    

    KEYSPACE_NAME = "CLONDIKE"
    TABLE_NAME = "TASK_TIMELINE"

    def initialize(host='localhost')
	cluster = Cassandra.connect(hosts: [host]) # connects to localhost by default
	@session  = cluster.connect('system') # create session, optionally scoped to a keyspace, to execute queries
	createKeySpaceIfNotExists()
	createTableIfNotExists()
	@tasks = Queue.new()
    end

    def createKeySpaceIfNotExists()
	keyspace_definition = <<-KEYSPACE_CQL
	create keyspace if not exists #{KEYSPACE_NAME} 
	with replication = {
	    'class': 'SimpleStrategy',
	    'replication_factor': 3
	}
	KEYSPACE_CQL
	@session.execute(keyspace_definition)
	@session.execute("USE #{KEYSPACE_NAME}")
    end

    def createTableIfNotExists()
	table_definition = <<-TABLE_CQL
	create table if not exists #{TABLE_NAME} (
	    id_task UUID,
	    id_src_node VARCHAR,
	    id_dst_node VARCHAR,
	    ret_send_task INT,
	    ret_accept_task INT,
	    ret_send_back INT,
	    ret_accept_back INT,
	    time_send_task TIMESTAMP,
	    time_accept_task TIMESTAMP,
	    time_send_back TIMESTAMP,
	    time_accept_backid TIMESTAMP,
	    PRIMARY KEY (id_task)
	)
	TABLE_CQL
	@session.execute(table_definition)
    end

    def createRecord(id_task, id_src_node, id_dst_node, ret, time)


    end
	

end

class TaskRecord
        attr_accessor :id_task, :id_src_node, :id_dst_node, :ret,:time
    def initialize(id_task, id_src_node, id_dst_node, ret, time)
    	@id_task = id_task
    	@id_src_node = id_src_node
    	@id_dst_node = id_dst_node
    	@ret = ret
    	@time = time
    end

end

CQLDriver = CQL3Driver.new()






