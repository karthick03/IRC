#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>

struct sockaddr_in dsock;
int cli = 0, size = 0;
char msg[2500];
char name[20];
int count = 0;

void print_intro();
void print_messages();
static void show_msg();
void client();
void add_user();

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

	printf("%s", msg);
}


void add_message(char s[])
{
	char g[100];
	sprintf(g, "echo \"client %d\" >> users.txt", getpid());
	
	int t = getpid();
	send(cli, &t, sizeof(t), 0);
	send(cli, &g, sizeof(g), 0);
}

void print_intro()
{
	int i;
	for(i = 0; i <= 16; i++) printf("* ");
	printf("\n");

	printf("*\t\t\t\t*\n*\t\t\t\t*\n*\t\t\t\t*\n*\t\tIRC\t\t*\n*\t\tBy\t\t*\n*\t\tFaizaan\t\t*\n*\t\tKarthick\t*\n*\t\tHarish\t\t*\n*\t\t\t\t*\n*\t\t\t\t*\n*\t\t\t\t*\n");
	
	for(i = 0; i <= 16; i++) printf("* ");
	printf("\n");
	
	printf("Welcome to the IRC Network\n");
	printf("Enter your name : ");
	scanf("%s", name);
}

char *format_time(char *tim)
{
	char cti[10];
	int i,j;
	for(i = 0, j = 11; i < 9 ; i++, j++) cti[i] = tim[j];

	return cti;
}

char *format_msg(char *tim, char *ini)
{
	char final_msg[120];
	int i = 0, j =0;
	final_msg[i++] = '[';

	for(; j < 9; i++, j++) final_msg[i] = tim[j];

	final_msg[i++] = ']';
	final_msg[i++] = '\t';
	final_msg[i++] = '<';

	j = 0;
	int len = strlen(name);
	for(; j < len; i++, j++) final_msg[i] = name[j];

	final_msg[i++] = '>';
	final_msg[i++] = ' ';

	j = 0;
	len = strlen(ini);
	for(; j < len; i++, j++) final_msg[i] = ini[j];

	final_msg[i] = '\0';
	return final_msg;
}

int main(int argc, char **argv)
{
	client();
	
	char g[100], tim_now[100];
	print_intro();
	
	time_t currtime;	
	while(1) 
	{
	/*	if(count == 20)
		{
			count--;
			int j;
//			for(j = 0; j < 19; j++) strcpy(msg[j], msg[j+1]);
			strcpy(msg[j], "\0");
		}*/
		
		printf("<%s> ", name);
		fgets(msg, 100 , stdin);
		if(!strcmp(msg, "\n")) 
		{
			print_messages();
			continue;
		}
		currtime = time(NULL);
		strcpy(tim_now, ctime(&currtime));
		strcpy(tim_now, format_time(tim_now));
		strcpy(tim_now, format_msg(tim_now, msg));

		send(cli, tim_now, sizeof(tim_now), 0);
		recv(cli, &msg, sizeof(msg), 0);
//		strcpy(msg, g);
		
		//add_message(g);
		print_messages();
	}

	return 0;
}

