all:
	gcc -pg -O3 hce.c utils.c cons.c tests.c interaction.c hashtable.c -o hce
debug:
	gcc -g hce.c utils.c cons.c tests.c interaction.c hashtable.c -o hce  -fprofile-arcs -ftest-coverage
