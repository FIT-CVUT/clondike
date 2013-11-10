require 'mkmf'

dir_config("libcage")
find_header("cage.hpp", "../libcage/src/")
find_library("libcage", "../libcage/src/")
#find_library("director-api", "initialize_director_api", "../director-api/") WTF this does not work?
$LDFLAGS += " -L../libcage/src/"
$libs = append_library($libs, "libcage")
#$libs = append_library($libs, "nl")
create_makefile("rb_libcage")
