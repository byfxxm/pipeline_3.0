gcc_config=-std=c++20 -O3
VPATH=.\..\pipeline

all:gcc_test.o pipeline.o
	g++ gcc_test.o -o gcc_test $(gcc_config)
gcc_test.o:gcc_test.cpp
	g++ gcc_test.cpp -c -o gcc_test.o $(gcc_config)
pipeline.o:pipeline.cpp pipeline.h
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config)
.PHONY:clean
clean:
	del gcc_test.exe
	del *.o