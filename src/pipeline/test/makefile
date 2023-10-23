VPATH=../pipeline
ifeq ($(MAKECMDGOALS),r)
	gcc_config=-std=c++20 -O3 -DNDEBUG
	show_msg=release
else
	gcc_config=-std=c++20 -O3
	show_msg=debug
endif

d: clean test.o
	g++ test.o -o a $(gcc_config)
	@echo $(show_msg)

r: d

test.o: test.cpp
	g++ test.cpp -c -o test.o $(gcc_config)

.PHONY: clean
clean:
	powershell "rm *.exe"
	powershell "rm *.*o"