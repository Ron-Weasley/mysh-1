#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <pthread.h>


#define SOCK_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "Hello from client"

#include "commands.h"
#include "built_in.h"


static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */

typedef struct temp_name{int server_sock; int* end; int len; int* flag;} thread_data;


	


void test_socket(int * end, int server_sock) {
	char buf[256];
    int bytes_rec;
	strcpy(buf, "socket connection is success!!");
	printf("Sending data....\n");
	int rc = send(end[1], buf, strlen(buf), 0);
	if (rc == -1) {
	printf("SEND ERROR = %d\n", 1);
	close(end[1]);
	exit(1);
	}
	else {
	printf("Data sent!\n");
	}


    /*
	strcpy(buf, "this is second segment received from client!");
	printf("Sending data...\n");
	rc = send(end[1], buf, strlen(buf), 0);
	if(rc==-1){
	printf("SEND ERRPR = %d\n", 1);
	close(end[1]);
	exit(1);
	}
	else {
	printf("Data sent!\n");
	}
	*/


	printf("waiting to read...\n");
	bytes_rec = recv(end[0], buf, sizeof(buf), 0);
	if (bytes_rec == -1){
	printf("RECV ERROR: %d\n", 1);
	close(server_sock);
	close(end[0]);
	exit(1);
	}
	else {
	printf("DATA RECEIVED = %s\n", buf);
	}

	
}

