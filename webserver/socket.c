#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

int creer_serveur(int port) {
    int socket_server;
    int optval = 1;
    struct sockaddr_in socketAddr;

    // paramètrage de la structure socketAddr
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = htons (port);
    socketAddr.sin_addr.s_addr = INADDR_ANY;

    // création de la socket serveur
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    // Vérification du bon déroullement de sa création
    if (socket_server == -1) {
        perror("socket_serveur");
        return socket_server;
    }

    // Permet de redemarrer le serveur san delais (p 13 - 14)
    if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
        perror("Can not set SO_REUSEADDR option");
        return -1;
    }

    // Attachement de la socket à une interface réseau et paramètrage
    if (bind(socket_server, (struct sockaddr *)&socketAddr, sizeof(socketAddr)) == -1) {
        perror("bind socket_serveur");
        return -1;
    }

    // démarrer l'attente de connexions
    if (listen(socket_server, 10) == -1) {
        perror("listen socket_serveur");
        return -1;
    }

    return socket_server;
}