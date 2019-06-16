#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "socket.h"

/* Fonction appelée par la structure sigaction, elle se charge de supprimer un
processus fils zombie */
void traitement_signal(int sig) {
    printf("Signal %d reçu\n", sig);
    int status;
    waitpid(-1, &status, WNOHANG);
}

/* Gestion du comportement du serveur lors de la reception de divers signaux */
void initialiser_signaux() {

    /* Permet d'ignorer le signal lorsque le serveur utilise la fonction write et
    que le client se déconnecte */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("Signal");
    }

    /* Mise en place de la structure sigaction permettant de supprimer les
    processus zombies */
    struct sigaction str_sigaction;

    str_sigaction.sa_handler = &traitement_signal;
    sigemptyset(&str_sigaction.sa_mask);
    str_sigaction.sa_flags = SA_RESTART;

    // On vérifie si sigaction se déroule bien
    if (sigaction(SIGCHLD, &str_sigaction, NULL) == -1) {
        perror("sigaction(SIGCHLD)");
    }
}

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
void perroquet(int descriptorClient) {
    FILE* socketClient = fdopen(descriptorClient, "w+");
    char buffer[250] = {0};

    // On vérifie l'ouverture du fichier socket
    if (socketClient == NULL) {
        perror("Ouverture de la socket client échouée");
    }

    // Boucle de lecture/réponse
    while(fgets(buffer, 250, socketClient) != NULL) {
        // Réponse du serveur
        fprintf(socketClient, "<Pawnee> %s", buffer);
        printf("<client %d> %s", descriptorClient, buffer);
    }
    fclose(socketClient);
}

void requetteHttp(int descriptorClient) {
    FILE* socketClient = fdopen(descriptorClient, "w+");
    char bufferRequest[250] = {0};
    char bufferContent[250] = {0};

    // On vérifie l'ouverture du fichier socket
    if (socketClient == NULL) {
        perror("Ouverture de la socket client échouée");
    }
    // On lit la première ligne, qui contiens la requette
    else if (fgets(bufferRequest, 250, socketClient) == NULL) {
        perror("fget retourne NULL");
    }

    // Lecture du contenu
    do {
        fgets(bufferContent, 250, socketClient);
    } while(strcmp(bufferContent, "\r\n") != 0);
    
    // Réponse
    if (strcmp(bufferRequest, "GET / HTTP/1.1\r\n") != 0) {
        perror("400 Bad Request");
        fprintf(socketClient, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad Request\r\n\r\n");
    }
    else {
        fprintf(socketClient, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: \r\n\r\nHello\r\n\r\n");
    }
    fclose(socketClient);
}

int main() {
    initialiser_signaux();
    int socket_server = creer_serveur(8080);
    int socket_client;
    int pid;

    while (1) {
        socket_client = accept(socket_server, NULL, NULL);

        // On créer un nouveau processus
        pid = fork();
        // On est dans le processus fils
        if (pid == 0) {

            if (socket_client == -1) {
                perror("accept");
            }

            // On appelle requetteHttp
            requetteHttp(socket_client);
            // On ferme le processus fils
            exit(0);
        // Une erreur c'est produite
        } else if (pid == -1) {
            perror("Création du processus a échouée");
        // On est dans le processus père
        } else {
            // On ferme la socket_client
            close(socket_client);
        }
    }
}