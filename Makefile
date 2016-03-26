all:
	gcc -pg -O3 hce.c utils.c cons.c tests.c interaction.c hashtable.c polyglot.c -o hce
debug:
	gcc -g -pg -O0 hce.c utils.c cons.c tests.c interaction.c hashtable.c polyglot.c -o hce  -fprofile-arcs -ftest-coverage
