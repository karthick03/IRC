#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>

int sfd = 0;
int pid[10];
int fd[10];
int ind = 0, msg = 0;

struct sockaddr_in dsock;
int size_of_sockaddr;

void start_process();
void send_msg();

static void msghandler(int signo)
{
	msg = 1;
}

//Creates a socket and binds to a port
void create_socket()
{
	sfd = socket(AF_INET, SOCK_STREAM, 0);

	dsock.sin_addr.s_addr = INADDR_ANY;
	dsock.sin_port = htons(3512);
	dsock.sin_family = AF_INET;

	size_of_sockaddr = sizeof(struct sockaddr_in);
	if(bind(sfd, (struct sockaddr*) &dsock, sizeof(struct sockaddr)) < 0) perror("bind(): ");
	else printf("bind() : success\n");

	listen(sfd, 10);
}

//Accept connection and create new process to handle it
void accept_and_create()
{
	struct sockaddr_in conn;
	fd[ind] = 0;
	while(1)
	{
		fd[ind] = accept(sfd, (struct sockaddr*) &conn, &size_of_sockaddr);
		pid[ind] = fork();
		printf("%d\n", pid[ind]);

		if(pid[ind] == 0) start_process();
		ind++;
	}
}

//Takes care of server-client communication
void start_process()
{
	printf("New client with socket %d and pid %d\n", fd[ind], pid[ind]); 
	char buf[1000];
	char g[100];
	
	int t;

//	struct sigaction psa;
//	psa.sa_handler = msghandler;
//	sigaction(SIGALRM, &psa, NULL);
	
	while(1)
	{
	//	printf("kill ");
	//	kill(pid[ind], SIGUSR1);
		recv(fd[ind], &g, sizeof(g), 0);
		sprintf(buf, "echo -n \"%s\" >> messages.txt", g);
		system(buf);
		printf("%s", g);
//		kill(0, SIGALRM);
		FILE *fp = fopen("messages.txt" ,"r");
		char contents[2500];
		int i = 0, eof;
		while((eof = fgetc(fp)) != EOF) contents[i++] = eof;
		contents[i] = '\0';
		printf("%d\n", send(fd[ind], &contents, sizeof(contents), 0));
	}
	//	recv(sfd, &buf, sizeof(buf), 0);
	//	printf("%s\n", buf);

}

int main()
{
	FILE *fp = fopen("messages.txt", "r");
	if(fp != NULL) {
		system("rm messages.txt");
		system("touch messages.txt");
	}else {
		system("touch messages.txt");
	}
	create_socket();

	accept_and_create();


	return 0;
}