void *server(void * Data1)
{
 int rc;
 thread_data* my_data = (thread_data*)Data1;
 struct sockaddr_un client_sockaddr;

 rc = listen(my_data->server_sock, 1);
 if (rc == -1){
     printf("LISTEN ERROR: %d\n", 1);
     close(my_data->server_sock);
     exit(1);
 }

*(my_data->flag)=1;
    
my_data->end[0] = accept(my_data->server_sock, (struct sockaddr *) &client_sockaddr, &my_data->len);
        if (my_data->end[0] == -1){
        printf("ACCEPT ERROR: %d\n", 1);
        close(my_data->server_sock);
        close(my_data->end[0]);
        exit(1);
        }
}
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
	//Initialize
	pid_t pid;
	int server_sock, len, rc;
    int end[2];
    int bytes_rec = 0;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	char buf[256];
	int backlog = 10;
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(buf, 0, 256);

	//Server
	server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_sock == -1) {
		printf("SOCKET ERROR: %d\n", 1);
		exit(1);
	}

	server_sockaddr.sun_family = AF_UNIX;
	strcpy(server_sockaddr.sun_path, SOCK_PATH);
	len = sizeof(server_sockaddr);
	unlink(SOCK_PATH);
	rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
	if (rc == -1) {
		printf("BIND ERROR: %d\n", 1);
		close(server_sock);
		exit(1);
	}

	//Listen & Accept
	int flag = 0;
	pthread_t threads[2];
	thread_data Data1 = {
		server_sock, end, len, &flag
	};
	rc = pthread_create(&threads[1], NULL, server, (void *)&Data1);
	if (rc) {
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	//Client
	end[1] = socket(AF_UNIX, SOCK_STREAM, 0);
	if (end[1] == -1) {
		printf("SOCKET ERROR = %d\n", 1);
		exit(1);
	}
	client_sockaddr.sun_family = AF_UNIX;
	strcpy(client_sockaddr.sun_path, CLIENT_PATH);
	len = sizeof(client_sockaddr);
	unlink(CLIENT_PATH);
	rc = bind(end[1], (struct sockaddr *) &client_sockaddr, len);
	if (rc == -1) {
		printf("BIND ERROR: %d\n", 1);
		close(end[1]);
		exit(1);
	}

	//Connect
	while (flag == 0);
	rc = connect(end[1], (struct sockaddr *) &server_sockaddr, len);
	if (rc == -1) {
		printf("CONNECT ERROR = %d\n", 1);
		close(end[1]);
		exit(1);
	}
   

	// test_socket(end, server_sock);

	/*Test redirection(between to)
    int in = dup(0);
    int out = dup(1);
   
    strcpy(buf, " test fails!! ");
    if(fork()==0){
    int t =1000;
    wait(&t);
    dup2(end[0], 0);
    read(0, buf, sizeof(buf));
    dup2(in, 0);
    write(1, buf, sizeof(buf));
    exit(1);
    }else{
    dup2(end[1], 1);
    strcpy(buf, " test success!! \n");
    write(1, buf, sizeof(buf));
    dup2(out, 1);
    }
    */

     
 


  if (n_commands > 0) {

    struct single_command* com = (*commands);
    char path[6][256]={"", "/usr/local/bin/", "/usr/bin/", "/bin/", "/usr/sbin/", "/sbin/"};
    assert(com->argc != 0);
        int in=dup(0);
        int out = dup(1);
  if(n_commands==1){
      int i=0;  
      bg_pid=0;
          if(strcmp((com+i)->argv[(com+i)->argc-1],"&")==0){
           (com+i)->argc--;
           (com+i)->argv[(com+i)->argc]=NULL;
           bg_pid=1;
           }
   int built_in_pos = is_built_in_command((com+i)->argv[0]);
    if (built_in_pos != -1)
    {
      if (built_in_commands[built_in_pos].command_validate((com+i)->argc, (com+i)->argv))
      {
        if (built_in_commands[built_in_pos].command_do((com+i)->argc, (com+i)->argv) != 0)
        {
          fprintf(stderr, "%s: Error occurs\n", (com+i)->argv[0]);
        }
      }
      else {
        fprintf(stderr, "%s: Invalid arguments\n", (com+i)->argv[0]);
        close(server_sock);
        close(end[0]);
        close(end[1]);
        return -1;
      }
    }
    else if (strcmp((com+i)->argv[0], "") == 0) {
        close(server_sock);
        close(end[0]);
        close(end[1]);
        return 0;
    }
    else if (strcmp((com+i)->argv[0], "exit") == 0) {
        close(server_sock);
        close(end[1]);
        close(end[0]);
        return 1;
    }else{
          
           if(fork()==0){ 	
           for(int j=0;j<6;j++){
           strcat(path[j], (com+i)-> argv[0]);
           execv(path[j], (com+i)->argv);
           }
       } else if(bg_pid==0){wait(NULL);}
    } 
  }else{
      for(int i=0;i<n_commands;i++){
      
    int built_in_pos = is_built_in_command((com+i)->argv[0]);
    int pid;
    if((pid=fork())==0){
            if(i==0){dup2(end[1], 1);}
            else if((i+1)!=n_commands){dup2(end[0],0);dup2(end[1], 1); }
            else {dup2(end[0], 0);}
    if (built_in_pos != -1)
    {
      if (built_in_commands[built_in_pos].command_validate((com+i)->argc, (com+i)->argv))
      {
        if (built_in_commands[built_in_pos].command_do((com+i)->argc, (com+i)->argv) != 0)
        {
          fprintf(stderr, "%s: Error occurs\n", (com+i)->argv[0]);
        }
      }
      else {
        fprintf(stderr, "%s: Invalid arguments\n", (com+i)->argv[0]);
        close(server_sock);
        close(end[0]);
        close(end[1]);
        return -1;
      }
    }
    else if (strcmp((com+i)->argv[0], "") == 0) {
        close(server_sock);
        close(end[0]);
        close(end[1]);
        return 0;
    }
    else if (strcmp((com+i)->argv[0], "exit") == 0) {
        close(server_sock);
        close(end[1]);
        close(end[0]);
        return 1;
    }else{ 
           for(int j=0;j<6;j++){
           strcat(path[j], (com+i)-> argv[0]);
           execv(path[j], (com+i)->argv);
           }
       }
    exit(1);
    }else if(pid!=-1) {if((i+1)!=n_commands) waitpid(pid,NULL,0); }          
             
          else  {printf("fork failed\n"); }
     }    
    } 
  

  }
          

  
 
        
close(server_sock);
close(end[0]);
close(end[1]);

return 0;
}
void free_commands(int n_commands, struct single_command (*commands)[512])
{
      for (int i = 0; i < n_commands; ++i) {
              struct single_command *com = (*commands) + i;
                  int argc = com->argc;
                      char** argv = com->argv;

                          for (int j = 0; j < argc; ++j) {
                                    free(argv[j]);
                                        }

                              free(argv);
                                }

        memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
