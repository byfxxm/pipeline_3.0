VPATH=.\..\pipeline
ifeq ($(MAKECMDGOALS),d)
	gcc_config=-std=c++20 -g
else
	gcc_config=-std=c++20 -O3 -DNDEBUG
endif

r: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo release

d: clean test.o pipeline.o
	g++ test.o pipeline.o -o test $(gcc_config)
	@echo debug

test.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config)

pipeline.o: pipeline.cpp
	g++ ../pipeline/pipeline.cpp -c -o pipeline.o $(gcc_config)

.PHONY: clean
clean:
	del *.exe
	del *.o