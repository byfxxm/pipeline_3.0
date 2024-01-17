VPATH=../pipeline
ifeq ($(MAKECMDGOALS),r)
	gcc_config=-std=c++20 -O3 -DNDEBUG
	show_msg=release
else
	gcc_config=-std=c++20 -O3
	show_msg=debug
endif

d: clean test.o gparser.o
	g++ test.o gparser.o -o a $(gcc_config)
	@echo $(show_msg)

r: d

test.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config)

gparser.o:
	g++ ./../pipeline/gparser/gparser.cpp -c -o gparser.o $(gcc_config)

.PHONY: clean
clean:
	del *.exe
	del *.o