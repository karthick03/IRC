#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

GtkWidget *msg_view;
GtkWidget *chatbox;

struct sockaddr_in dsock;
int cli = 0, size = 0;

void termination_handler(int signum)
{
	printf("Signal detected\n");
	char g[100];
	printf("%d", recv(cli, &g, sizeof(g), 0));

	GtkTextBuffer *ms = gtk_text_view_get_buffer(GTK_TEXT_VIEW(msg_view));
	gtk_text_buffer_set_text(ms, g, -1);

}


static void print_hello(GtkWidget *view, gpointer data)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chatbox));
	GtkTextIter start, end;

	gtk_text_buffer_get_iter_at_offset(buffer, &start, 0);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, 100);
	char *txt = (char *)gtk_text_buffer_get_text(buffer, &start, &end, 0);
	g_print("%s",txt);
	
	GtkTextBuffer *ms = gtk_text_view_get_buffer(GTK_TEXT_VIEW(msg_view));

	gtk_text_buffer_set_text(ms, txt, -1);

	g_print("Hello, world!");

}

static void show_msg()
{
	char buf[1000];
	recv(cli, &buf, sizeof(buf), 0);
	printf("%s\n", buf);

	GtkTextBuffer *ms = gtk_text_view_get_buffer(GTK_TEXT_VIEW(msg_view));
	gtk_text_buffer_set_text(ms, buf, -1);

}

static void activate(GtkApplication *app, gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *button;

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "KHF: IRClient");
	gtk_window_set_default_size(GTK_WINDOW(window), 800,600);

	grid = gtk_grid_new();
	msg_view = gtk_text_view_new();
	chatbox = gtk_text_view_new();

	gtk_container_add(GTK_CONTAINER(window), grid);

	button = gtk_button_new_with_label("Send");
	g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);

	gtk_grid_attach(GTK_GRID(grid), msg_view, 0, 0, 2, 1); 
	gtk_grid_attach(GTK_GRID(grid), chatbox, 0, 500, 2, 1); 
	gtk_grid_attach(GTK_GRID(grid), button, 0,510, 2, 1); 

	gtk_widget_show_all(window);
}

void client()
{
	cli = socket(AF_INET, SOCK_STREAM, 0);

	dsock.sin_addr.s_addr = INADDR_ANY;
	dsock.sin_port = htons(3512);
	dsock.sin_family = AF_INET;

	size = sizeof(struct sockaddr_in);
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
	
	if(connect(cli, (struct sockaddr*) &dsock, size) < 0)
	{
	   	perror("connect(): ");
		exit(0);
	}

	signal(SIGINT, termination_handler);

	printf("My pid %d", getpid());	
	GtkApplication *app;

	app = gtk_application_new("org.khf.irc", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	int status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	while(1){}

	return status;
}


