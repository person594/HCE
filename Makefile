all:
	gcc -g -pg -O3 hce.c utils.c cons.c tests.c interaction.c hashtable.c -o hce
