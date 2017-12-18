#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <features.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <assert.h>
#include <inttypes.h>

void afficher(pid_t pid, int couleur, int lecture);
void handler(int sig);
int n;
pid_t *etageZero = NULL;
/**
 * n doit correspondre au nombre de processus souhaité. c le nombre de couleur.
 */
int main(int argc, char *argv[])
{
	int c, i, j, **tube = NULL, fichier;
	if (argc != 3)
	{
		fprintf(stderr,
				"Erreur, nombre d'argument insuffisant.\nDeux sont necessaires.\n");
		exit(EXIT_FAILURE);
	}
	n = atoi(argv[1]), c = atoi(argv[2]);
	if (n < 2)
	{
		fprintf(stderr, "Nombre de processus insuffisant : 2 minimum.\n");
		exit(EXIT_SUCCESS);
	}
	else if (c < 2)
	{
		fprintf(stderr, "Nombre de couleur insuffisant : 2 minimum.\n");
		exit(EXIT_SUCCESS);
	}
	etageZero = (pid_t *) malloc(n * sizeof(pid_t));
	tube = (int **) malloc(n * sizeof(int *));
	for (i = 0; i < n; ++i)
	{
		tube[i] = (int *) malloc(2 * sizeof(int));
		assert(pipe(tube[i]) == 0);
	}
	fichier = open("elu.txt", O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
	signal(SIGUSR1, handler);
	/**
	 *  La première boucle va générer les étages.
	 *  La seconde s'occupe des différents processus de chaque étage.
	 */
	for (i = (n - 1); i >= 0; --i)
	{
		for (j = i; j >= 0; --j)
		{
			/**
			 * Si tu es le premier processus ou le dernier de l'étage, tu vas utiliser un seul tube et non deux.
			 * Sinon le reste utilise deux tubes.
			 */
			if (j == i || j == 0)
			{
				/**
				 * Si tu es au premier étage, tu fais ceci (initialisation)
				 * Sinon si tu es au dernier étage tu lis uniquement et envoie un signal au père.
				 * Sinon tu es dans un autre étage, tu récupères avec deux tubes et tu envoies dans un seul tube.
				 */
				if (i == (n - 1))
				{
					if ((etageZero[i] = fork()) == 0)
					{
						int couleur, lecture;
						char buff[10];
						srand(time(NULL) + getpid());
						couleur = (rand() % c) + 1;
						write(tube[j][1], (void *) &couleur, sizeof(int));
						pause();
						read(fichier, (void *) buff, 1 * sizeof(char));
						lecture = atoi(buff);
						afficher(getpid(), couleur, lecture);
						exit(EXIT_SUCCESS);
					}
				}
				else if (i == 0)
				{
					if (fork() == 0)
					{
						int couleur[2], size;
						char *buff = NULL;
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						/*
						 * La couleur est choisie aléatoirement puis convertit en caractère.
						 * Écriture dans le fichier ensuite.
						 */
						size = asprintf(&buff, "%d", couleur[rand() % 2]);
						write(fichier, (void *) buff, size);
						kill(getppid(), SIGUSR1);
						free(buff);
						buff = NULL;
						exit(EXIT_SUCCESS);
					}
				}
				else
				{
					if ((etageZero[i] = fork()) == 0)
					{
						int couleur[2], choix;
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						choix = rand() % 2;
						write(tube[j][1], (void *) &couleur[choix],
								sizeof(int));
						exit(EXIT_SUCCESS);
					}
				}
				/*
				 * Tu n'es pas le premier ou dernier processus d'un étage.
				 */
			}
			else
			{
				/**
				 * Si tu es au premier étage, tu fais ceci (initialisation en gros)
				 * Sinon tu fais le reste.
				 */
				if (i == (n - 1))
				{
					if ((etageZero[i] = fork()) == 0)
					{
						int couleur, lecture;
						char buff[10];
						srand(time(NULL) + getpid());
						couleur = (rand() % c) + 1;
						write(tube[j][1], (void *) &couleur, sizeof(int));
						write(tube[j][1], (void *) &couleur, sizeof(int));
						pause();
						/*
						 * Après réception d'un signal.
						 */
						read(fichier, (void *) buff, 1 * sizeof(char));
						lecture = atoi(buff);
						afficher(getpid(), couleur, lecture);
						exit(EXIT_SUCCESS);
					}
				}
				else
				{
					if (fork() == 0)
					{
						int couleur[2], choix;
						read(tube[j][0], (void *) &couleur[0], sizeof(int));
						read(tube[j + 1][0], (void *) &couleur[1], sizeof(int));
						choix = rand() % 2;
						write(tube[j][1], (void *) &couleur[choix],
								sizeof(int));
						write(tube[j][1], (void *) &couleur[choix],
								sizeof(int));
						exit(EXIT_SUCCESS);
					}
				}
			}
		}
	}
	for (i = 0; i < n; ++i)
	{
		waitpid(etageZero[i], NULL, 0);
	}
	for (i = 0; i < n; ++i)
	{
		close(tube[i][1]);
		close(tube[i][0]);
		free(tube[i]);
	}
	free(tube);
	free(etageZero);
	close(fichier);
	return EXIT_SUCCESS;
}

void afficher(pid_t pid, int couleur, int lecture)
{
	printf("Le processus %d a la couleur %d et la couleur elue est %d", pid,
			couleur, lecture);
	if (couleur == lecture)
	{
		printf(" : il a gagné !\n");
	}
	else
	{
		printf(", il a perdu.\n");
	}
}

void handler(int sig)
{
	static int flag = 0;

	if (sig != SIGUSR1)
	{
		signal(SIGUSR1, handler);
	}
	else
	{
		/*
		 * Le père a reçu son signal
		 */
		if (flag == 0)
		{
			int i;
			++flag;
			for (i = 0; i < n; ++i)
			{
				kill(etageZero[i], SIGUSR1);
			}
		}
	}
}
