#include <event.h>
#include <pthread.h>
#include "cage.hpp"
#include "libcage-api.hpp"

/**
 *  Helper functions and dates
 */
#define LIBCAGE_API_JOIN_SUCCESSED 1
#define LIBCAGE_API_JOIN_FAILED 0
#define LIBCAGE_API_JOIN_UNDEFINED -1

pthread_mutex_t	libcage_api_callback_mutex;
pthread_attr_t	libcage_api_joinable_attr;
pthread_t		libcage_api_thread_id;
int				libcage_api_callback_value;
void join_callback(bool result)
{
	if (result)
	{
		libcage_api_callback_value = LIBCAGE_API_JOIN_SUCCESSED;
		std::cout << "join: succeeded" << std::endl;
	}
	else
	{
		libcage_api_callback_value = LIBCAGE_API_JOIN_FAILED;
		std::cout << "join: failed" << std::endl;
	}

	pthread_mutex_unlock(&libcage_api_callback_mutex);
}
void *dispatch_thread(void *attribute)
{
	std::cout << "pthread_dispatch()" << std::endl;
	libcage_dispatch();
	pthread_exit(NULL);
}

/**
 *  Functions for export to Ruby
 */
libcage::cage *libcage_open(int port)
{
	event_init();
	libcage::cage *cage = new libcage::cage();
	std::cout << __FILE__ << ":" << __LINE__ << " Try open port " << port << std::endl;
	if(!cage->open(PF_INET, port))
		std::cerr << "ERROR: " << __FILE__ << ":" << __LINE__ << " Cannot open port " << port << std::endl;
	cage->set_global();
	return cage;
}
void libcage_print_state(const libcage::cage *cage)
{
	cage->print_state();
}
bool libcage_join(libcage::cage *cage, const char *host, int port)
{
	pthread_attr_init(&libcage_api_joinable_attr);
	pthread_attr_setdetachstate(&libcage_api_joinable_attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&libcage_api_callback_mutex, NULL);
	libcage_api_callback_value = -1;
	pthread_mutex_lock(&libcage_api_callback_mutex);
	
	cage->join(host,port, &join_callback);
	std::cout << "After join..." << std::endl;

	pthread_create(&libcage_api_thread_id, &libcage_api_joinable_attr, dispatch_thread, NULL);
	std::cout << "After join dispatch" << std::endl;
	
	pthread_mutex_lock(&libcage_api_callback_mutex);
	printf("Odblokovani libcage_join()\n");
	printf("libcage_api_callback_value: %d\n", libcage_api_callback_value);

	return (libcage_api_callback_value==LIBCAGE_API_JOIN_SUCCESSED ? true : false);
}
void libcage_dispatch()
{
	event_dispatch();
}

/*
int main (int argc, char *argv[])
{
	libcage::cage *node1, *node2;

	node1 = libcage_open(10001);
	node2 = libcage_open(10002);
	libcage_print_state(node1);
	libcage_print_state(node2);
	std::cout << "Will join..." << std::endl;
	libcage_join(node1, "localhost", 10002);
	return 0;
}
*/
