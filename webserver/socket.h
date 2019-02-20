#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <unistd.h>

int creer_serveur(int port);
#endif
