#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "socket.h"

/* Lit le ficher welcome et le transmet au client, prend la socket client en 
paramètre, et retourne 0 si l'opération c'est passée, sinon -1*/
int bienvenue(int socket_client) {
    int fileDescriptor = open("welcome.txt", O_RDONLY);
    char buffer[80] = {0};

    // On vérifie que le fichier est correctement ouvert
    if (fileDescriptor == -1) {
        perror("Ouverture du fichier");
        return -1;
    }

    // On lit tant que le fichier n'est pas vide
    while (read(fileDescriptor, buffer, 79) != -1) {
        write(socket_client, buffer, strlen(buffer));
        // On vide le buffer
        memset(buffer, 0, 80);
    }
    // On ferme le fichier
    close(fileDescriptor);
    return 0;
}

/* Même chose que bienvenue(), sauf qu'elle lit le fichier ligne et le retourne 
au client par un nombre de d'appels à la fonction write équivalent au nombre de 
lignes.
De plus elle intègre un délais d'une seconde */
int bienvenueWithDelay(int socket_client) {
    sleep(1);

    FILE* fichier = fopen("welcome.txt", "r");
    char* ligne = NULL;
    size_t tailleLigne = 0;

    // On vérifie que le fichier est correctement ouvert
    if (fichier == NULL) {
        perror("Ouverture du fichier");
        return -1;
    }
    
    // On lit le fichier ligne par ligne avec getline
    while(getline(&ligne, &tailleLigne, fichier) != -1) {
        printf("%s", ligne);
        write(socket_client, ligne, tailleLigne);
        // On réinitialise la ligne à null
        ligne = NULL;    
    }
    // On ferme le fichier
    fclose(fichier);
    return 0;
}

/* Lit les messages du client et les lui renvoie.
Prend en paramètre la socket_client et renvoie void */
void perroquet(int socket_client) {
    char buffer[80] = {0};

    while (read(socket_client, buffer, 79) > 0) {
        write(socket_client, buffer, strlen(buffer));
        // On vide le buffer
        memset(buffer, 0, 80);
    }
}

int main() {
    int socket_server = creer_serveur(8080);
    int socket_client;

    while (1) {
        socket_client = accept(socket_server, NULL, NULL);

        if (socket_client == -1) {
            perror("accept");
            return socket_client;
        }
        // On appelle la fonction bienvenue
        // bienvenue(socket_client);
        bienvenueWithDelay(socket_client);
        // On appelle le perroquet
        perroquet(socket_client);
        // On ferme la socket_client
        close(socket_client);
    }
}