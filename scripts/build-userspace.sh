cd /root/clondike/userspace/director-api
make clean
make

cd ../ruby-director-api
make clean
make
make install

echo "build without npfs"
#cd /root/clondike/root/npfs_install
#make clean
#make
