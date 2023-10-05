gcc_config=-std=c++20 -O3 -g
gcc_release_macro=-DNDEBUG
VPATH=.\..\pipeline

r: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo release

d: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo debug

test.o: test.cpp
ifeq ($(MAKECMDGOALS), r)
	g++ test.cpp -c -o test.o $(gcc_config) $(gcc_release_macro)
else
	g++ test.cpp -c -o test.o $(gcc_config)
endif

pipeline.o: pipeline.cpp
ifeq ($(MAKECMDGOALS), r)
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config) $(gcc_release_macro)
else
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config)
endif

.PHONY: clean
clean:
	del *.exe
	del *.o