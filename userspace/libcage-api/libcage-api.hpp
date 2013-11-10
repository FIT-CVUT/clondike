extern "C"
{
	libcage::cage	*libcage_open(int port);
	void			libcage_print_state(const libcage::cage *cage);
	bool			libcage_join(libcage::cage *cage, const char *host, int port);
	void			libcage_dispatch();
}
