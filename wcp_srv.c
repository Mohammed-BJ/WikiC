/* fichiers de la bibliothèque standard */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/* bibliothèque standard unix */
#include <unistd.h> /* close, read, write */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
/* spécifique à internet */
#include <arpa/inet.h> /* inet_pton */
/* spécifique aux comptines */
#include "comptine_utils.h"
#include <semaphore.h>
#include <pthread.h>
#define PORT_WCP 4321
#define LOG_NAME "srv.log"

void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s repertoire_comptines\n"
			"serveur pour WCP (Wikicomptine Protocol)\n"
			"Exemple: %s comptines\n", nom_prog, nom_prog);
}
/** Retourne en cas de succès le descripteur de fichier d'une socket d'écoute
 *  attachée au port port et à toutes les adresses locales. */
int creer_configurer_sock_ecoute(uint16_t port);

/** Écrit dans le fichier de desripteur fd la liste des comptines présents dans
 *  le catalogue c comme spécifié par le protocole WCP, c'est-à-dire sous la
 *  forme de plusieurs lignes terminées par '\n' :
 *  chaque ligne commence par le numéro de la comptine (son indice dans le
 *  catalogue) commençant à 0, écrit en décimal, sur 6 caractères
 *  suivi d'un espace
 *  puis du titre de la comptine
 *  une ligne vide termine le message */
void envoyer_liste(int fd, struct catalogue *c);

/** Lit dans fd un entier sur 2 octets écrit en network byte order
 *  retourne : cet entier en boutisme machine. */
uint16_t recevoir_num_comptine(int fd);

/** Écrit dans fd la comptine numéro ic du catalogue c comme spécifié par le
 *  protocole WCP, c'est-à-dire :
 *  chaque ligne du fichier de comptine est écrite avec son '\n' final, y
 *  compris son titre
 *  deux lignes vides terminent le message */
void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic);
int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
        // Ouverture du journal des connexions
    int log = open(LOG_NAME, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (log < 0) {
        perror("open");
        exit(2);
    }
    // Création d'un mutex pour l'écriture dans le journal des connexions
    pthread_mutex_t mut_logfile;
    pthread_mutex_init(&mut_logfile, NULL);
    /* À compléter */
	int sock1 = creer_configurer_sock_ecoute(PORT_WCP);
	struct catalogue * catal = creer_catalogue(argv[1]);
	while(1) {
        // Accepter une nouvelle connexion
        struct sockaddr_in addr_clt;
        socklen_t taille_addr = sizeof(struct sockaddr_in);
        int sock_clt = accept(sock1,(struct sockaddr*)&addr_clt, &taille_addr);
        if (sock_clt < 0) {
            perror("accept");
            exit(4);
        }
        printf("Message reçu du client %s\n", inet_ntoa(addr_clt.sin_addr));
        char addr_char[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &(addr_clt.sin_addr), addr_char, INET_ADDRSTRLEN) == NULL) {
            perror("inet_ntop");
        } else {
        time_t now = time(NULL);
        char date_heure[32], log_mess[256];
        strftime(date_heure, 32, "%F:%T", localtime(&now));
        sprintf(log_mess, "%s : connection avec %s\n", date_heure, addr_char);
        pthread_mutex_lock(&mut_logfile);
        write(log, log_mess, strlen(log_mess));
        pthread_mutex_unlock(&mut_logfile);
        }
        envoyer_liste(sock_clt, catal);
		uint16_t num_comptine = recevoir_num_comptine(sock_clt);
		envoyer_comptine(sock_clt,argv[1],catal,num_comptine);
        // Fermer la connexion avec le client
        close(sock_clt);
    }
	// Fermer la socket d'écoute
    close(sock1);
		liberer_catalogue(catal);

    return 0;
}


int creer_configurer_sock_ecoute(uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(2);
	}
	struct sockaddr_in sa = { .sin_family = AF_INET,.sin_port = htons(port)};
	//if (inet_pton(AF_INET, "0.0.0.0", &sa.sin_addr) != 1){
		//perror("error");
       // exit(3);
	//}
	if (bind(sock, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		exit(3);
	}
	if (listen(sock, 128) < 0) {
		perror("listen");
		exit(2);
	}
	return sock;
}

void envoyer_liste(int fd, struct catalogue *c)
{
	char n[2] = "\n";
    for (int i = 0; i < c->nb; i++) {
        // Écriture du numéro de la comptine sur 6 caractères, suivi d'un espace
        dprintf(fd, "%6d ", i);

        // Écriture du titre de la comptine
        dprintf(fd, "%s\n", c->tab[i]->titre);
    }
	write(fd,n,2);
}

uint16_t recevoir_num_comptine(int fd)
{
    uint16_t entier;
    if(read(fd,&entier,sizeof(uint16_t)) < 0){
        perror("Erreur de lecture");
        exit(EXIT_FAILURE);
    }
	entier = ntohs(entier);
    return entier; 
}

void envoyer_comptine(int fd, const char *dirname, struct catalogue *c, uint16_t ic) {
    if (ic >= c->nb) {
        perror("Indice de comptine invalide");
        return;
    }

    char filename[100];
    sprintf(filename, "%s/%s", dirname, c->tab[ic]->nom_fichier);
    int file = open(filename, 0);
    if (file == -1) {
        perror("Erreur d'ouverture du fichier de la comptine");
        return;
    }

    char buffer[256];
    int read_count;
    while ((read_count = read(file, buffer, 256)) > 0) {
        write(fd, buffer, read_count);
    }
	write(fd, "\n\n", 2);
    close(file);
}
