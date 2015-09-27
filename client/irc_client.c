#include "../irc_common.h"

typedef struct f {
	int sockfd; // user's socket descriptor
	char alias[ALIASLEN]; // user's name
	char channel[CHNLEN]; //channel's name
}USER;
 
typedef struct e{
    pthread_t thread_ID; // thread's pointer
    int sockfd; // socket file descriptor
}THREADINFO;
 
int isconnected, sockfd, SERVERPORT;
char option[LINEBUFF];
USER me;
 
int connect_with_server();
void setalias(USER *me);
void logout(USER *me);
void login(USER *me);
void *receiver(void *param);
void sendtoall(USER *me, char *msg);
void sendtoalias(USER *me, char * target, char *msg);
 
int main(int argc, char **argv) {

	int sockfd, aliaslen;
   
	if(argc < 2) {
		fprintf(stderr, "Usage: server <port> \n");
		exit(-1);
	}

	printf("Type \\HELP to see help\n");
   	SERVERPORT = atoi(argv[1]);	
    memset(&me, 0, sizeof(USER));
    
    while(gets(option)) {
        if(!strncmp(option, "\\EXIT", 5)) {
            logout(&me);
            break;
        }
        if(!strncmp(option, "\\HELP", 5)) {
			printf("\\HELP - to get a list of commands\n");
			printf("\\LOGIN - to login into the network\n");
			printf("\\NICKSERV - to register/change you nick\n");
			printf("\\JOIN - to join a particular channels\n");
			printf("\\LOGOUT - to quit the network\n");
			printf("\\EXIT - to quit the network and exit the application\n");
        }
        else if(!strncmp(option, "\\LOGIN", 6)) {
            char *ptr = strtok(option, " ");
            ptr = strtok(0, " ");
            memset(me.alias, 0, ALIASLEN);
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > ALIASLEN) ptr[ALIASLEN] = 0;
                strcpy(me.alias, ptr);
            }
            else strcpy(me.alias, "Anonymous");
            
            login(&me);
        }
        else if(!strncmp(option, "\\NICKSERV", 9)) {
            char *ptr = strtok(option, " ");
            ptr = strtok(0, " ");
            memset(me.alias, 0, sizeof(char) * ALIASLEN);
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > ALIASLEN) ptr[ALIASLEN] = 0;
                strcpy(me.alias, ptr);
                setalias(&me);
            }
        }
		else if(!strncmp(option, "\\JOIN", 8)) {
		   char *ptr = strtok(option, " ");
		   ptr = strtok(0, " ");
		   memset(me.channel, 0, CHNLEN);
		   if(ptr != NULL) strcpy(me.channel, ptr);
		}
       /* else if(!strncmp(option, "whisp", 5)) {
            char *ptr = strtok(option, " ");
            char temp[ALIASLEN];
            ptr = strtok(0, " ");
            memset(temp, 0, sizeof(char) * ALIASLEN);
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > ALIASLEN) ptr[ALIASLEN] = 0;
                strcpy(temp, ptr);
                while(*ptr) ptr++; ptr++;
                while(*ptr <= ' ') ptr++;
                sendtoalias(&me, temp, ptr);
            }
        }*/
        else if(!strncmp(option, "\\LOGOUT", 7)) {
            logout(&me);
        }
		else sendtoall(&me, &option);
    }
    return 0;
}
 
void login(USER *me) {
    int recvd;
    if(isconnected) {
        fprintf(stderr, "You are already connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return;
    }
    sockfd = connect_with_server();
    if(sockfd >= 0) {
        isconnected = 1;
        me->sockfd = sockfd;
        if(strcmp(me->alias, "Anonymous")) setalias(me);
        printf("Logged in as %s\n", me->alias);
        printf("Receiver started [%d]...\n", sockfd);
        THREADINFO threadinfo;
        pthread_create(&threadinfo.thread_ID, NULL, receiver, (void *)&threadinfo);
 
    }
    else  fprintf(stderr, "Connection rejected...\n");
    
}
 
int connect_with_server() {
    int newfd, err_ret;
    struct sockaddr_in serv_addr;
    struct hostent *to;
 
    /* generate address */
    if((to = gethostbyname(SERVERIP)) == NULL) {
        err_ret = errno;
        fprintf(stderr, "gethostbyname() error...\n");
        return err_ret;
    }
 
    /* open a socket */
    if((newfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        fprintf(stderr, "socket() error...\n");
        return err_ret;
    }
 
    /* set initial values */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    serv_addr.sin_addr = *((struct in_addr *)to->h_addr);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    /* try to connect with server */
    if(connect(newfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "connect() error...%m\n");
        return err_ret;
    }
    else {
        printf("Connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return newfd;
    }
}
 
void logout(USER *me) {
    int sent;
    PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&packet, 0, sizeof(PACKET));
    strcpy(packet.option, "exit");
    strcpy(packet.alias, me->alias);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(PACKET), 0);
    isconnected = 0;
}
 
void setalias(USER *me) {
    int sent;
    PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&packet, 0, sizeof(PACKET));
    strcpy(packet.option, "alias");
    strcpy(packet.alias, me->alias);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(PACKET), 0);
}
 
void *receiver(void *param) {
    int recvd;
    PACKET packet;
    
    printf("Waiting here [%d]...\n", sockfd);
    while(isconnected) {
        
        recvd = recv(sockfd, (void *)&packet, sizeof(PACKET), 0);
        if(!recvd) {
            fprintf(stderr, "Connection lost from server...\n");
            isconnected = 0;
            close(sockfd);
            break;
        }
        if(recvd > 0) {
		printf("in receiver %s", packet.channel);
	    	if(!strcmp(packet.channel, me.channel)) 
			printf("[%s] <%s>: %s\n", ctime(&packet.ptime), packet.alias, packet.buff);
        }
        memset(&packet, 0, sizeof(PACKET));
    }
    return NULL;
}
 
void sendtoall(USER *me, char *msg) {
    int sent;
    PACKET packet;
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    msg[BUFFSIZE] = 0;
    
    memset(&packet, 0, sizeof(PACKET));
    strcpy(packet.option, "send");
    strcpy(packet.alias, me->alias);
    strcpy(packet.buff, msg);
    strcpy(packet.channel, me->channel);

    packet.ptime = time(NULL);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(PACKET), 0);
}
 
void sendtoalias(USER *me, char *target, char *msg) {
    int sent, targetlen;
    PACKET packet;
    
    if(target == NULL || msg == NULL) { 
        return;
    }
    
    if(!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }

    msg[BUFFSIZE] = 0;
    targetlen = strlen(target);
    
    memset(&packet, 0, sizeof(PACKET));
    strcpy(packet.option, "whisp");
    strcpy(packet.alias, me->alias);
    strcpy(packet.buff, target);
    strcpy(&packet.buff[targetlen], " ");
    strcpy(&packet.buff[targetlen+1], msg);
    
    /* send request to close this connetion */
    sent = send(sockfd, (void *)&packet, sizeof(PACKET), 0);
}
