all: task1

task1:
	g++ --std=c++11 -DCPP -DGPP -I/usr/csshare/pkgs/csim_cpp-19.0/lib -m32 main.cpp /usr/csshare/pkgs/csim_cpp-19.0/lib/csim.cpp.a -lm -o main.o 

