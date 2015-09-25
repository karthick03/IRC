#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
 
#define IP "127.0.0.1"
#define BACKLOG 10
#define CLIENTS 10
 
#define BUFFSIZE 1024
#define ALIASLEN 32
#define CHNLEN 16
#define OPTLEN 16
#define SERVERIP "127.0.0.1"
#define LINEBUFF 2048

struct PACKET {
    char option[OPTLEN]; // instruction
    char alias[ALIASLEN]; // client's alias
    char channel[CHNLEN];
    char buff[BUFFSIZE]; // payload
    time_t recvtime;
};
