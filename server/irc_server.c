#include "../irc_common.h"

#define PATH(x) #x
#define HOMEFILE PATH(server.txt)
typedef struct a {
    pthread_t thread_ID; // thread's pointer
    int sockfd; // socket file descriptor
    char alias[ALIASLEN]; // client's alias
} THREADINFO;
 
typedef struct b {
    THREADINFO threadinfo;
    struct b *next;
} LLNODE;
 
typedef struct c {
    LLNODE *head, *tail;
    int size;
} LLIST;
 
int sockfd, newfd, PORT, isconnected;
THREADINFO thread_info[CLIENTS];
LLIST client_list;
pthread_mutex_t clientlist_mutex;

int compare(THREADINFO *, THREADINFO *);
int list_insert(LLIST *, THREADINFO *);
int list_delete(LLIST *, THREADINFO *);
void list_init(LLIST *);
void list_dump(LLIST *);
int init_as_client(int);
void *client_handler(void *);
void *io_handler(void *);
void *receiver(void *);
int fileExists(char *);
int getPortNumber(char *);
void writeMyDetails(char *);

int main(int argc, char **argv) {
    
	if(argc < 2) {
		fprintf(stderr, "Usage: client <port> \n");
		exit(-1);
	}

   	PORT = atoi(argv[1]);	

	int err_ret, sin_size;
    struct sockaddr_in serv_addr, client_addr;
    pthread_t interrupt;
 	
	list_init(&client_list);
 
    pthread_mutex_init(&clientlist_mutex, NULL);

	if(fileExists(HOMEFILE) == 1)
	{
		if(init_as_client(getPortNumber(HOMEFILE)) == -1)
			fprintf(stderr, "Can't connect to other server..will operate as solo server\n");
		else if(pthread_create(&interrupt, NULL, receiver, NULL) != 0)
			fprintf(stdout, "Waiting for messages from parent server...\n");
	}
	else writeMyDetails(HOMEFILE);
 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        fprintf(stderr, "socket() failed...\n");
        return err_ret;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "bind() failed...\n");
        return err_ret;
    }
 
    if(listen(sockfd, BACKLOG) == -1) {
        err_ret = errno;
        fprintf(stderr, "listen() failed...\n");
        return err_ret;
    }
 
    printf("Starting admin interface...\n");
    if(pthread_create(&interrupt, NULL, io_handler, NULL) != 0) {
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed...\n");
        return err_ret;
    }
 
    printf("Starting socket listener...\n");
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if((newfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&sin_size)) == -1) {
            err_ret = errno;
            fprintf(stderr, "accept() failed...\n");
            return err_ret;
        }
        else {
            if(client_list.size == CLIENTS) {
                fprintf(stderr, "Connection full, request rejected...\n");
                continue;
            }
            printf("Connection requested received...\n");
            THREADINFO threadinfo;
            threadinfo.sockfd = newfd;
            strcpy(threadinfo.alias, "Anonymous");
            pthread_mutex_lock(&clientlist_mutex);
            list_insert(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            pthread_create(&threadinfo.thread_ID, NULL, client_handler, (void *)&threadinfo);
        }
    }
 
    return 0;
}

int fileExists(char *path) 
{ 
	FILE *fp = fopen(path, "r"); 
	if(fp == NULL){
		printf("fp is NULL\n");
	   	return 0;
	}
	else return 1;
}

int getPortNumber(char *path)
{
	if(!fileExists(path)) return -1;

	char tmpbuf[50];
	sprintf(tmpbuf, "tail -n1 %s | awk '{print $2}' >> o.txt", path);
	system(tmpbuf);

	FILE *fp = fopen("o.txt", "r");
	fgets(tmpbuf, 5, fp);
	if(strlen(tmpbuf) == 5) tmpbuf[5] = '\0';

	system("rm o.txt");
	return atoi(tmpbuf);
}

void writeMyDetails(char *path) 
{
	char tmpbuf[50];
	sprintf(tmpbuf, "echo \"server %d\" >> %s", PORT, path);

	printf("%s", path);
	system(tmpbuf);
}

int compare(THREADINFO *a, THREADINFO *b) { return a->sockfd - b->sockfd; }
 
void list_init(LLIST *ll) {
    ll->head = ll->tail = NULL;
    ll->size = 0;
}
 
int list_insert(LLIST *ll, THREADINFO *thr_info) {
    if(ll->size == CLIENTS) return -1;
    if(ll->head == NULL) {
        ll->head = (LLNODE *)malloc(sizeof(LLNODE));
        ll->head->threadinfo = *thr_info;
        ll->head->next = NULL;
        ll->tail = ll->head;
    }
    else {
        ll->tail->next = (LLNODE *) malloc(sizeof(LLNODE));
        ll->tail->next->threadinfo = *thr_info;
        ll->tail->next->next = NULL;
        ll->tail = ll->tail->next;
    }
    ll->size++;
    return 0;
}
 
