./build_nachos.sh -j
./build_nachos.sh -jc
make clean
make
../build.linux/nachos -e halt
make createFile fileIO_test1 fileIO_test2
make hw3t1 hw3t2 hw3t3
make hw4t1 hw4t2
