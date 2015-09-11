#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

struct sockaddr_in dsock;
int cli = 0, size = 0;
char msg[10][100];
int count = 0;

void print_messages();
void sig_handle(int signum);

void sig_handle(int signum)
{
	    printf("Signal detected\n");
		char g[100];
		recv(cli, &g, sizeof(g), 0);
		strcpy(msg[count++], g);
		print_messages();
}

static void show_msg()
{
	char buf[1000];
	recv(cli, &buf, sizeof(buf), 0);
	printf("%s\n", buf);

}

void client()
{
	cli = socket(AF_INET, SOCK_STREAM, 0);

	dsock.sin_addr.s_addr = INADDR_ANY;
	dsock.sin_port = htons(3512);
	dsock.sin_family = AF_INET;

	size = sizeof(struct sockaddr_in);
	
	if(connect(cli, (struct sockaddr*) &dsock, size) < 0)
	{
	   	perror("connect(): ");
		exit(0);
	}
	else
		printf("Connected with server\n");
}

void print_messages()
{
	system("clear");

	int i = 0;
	for(; i < count;i++) printf("%s", msg[i]);

	for(; i <=10; i++, printf("\n"));	
}


void add_user()
{
	char g[100];
	sprintf(g, "echo \"client %d\" >> users.txt", getpid());
	
	int t = getpid();
	send(cli, &t, sizeof(t), 0);
	send(cli, &g, sizeof(g), 0);

}

int main(int argc, char **argv)
{
	client();
	
	signal(SIGUSR1, sig_handle);

//	add_user();

	printf("My pid %d\n", getpid());	

	char g[100];
		
	while(1) 
	{
		if(count == 10)
		{
			count--;
			int j;
			for(j = 0; j < 9; j++) strcpy(msg[j], msg[j+1]);
			strcpy(msg[j], "\0");
		}
		fgets(msg[count], 100 , stdin);
		send(cli, msg[count], sizeof(g), 0);
		recv(cli, &g, sizeof(g), 0);
		count++;
		strcpy(msg[count], g);
		print_messages();
	}

	return 0;
}

