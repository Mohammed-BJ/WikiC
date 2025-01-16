/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <sys/types.h>
#include <sys/socket.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"
#include <semaphore.h>
#define PORT_WCP 4321
void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s addr_ipv4\n"
			"client pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}

/** Retourne (en cas de succès) le descripteur de fichier d'une socket
 *  TCP/IPv4 connectée au processus écoutant sur port sur la machine d'adresse
 *  addr_ipv4 */
int creer_connecter_sock(char *addr_ipv4, uint16_t port);

/** Lit la liste numérotée des comptines dans le descripteur fd et les affiche
 *  sur le terminal.
 *  retourne : le nombre de comptines disponibles */
uint16_t recevoir_liste_comptines(int fd);

/** Demande à l'utilisateur un nombre entre 0 (compris) et nc (non compris)
 *  et retourne la valeur saisie. */
uint16_t saisir_num_comptine(uint16_t nb_comptines);

/** Écrit l'entier ic dans le fichier de descripteur fd en network byte order */
void envoyer_num_comptine(int fd, uint16_t nc);

/** Affiche la comptine arrivant dans fd sur le terminal */
void afficher_comptine(int fd);

int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
    /* À compléter */
	int sock = creer_connecter_sock(argv[1],PORT_WCP);
    //uint16_t num_comptine = saisir_num_comptine(nb_comptines);
	uint16_t nb_comptines = recevoir_liste_comptines(sock);
    printf("Nombre de comptines : %hd\n", nb_comptines);
    //printf("\n");
    uint16_t num_comptine = saisir_num_comptine(nb_comptines);
    envoyer_num_comptine(sock,num_comptine);
    afficher_comptine(sock);
    //Fermer la connexion avec le serveur
   close(sock);

	return 0;
}

int creer_connecter_sock(char *addr_ipv4, uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(2);
	}
	struct sockaddr_in sa = { .sin_family = AF_INET,.sin_port = htons(port)};
	if (inet_pton(AF_INET, addr_ipv4, &sa.sin_addr) != 1){
		perror("error");
        exit(3);
	}
	if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("Erreur lors de la connexion de la socket");
        exit(EXIT_FAILURE);
    }
	return sock;
}


/*uint16_t recevoir_liste_comptines(int fd){
    int i;
    int nb_comptines = 0;
    char c = '\0';
    //printf("Nombre de comptines : %hd\n", nb_comptines);
    while((i = read(fd, &c, 1)) > 0){
        if(c == '\0'){
            break;
        } else if(c == '\n'){
            nb_comptines++;
        }
        printf("%c", c);
    }
    if(i < 0){
        perror("Erreur de lecture");
        exit(EXIT_FAILURE);
    } 
    return nb_comptines;
}*/

uint16_t recevoir_liste_comptines(int fd){
    int nb_comptines = 0;
    char buf[1024];
    while (1) {
        int n = read_until_nl(fd, buf);
        buf[n] = '\0';
        printf("%s\n", buf);
        if(n <= 0){
            break;
        }
        nb_comptines++;
    }
    if (nb_comptines == 0) {
        printf("Aucune comptine trouvée.\n");
    }
    return nb_comptines;
}


uint16_t saisir_num_comptine(uint16_t nb_comptines)
{
	uint16_t val;
	printf("Veuillez choisir un nombre entre 0 inclus et le nombre de comptine (exclus)");
	scanf("%hd",&val);
	if((val < 0) || (val >= nb_comptines)){
		printf("vous n'avez pas respecte les conditions\n");
		printf("Veuillez choisir un nombre entre 0 inclus et le nombre de comptine (exclus)");
		scanf("%hd",&val);
	}
	return val;
}

void envoyer_num_comptine(int fd, uint16_t nc)
{
	uint16_t val = htons(nc);
	write(fd, &val,16);
}



void afficher_comptine(int fd) {
    char buffer[256];
    int read_count;
    while ((read_count = read(fd, buffer, 256)) > 0) {
        for (int i = 0; i < read_count; i++) {
            if (buffer[i] == '\n') {
                printf("\n");
            } else {
                printf("%c", buffer[i]);
            }
        }
    }
}