VPATH=.\..\pipeline
ifeq ($(MAKECMDGOALS),d)
	gcc_config=-std=c++20 -O3
else
	gcc_config=-std=c++20 -O3 -DNDEBUG
endif

d: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo debug
	
r: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo release

test.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config)

pipeline.o: pipeline.cpp
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config)

.PHONY: clean
clean:
	del *.exe
	del *.o