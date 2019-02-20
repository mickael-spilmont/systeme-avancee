#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "socket.h"

int main() {
    int socket_server = creer_serveur(8080);
    int socket_client;

    while (1) {
        socket_client = accept(socket_server, NULL, NULL);

        if (socket_client == -1) {
            perror("accept");
            return socket_client;
        }
        write(socket_client, "Bienvenue", strlen("Bienvenue"));
    }
}