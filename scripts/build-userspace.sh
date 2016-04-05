cd /root/clondike/userspace/director-api
make clean
make

cd ../ruby-director-api
make clean
make
#now install is done using cp command
#make install
LIB_DIR=../simple-ruby-director/
echo "copy library to $LIB_DIR"
cp directorApi.so $LIB_DIR

echo "build without npfs"
#cd /root/clondike/root/npfs_install
#make clean
#make
