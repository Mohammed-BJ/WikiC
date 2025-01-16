#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "comptine_utils.h"

int read_until_nl(int fd, char *buf)
{
	char car;
	int nb_octets = 0;
	while (read(fd,&car,1) > 0 && car != '\n'){
		buf[nb_octets] = car;
		nb_octets++;
	}
	return nb_octets;
}

int est_nom_fichier_comptine(char *nom_fich)
{
	int length = 0;
	while(nom_fich[length] != '\0'){  
		length++;  
	}
		if(nom_fich[length-1] == 't' && nom_fich[length-2] == 'p' && nom_fich[length-3] == 'c' && nom_fich[length-4] == '.'){
			return 1;
		}
	return 0;
}

char* concat_avec_slash(char* s1, char* s2){
    int length1 = 0;
    int length2 = 0;
    for(int i = 0; s1[i] != '\0'; i++){  length1++; }
    for(int i = 0; s2[i] != '\0'; i++){  length2++;  }
	char* s = (char*) malloc((length1 + length2 + 2) * sizeof(char)); // +2 pour le slash et le caractère nul
    if (s == NULL) {
        printf("Erreur d'allocation de mémoire");
        exit(1);
    }
    for(int i = 0; i < length1; i++){
        s[i] = s1[i];
    }
    s[length1] = '/';
	int j = length1 + 1;
    for(int i = 0; i < length2; i++){
        s[j] = s2[i];
		j++;
    }
	s[length1 + length2 + 2] = '\0';
    return s;
}

struct comptine *init_cpt_depuis_fichier(const char *dir_name, const char *base_name)
{
  	struct comptine *compt = malloc(sizeof(struct comptine));
	  if (compt == NULL) {
        printf("Erreur d'allocation de mémoire\n");
        exit(1);
	}
	compt->nom_fichier = malloc(strlen(base_name) + 1);
	strcpy(compt->nom_fichier,base_name);
	if(est_nom_fichier_comptine((char*)base_name)){
		compt->nom_fichier = (char*)base_name;
		int fd1;
		char *s = concat_avec_slash((char*)dir_name,(char*)base_name);
		if((fd1 = open(s, O_RDONLY, 0640)) == -1){
			printf("error\n");
			exit(0);
		}
		free(s);
		char *tmp = malloc(100*sizeof(char));
		read_until_nl(fd1, tmp);
		compt->titre = malloc(strlen(tmp) + 1);
		strcpy(compt->titre,tmp);
		return compt;
 	}
	return NULL;
}


void liberer_comptine(struct comptine *cpt)
{
	free(cpt->titre);
    free(cpt->nom_fichier);
    free(cpt);
}

struct catalogue *creer_catalogue(const char *dir_name) {
    // Ouvrir le répertoire
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        printf("Erreur d'ouverture du répertoire\n");
        return NULL;
    }

    // Compter le nombre de fichiers de comptines dans le répertoire
    int nb_comptines = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (est_nom_fichier_comptine(ent->d_name)) {
            nb_comptines++;
        }
    }
    rewinddir(dir);

    // Allouer la mémoire pour le catalogue et le tableau de comptines
    struct catalogue *catal = malloc(sizeof(struct catalogue));
    if (catal == NULL) {
        printf("Erreur d'allocation de mémoire\n");
        closedir(dir);
        return NULL;
    }
    catal->nb = nb_comptines;
    catal->tab = malloc(catal->nb * sizeof(struct comptine *));
    if (catal->tab == NULL) {
        printf("Erreur d'allocation de mémoire\n");
        free(catal);
        closedir(dir);
        return NULL;
    }

    // Lire les fichiers de comptines dans le répertoire et initialiser les structures de comptines
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (est_nom_fichier_comptine(ent->d_name)) {
            catal->tab[i] = init_cpt_depuis_fichier(dir_name, ent->d_name);
            i++;
        }
    }
    rewinddir(dir);

    // Fermer le répertoire et retourner le catalogue
    closedir(dir);
    return catal;
}

void liberer_catalogue(struct catalogue *c)
{
	for(int i = 0; i < c->nb; i++){
		liberer_comptine(c->tab[i]);
	}
	free(c->tab);
	free(c);
}

