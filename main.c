#define _XOPEN_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <features.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void handler(int sig);

/**
 * n doit correspondre au nombre de processus souhaité. c le nombre de couleur.
 */
int main(int argc, char *argv[]) {
	int n, c, i, j, **tube, nbrPipe, fichier;
	pid_t *etageZero = NULL;
	if (argc != 3) {
		fprintf(stderr,
				"Erreur, nombre d'argument insuffisant.\nDeux sont necessaires.\n");
		exit(EXIT_FAILURE);
	}
	n = atoi(argv[1]), c = atoi(argv[2]);
	etageZero = (pid_t *) malloc(n * sizeof(pid_t));
	nbrPipe = 2 * n - 2; /* Calcul le nombre de pipe nécessaire */
	tube = (int **) malloc(nbrPipe * sizeof(int*));
	for (i = 0; i < nbrPipe; ++i) {
		tube[i] = (int *) malloc(2 * sizeof(int));
		pipe(tube[i]);
	}
	srand(time(NULL) + getpid());
	/**
	 *  La première boucle va générer les étages.
	 *  La seconde s'occupe des différents processus de chaque étage.
	 */
	for (i = n - 2; i >= 0; --i) {
		for (j = i; j >= 0; --j) {
			/**
			 * Si tu es le premier processus ou le dernier de l'étage, tu vas utiliser un seul tube et non deux.
			 * Sinon le reste utilise deux tubes.
			 */
			if (j == i || j == 0) {
				/**
				 * Si tu es au premier étage, tu fais ceci (initialisation en gros)
				 * Sinon si tu es au dernier étage tu lis uniquement et envoie un signal au père.
				 * Sinon tu fais le reste.
				 */
				if (i == (n - 1)) {
					if ((etageZero[i] = fork()) == 0) {
						int couleur;
						couleur = (rand() % c) + 1;
						write(tube[j][1], (void*) &couleur, sizeof(int));
						pause();
					}
				} else if (i == 0) {
					if (fork() == 0) {
						int couleur[2];
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						fichier = open("elu.txt", O_CREAT | O_RDWR | O_APPEND,
						S_IRUSR | S_IWUSR);
						/*
						 * La couleur est choisie aléatoirement.
						 * Écriture dans le fichier ensuite.
						 */
						write(fichier, (void *) &couleur[rand() % 2],
								sizeof(int));
						kill(getppid(), SIGUSR1);
					}
				} else {
					if ((etageZero[i] = fork()) == 0) {
						int couleur[2], choix;
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						choix = rand() % 2;
						write(tube[j][0], (void *) &couleur[choix],
								sizeof(int));
						write(tube[j][0], (void *) &couleur[choix],
								sizeof(int));
						exit(EXIT_SUCCESS);
					}
				}
			} else {
				/**
				 * Si tu es au premier étage, tu fais ceci (initialisation en gros)
				 * Sinon si tu es au dernier étage tu lis uniquement et envoie un signal au père. + écriture dans le fichier.
				 * Sinon tu fais le reste.
				 */
				if (i == (n - 1)) {
					if ((etageZero[i] = fork()) == 0) {
						int couleur;
						couleur = (rand() % c) + 1;
						write(tube[j][1], (void*) &couleur, sizeof(int));
						pause();
					}
				} else if (i == 0) {
					if (fork() == 0) {
						int couleur[2];
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						fichier = open("elu.txt", O_CREAT | O_RDWR | O_APPEND,
						S_IRUSR | S_IWUSR);
						/*
						 * La couleur est choisie aléatoirement.
						 * Écriture dans le fichier ensuite.
						 */
						write(fichier, (void *) &couleur[rand() % 2],
								sizeof(int));
						kill(getppid(), SIGUSR1);
					}
				} else {
					if ((etageZero[i] = fork()) == 0) {
						int couleur[2], choix;
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						choix = rand() % 2;
						write(tube[j][0], (void *) &couleur[choix],
								sizeof(int));
						write(tube[j][0], (void *) &couleur[choix],
								sizeof(int));
						exit(EXIT_SUCCESS);
					}
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

void handler(int sig) {
	if(sig != SIGUSR1){

	}
}
