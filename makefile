all:
	gcc -g -pg hce.c utils.c cons.c tests.c interaction.c hashtable.c -o hce
