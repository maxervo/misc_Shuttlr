#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>


/* autres includes (eventuellement) */

#define MAX_LEN_HOSTNAME 50
#define ERROR_EXIT(str) {perror(str);exit(EXIT_FAILURE);}
#define BACKLOG 20

/* definition du type des infos */
/* de connexion des processus dsm */
struct dsm_proc_conn  {
   int rank; // rank of the process
   int dsm_pid; // pid of the process
   
   /* a completer */
};
typedef struct dsm_proc_conn dsm_proc_conn_t;

/* definition du type des infos */
/* d'identification des processus dsm */
struct dsm_proc {
  pid_t pid;
  dsm_proc_conn_t connect_info;
};
typedef struct dsm_proc dsm_proc_t;

int create_socket(int prop, int *port_num);

char** create_pool_hosts(char *filename, int *num_procs);
void destroy_pool_hosts(char **pool, int num_procs);