int list_delete(LLIST *ll, THREADINFO *thr_info) {
    LLNODE *curr, *temp;
    
	if(ll->head == NULL) return -1;
    
	if(compare(thr_info, &ll->head->threadinfo) == 0) {
        temp = ll->head;
        ll->head = ll->head->next;
        if(ll->head == NULL) ll->tail = ll->head;
        free(temp);
        ll->size--;
        return 0;
    }
    
	for(curr = ll->head; curr->next != NULL; curr = curr->next) {
        if(compare(thr_info, &curr->next->threadinfo) == 0) {
            temp = curr->next;
            if(temp == ll->tail) ll->tail = curr;
            curr->next = curr->next->next;
            free(temp);
            ll->size--;
            return 0;
        }
    }
    return -1;
}
 
void list_dump(LLIST *ll) {
    LLNODE *curr;
    THREADINFO *thr_info;
    printf("Connection count: %d\n", ll->size);
    
	for(curr = ll->head; curr != NULL; curr = curr->next) {
        thr_info = &curr->threadinfo;
        printf("[%d] %s\n", thr_info->sockfd, thr_info->alias);
    }
}
 
 
int init_as_client(int sport)
{
	int cfd = socket(AF_INET, SOCK_STREAM, 0);

	if(cfd == -1) return -1;

	struct sockaddr_in serv_addr;    

	serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(sport);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(cfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		return -1;

	isconnected = 1;
	PACKET packet;



	return 0;
}	
 
void *io_handler(void *param) {
    char option[OPTLEN];
    while(scanf("%s", option) == 1) {
        if(!strcmp(option, "\\EXIT")) {
            printf("Terminating server...\n");
			pthread_mutex_destroy(&clientlist_mutex);
            close(sockfd);
            exit(0);
        }
        else if(!strcmp(option, "\\LIST")) {
            pthread_mutex_lock(&clientlist_mutex);
            list_dump(&client_list);
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else {
            fprintf(stderr, "Unknown command: %s...\n", option);
        }
    }
    return NULL;
}
 
void *client_handler(void *fd) {
   
   	THREADINFO threadinfo = *(THREADINFO *)fd;
    PACKET packet;
    LLNODE *curr;
    int bytes, sent;
    while(1) {
        bytes = recv(threadinfo.sockfd, (void *)&packet, sizeof(PACKET), 0);
        
		if(!bytes) {
            fprintf(stderr, "Connection lost from [%d] %s...\n", threadinfo.sockfd, threadinfo.alias);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        
		printf("[%d] %s %s %s %u %s\n", threadinfo.sockfd, packet.option, packet.alias, packet.channel, (unsigned)packet.ptime, packet.buff);

        if(!strcmp(packet.option, "alias")) {
            printf("Set alias to %s\n", packet.alias);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(compare(&curr->threadinfo, &threadinfo) == 0) {
                    strcpy(curr->threadinfo.alias, packet.alias);
                    strcpy(threadinfo.alias, packet.alias);
                    break;
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "whisp")) {
            int i;
            char target[ALIASLEN];
            for(i = 0; packet.buff[i] != ' '; i++) packet.buff[i++] = 0;
            strcpy(target, packet.buff);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(target, curr->threadinfo.alias) == 0) {
                    PACKET spacket;
                    memset(&spacket, 0, sizeof(PACKET));
                    if(!compare(&curr->threadinfo, &threadinfo)) continue;
                    strcpy(spacket.option, "msg");
                    strcpy(spacket.alias, packet.alias);
                    strcpy(spacket.buff, &packet.buff[i]);
                    sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(PACKET), 0);
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "exit")) {
            printf("[%d] %s has disconnected...\n", threadinfo.sockfd, threadinfo.alias);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
        else {
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                PACKET spacket;
                memset(&spacket, 0, sizeof(PACKET));
                if(!compare(&curr->threadinfo, &threadinfo)) continue;
                strcpy(spacket.option, "msg");
                strcpy(spacket.alias, packet.alias);
                strcpy(spacket.buff, packet.buff);
				strcpy(spacket.channel, packet.channel);
				spacket.ptime = packet.ptime;
                sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(PACKET), 0);
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
    }
 
    close(threadinfo.sockfd);
 
    return NULL;
}

void *receiver(void *param) {
    int recvd;
    PACKET packet;
    
    printf("Waiting here [%d]...\n", sockfd);
    while(isconnected) {
       printf("in connected"); 
        recvd = recv(sockfd, (void *)&packet, sizeof(PACKET), 0);
        if(!recvd) {
            fprintf(stderr, "Connection lost from parent server...\n");
            isconnected = 0;
            close(sockfd);
            break;
        }
        if(recvd > 0) {
			printf("[%s] <%s>: %s\n", ctime(&packet.ptime), packet.alias, packet.buff);
        }
        memset(&packet, 0, sizeof(PACKET));
    }
    return NULL;
}
