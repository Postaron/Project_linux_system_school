#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
	int n, c;
	if (argc != 3) {
		fprintf(stderr,
				"Erreur, nombre d'argument insuffisant.\nDeux sont nécessaires.\n");
		exit(EXIT_FAILURE);
	}
	n = atoi(argv[1]), c = atoi(argv[2]);
	return EXIT_SUCCESS;
}
