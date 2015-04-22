require 'cassandra'

#The class provides an object for logging to Apache Cassandra
#Beta version is preparation - the need to debug with Cassandra


class CQL3Driver

    records = {}
    @cluster = nil
    @session = nil    
    @tasks = nil    

    KEYSPACE_NAME = "CLONDIKE"
    TABLE_NAME = "TASK_TIMELINE"

    def initialize(host='localhost')
	@cluster = Cassandra.connect(hosts: [host]) # connects to localhost by default
	@session  = cluster.connect('system') # create session, optionally scoped to a keyspace, to execute queries
	createKeySpaceIfNotExists()
	createTableIfNotExists()
	@tasks = Queue.new()
	@cluster.use('#{KEYSPACE_NAME}')
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
	    ret_em_task INT,
	    ret_em_f_task INT,
	    ret_imm_r_task INT,
	    ret_imm_c_task INT,
	    time_em_task TIMESTAMP,
	    time_em_f_task TIMESTAMP,
	    time_imm_r_task TIMESTAMP,
	    time_imm_c_task TIMESTAMP,
	    PRIMARY KEY (id_task)
	)
	TABLE_CQL
	@session.execute(table_definition)
    end

    #
    # Creates records by type, for testing is used command echo into specific log file
    # 
    def createRecord(type, id_task, id_src_node, id_dst_node, ret, time)
    	case type
    		when "EMIGRATE"
    			# stat = cluster.execute("INSERT INTO #{TABLE_NAME}(id_task, id_src_node, id_dst_node, ret_em_task, time_em_task) VALUES(#{id_task}, #{id_src_node}, #{id_dst_node}, #{ret}, #{time})", consistency: :one)
    			`echo "#{id_task} #{id_src_node} #{id_dst_node} #{ret} #{time}" >> /home/clondike/emigrate.log`

    		when "EMIGRATE_FAILED"	
				# stat = cluster.execute("INSERT INTO #{TABLE_NAME}(id_task, id_src_node, id_dst_node, ret_em_f_task, time_em_f_task) VALUES(#{id_task}, #{id_src_node}, #{id_dst_node}, #{ret}, #{time})", consistency: :one)
    			`echo "#{id_task} #{id_src_node} #{id_dst_node} #{ret} #{time}" >> /home/clondike/emigrate_failed.log`

    		when "IMMIGRATION_REQUEST"
    			# stat = cluster.execute("INSERT INTO #{TABLE_NAME}(id_task, id_src_node, id_dst_node, ret_imm_r_task, time_imm_r_task) VALUES(#{id_task}, #{id_src_node}, #{id_dst_node}, #{ret}, #{time})", consistency: :one)
    			`echo "#{id_task} #{id_src_node} #{id_dst_node} #{ret} #{time}" >> /home/clondike/immigration_request.log`
    		
    		when "IMMIGRATION_CONFIRMED"
    			# stat = cluster.execute("INSERT INTO #{TABLE_NAME}(id_task, id_src_node, id_dst_node, ret_imm_c_task, time_imm_c_task) VALUES(#{id_task}, #{id_src_node}, #{id_dst_node}, #{ret}, #{time})", consistency: :one)
    			`echo "#{id_task} #{id_src_node} #{id_dst_node} #{ret} #{time}" >> /home/clondike/immigration_confirmed.log`
    		else
    			$log.err("Unknown type of record #{type}")
    		end	
    end
	

end

# Class represents record for Clondike table named task_timeline

class TaskRecord
        attr_accessor :id_task, :id_src_node, :id_dst_node, :ret,:time
    def initialize(type, id_task, id_src_node, id_dst_node, ret, time)
    	@type = type
    	@id_task = id_task
    	@id_src_node = id_src_node
    	@id_dst_node = id_dst_node
    	@ret = ret
    	@time = time
    end

end






