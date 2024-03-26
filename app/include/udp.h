#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <string.h>
#include<ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<stdbool.h>

#define MAX_PACKET_SIZE 1500
#define BUF_LENGTH 50

// File for handling UDP connection. Taken from As2

void udp_init(void);
void udp_cleanup(void);
// In case the state of UDP is needed externally.
bool udp_isInitialized(void);