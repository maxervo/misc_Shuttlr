#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>

/* autres includes (eventuellement) */
#define BUFFER_SIZE 100
#define MAX_LEN_HOSTNAME 50
#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank;
   int port;
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  int fd;   //for connection identification
  pid_t pid;
  char *hostname;
  int hostname_len;  //protects against buffer overflows when exchanging length/hostanme during init
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int creer_socket(int type, int *port_num);

char** create_pool_hosts(char *filename, int *num_procs);
void destroy_pool_hosts(char **pool, int num_procs);

void do_send(int sockfd, char *buffer, int buffer_size);
int do_recv(int sockfd, char *buffer, int buffer_size);

//TODO tmp replace by Joseph
int create_socket();
