gcc_config_release=-std=c++20 -O3 -g -DNDEBUG
gcc_config_debug=-std=c++20 -O3 -g
VPATH=.\..\pipeline

r: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config_release)
	@echo release
test.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config_release)
pipeline.o: pipeline.cpp
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config_release)

d: clean testd.o pipelined.o
	g++ test.o pipeline.o -o test $(gcc_config_debug)
	@echo debug
testd.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config_debug)
pipelined.o: pipeline.cpp
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config_debug)
	
.PHONY: clean
clean:
	del *.exe
	del *.o