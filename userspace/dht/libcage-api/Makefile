all:
	cd `git rev-parse --show-toplevel`; git submodule init; git submodule update
	cd libcage; omake DEBUG=TRUE
	g++ -Wall -Wno-long-long -I./libcage/src/ -fPIC -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -fno-strict-aliasing -o libcage-api.o -c libcage-api.cpp
	g++ -Wall -Wno-long-long -shared -o libcage-api.so libcage-api.o ./libcage/src/*.o -Wl,-z,relro -rdynamic -Wl,-export-dynamic -lpthread -lrt -ldl -lcrypt -lm -lc -lssl -levent
	#g++ -I./libcage/src/ -fPIC -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -Wall -fno-strict-aliasing ./libcage/src/libcage.so -o libcage-api-test libcage-api.cpp

edit:
	vim -p libcage-api.*pp Libcage*.rb examples/* Makefile
