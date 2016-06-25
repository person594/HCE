all:
	gcc -pg -O3 src/*.c -o hce
debug:
	gcc -g -pg -O0 src/*.c -o hce  -fprofile-arcs -ftest-coverage
