#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ev.h>

#define MAX_PAGE_NUMBER
#define MAX_PAGE_SIZE

#define ARG_USAGE_MOD 2     //-2: because bin/dsmexec machine_file
#define ARG_EXECVP_NAME 1        //+3: because ssh @addr cmd_argv  NULL
#define ARG_SSH_TARGET 1
#define ARG_EXECVP_NULL 1
#define MAX_FACTOR_QUEUE 5

//TODO location
// every watcher type has its own typedef'd struct with the name ev_TYPE
struct carrier_ev_io {
  ev_io io;
  int slave_port; //to identify where it comes from, locally
  int num_procs;
  dsm_proc_t *pool_remote_processes;
};
typedef struct carrier_ev_io carrier_ev_io_t;

carrier_ev_io_t remote_process_watcher;
carrier_ev_io_t init_transmit_watcher;

/*
struct remote_process {
  int rank;
  int slave_port; //to identify where it comes from, locally
  int pid;
  int inter_port;
  char *hostname;
};*/

static void remote_process_cb(EV_P_ ev_io *watcher, int revents);
static void init_interact_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
static void init_accept_cb(EV_P_ ev_io *watcher, int revents);
void child_actor(int pipefd[], char *target, int cmd_argc, char *cmd_argv[]);
void monitor_loop(int master_sockfd, dsm_proc_t *pool_remote_processes, int pipe_array[], int num_procs);
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);
void attach_cli_data(carrier_ev_io_t *watcher_cli, ev_io *watcher, struct sockaddr_in cli_addr);

dsm_proc_t *create_pool_remote_processes(int num_procs);
void destroy_remote_process(dsm_proc_t *pool, int num_procs);
int get_slave_rank(dsm_proc_t *pool_remote_processes, int slave_port, int num_procs);
void insert_pool_proc(dsm_proc_t *pool_remote_processes, int slave_port, int num_procs);
void handle_init_data(char *buffer, dsm_proc_t *dsm_proc, int slave_port, int num_procs);
int init_status(dsm_proc_t *pool_remote_processes, int num_procs);

//creer processus et tubes pour le 28/11
// récuperation de l'entrée des processus

//processus fils->processus distant

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
//dsm_proc_t *pool_remote_processes = NULL; global

/* le nombre de processus effectivement crees */
volatile int num_procs_creat = 0;

void usage(void)
{
  fprintf(stdout,"Usage : dsmexec machine_file executable arg1 arg2 ...\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}

void sigchld_handler(int sig)
{
   /* on traite les fils qui se terminent */
   /* pour eviter les zombies */
  int err_saved = errno;
  while (waitpid(-1,NULL,WNOHANG) > 0)
  {
    num_procs_creat=num_procs_creat-1;
  }
  errno = err_saved;
}


int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    usage();
  }
  else
  {
     pid_t pid;
     int num_procs = 0;
     int i;
     char **pool_hosts = NULL;
     dsm_proc_t *pool_remote_processes = NULL;
     int master_sockfd;
     struct sockaddr_in serv_addr;
     int port_no = 2500;  //TODO or let OS decide?

     char *machine_filename = argv[1];
     //int j;
     //char *pointer;

     /* Mise en place d'un traitant pour recuperer les fils zombies */
     /* XXX.sa_handler = sigchld_handler; */


     /* lecture du fichier de machines et number of proc to launch */
     pool_hosts = create_pool_hosts(machine_filename, &num_procs);
     printf("Procs %s\n", pool_hosts[0]);

     /* pool of remote processes */
     pool_remote_processes = create_pool_remote_processes(num_procs);

     /* creation de la socket d'ecoute + lecture effective */
     master_sockfd = create_socket();
     init_serv_address(&serv_addr, port_no);
     do_bind(master_sockfd, &serv_addr);
     if(listen(master_sockfd, MAX_FACTOR_QUEUE*num_procs) < 0) {
       ERROR_EXIT("Error - listen");
     }

    /* creation des fils */
    int *pipe_array = malloc(2*num_procs*sizeof(int *));   //2: pipefd[2]
    int *pipefd = NULL;

    for(i = 0; i < num_procs ; i++) {
      pipefd = pipe_array+2*i;
      pipe(pipefd);

      /* creation du tube pour rediriger stdout */
      pid = fork();
      if(pid < 0)
      {
        ERROR_EXIT("fork");

      } else if (pid == 0) { /* child */
        printf("child is %s\n", pool_hosts[i]);
        child_actor(pipefd, pool_hosts[i], argc-ARG_USAGE_MOD, argv+ARG_USAGE_MOD); //becomes writer

      } else if(pid > 0) { /* parent */
        /* fermeture des extremites des tubes non utiles */
        close(pipefd[1]); //becomes reader
        num_procs_creat++;

        //launch monitoring loop after last one filled    //TODO
        if (i == num_procs-1) {
          sleep(1); //TODO

          /*
          printf("ok ready\n");
          char buffer[50] = {'\0'};
          read(master_sockfd, buffer, 50);
          printf("here %s\n", buffer);
          sleep(50);*/

          monitor_loop(master_sockfd, pool_remote_processes, pipe_array, num_procs);
        }
      }
    }

    destroy_pool_hosts(pool_hosts, num_procs);
    //ev_loop_destroy (EV_DEFAULT_UC);
    //free(pipe_array);
    //destroy_remote_process


     for(i = 0; i < num_procs ; i++){
  /* on accepte les connexions des processus dsm */

	/*  On recupere le nom de la machine distante */
	/* 1- d'abord la taille de la chaine */
	/* 2- puis la chaine elle-meme */

	/* On recupere le pid du processus distant  */

	/* On recupere le numero de port de la socket */
	/* d'ecoute des processus distants */
     }

     /* envoi du nombre de processus aux processus dsm*/

     /* envoi des rangs aux processus dsm */

     /* envoi des infos de connexion aux processus */

     /* gestion des E/S : on recupere les caracteres */
     /* sur les tubes de redirection de stdout/stderr */
     /* while(1)
         {
            je recupere les infos sur les tubes de redirection
            jusqu'à ce qu'ils soient inactifs (ie fermes par les
            processus dsm ecrivains de l'autre cote ...)

         };
      */

     /* on attend les processus fils */

     /* on ferme les descripteurs proprement */

     /* on ferme la socket d'ecoute */
  }

  exit(EXIT_SUCCESS);
}

void monitor_loop(int master_sockfd, dsm_proc_t *pool_remote_processes, int pipe_array[], int num_procs) {
  //default libev loop
  struct ev_loop *loop = EV_DEFAULT;
  int init_finished = 0;  //init running

  //init_transmit_watcher.data = (void*) pipe_array;
  //printf("hi %d\n", pool_remote_processes[1].connect_info.rank);

  /* Waterfall : attach data to callbacks */
  //init_transmit_watcher
  init_transmit_watcher.num_procs = num_procs;
  init_transmit_watcher.pool_remote_processes = pool_remote_processes;

  //Exchanging data through transmission canals at initialization
  ev_io_init(&(init_transmit_watcher.io), init_accept_cb, master_sockfd, EV_READ);
  ev_io_start(loop, &(init_transmit_watcher.io));

  for (int i = 0; i < num_procs; i++) {                    //STDIN_FILENO
    //init watcher on stdout/stderr pipe                  //pipe_array[2*i] TODO
    ev_io_init(&remote_process_watcher.io, remote_process_cb, pipe_array[2*i], EV_READ);   //pipe_array[0], pipe_array[2]...etc : because parent is reader so reading fd is monitored
    ev_io_start(loop, &remote_process_watcher.io);
  }

  //waiting loop for events
  while (1) { //TODO conditions
    init_finished = init_status(pool_remote_processes, num_procs);

    //event loop
    ev_loop(loop, 0);

    //Send all DSM information to remote machines
    if (init_finished) {
      send_dsm_info();
    }
  }

}

static void remote_process_cb(EV_P_ ev_io *watcher, int revents)
{
  if(EV_ERROR & revents)
  {
    perror("invalid event detected");
    return;
  }
  int pipefd = watcher->fd;
  char buffer[50] = {'\0'};

  printf("stdin ready\n");
  read(pipefd, buffer, 50);
  printf("hey %s\n", buffer);

  //TODO if read = 0 then close and stop watcher

  //for one-shot event
  //ev_io_stop (EV_A_ watcher);

  //stop iterating all nested ev_run's
  //ev_break (EV_A_ EVBREAK_ALL);
}

static void init_accept_cb(EV_P_ ev_io *watcher, int revents)
{
  struct sockaddr_in cli_addr;
  socklen_t cli_len = sizeof(cli_addr);
  int cli_sockfd;
  //tmp struct ev_io *watcher_cli = (struct ev_io*) malloc (sizeof(struct ev_io));
  carrier_ev_io_t *watcher_cli = (carrier_ev_io_t *) malloc (sizeof(carrier_ev_io_t));  //Tcleaned at end of communication init_interact_cb

  if(EV_ERROR & revents)
  {
    perror("invalid event detected");
    return;
  }

  // Accept client request
  cli_sockfd = accept(watcher->fd, (struct sockaddr *)&cli_addr, &cli_len);
  if (cli_sockfd < 0)
  {
    perror("accept error");
    return;
  }

  /* Waterfall : attach data to client callback, and inserts process in pool */
  attach_cli_data(watcher_cli, watcher, cli_addr);

  //Init and start watcher to read client requests TODO write
  ev_io_init(&(watcher_cli->io), init_interact_cb, cli_sockfd, EV_READ);
  ev_io_start(loop, &(watcher_cli->io));

  printf("Successfully connected to client.\n");
}

/* Read client message */
static void init_interact_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {    //occupies my event loop, function here synchronous, will complete before next callback called by event-loop
  //reception variables
  char buffer[BUFFER_SIZE]; memset(buffer, '\0', BUFFER_SIZE);
  int reception_control;

  //extract data carried by callback
  carrier_ev_io_t *carrier_watcher = (carrier_ev_io_t *) watcher;
  dsm_proc_t *pool_remote_processes = carrier_watcher->pool_remote_processes;
  int slave_port = carrier_watcher->slave_port;
  int num_procs = carrier_watcher->num_procs;

  if(EV_ERROR & revents)
  {
    perror("got invalid event");
    return;
  }

  // Receive message from client socket
  reception_control = do_recv(watcher->fd, buffer, BUFFER_SIZE);
  if(reception_control < 0)
  {
    perror("read error");
    return;
  }

  //Socket closing
  if(reception_control == 0)
  {
    //Stop and free watchet if client socket is closing
    ev_io_stop(loop,watcher);
    free(watcher);
    perror("peer might be closing");
    return;
  }

  //Receive data
  else
  {
    int rank = get_slave_rank(pool_remote_processes, slave_port, num_procs);
    if (rank < 0) {
      perror("Not expected, client not inserted");
    }
    else {
      handle_init_data(buffer, pool_remote_processes+rank, slave_port, num_procs);  //considering only the connected process e.i current rank
    }

    printf("message:%s\n", buffer);
  }

  // Send message bach to the client
  //send(watcher->fd, buffer, read, 0);
  //bzero(buffer, read);
}

void child_actor(int pipefd[], char *target, int cmd_argc, char *cmd_argv[]) {
  //Var
  char **ssh_tab = NULL;
  int len_tab = cmd_argc + ARG_EXECVP_NAME + ARG_SSH_TARGET + ARG_EXECVP_NULL;   //+3: because ssh @addr cmd_argv  NULL

  //Becoming writer
  close(pipefd[0]);
  dup2(pipefd[1],STDOUT_FILENO);    //redirect stdout
  //dup2(pipefd[1],STDERR_FILENO);    //redirect stderr TODO

  //Building exec ssh arguments
  ssh_tab = malloc(len_tab*sizeof(char *));       //Later on possible to extend by doing custom ports cf. for use on docker containers port maps
  ssh_tab[0] = "ssh";
  ssh_tab[1] = target;
  for (int i = 0; i < cmd_argc; i++) {
    ssh_tab[i+ARG_EXECVP_NAME+ARG_SSH_TARGET] = cmd_argv[i];
  }
  ssh_tab[len_tab-1] = NULL;

  //Jump to new program
  sleep(2);   //TODO
  //execvp(ssh_tab[0], ssh_tab);
  //printf("Commande %s, then %s, %s, %s, %s\n", ssh_tab[0], ssh_tab[1], ssh_tab[2], ssh_tab[3], ssh_tab[4]);
  printf("testchild\n");

  //Clean
  sleep(2000);  //TODO normally ok remote processes will be launched as daemons so child fork not dying ok (or in dsmwrap send a special text to end it)
  free(ssh_tab);
}


/* Utilities */
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  serv_addr_ptr->sin_addr.s_addr = htonl(INADDR_ANY);  //INADDR_ANY : all interfaces - not just "localhost", multiple network interfaces OK
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}

void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr) {
  if ( bind(sockfd, (struct sockaddr *) serv_addr_ptr, sizeof(struct sockaddr_in))<0 ) {  //cast generic struct
    ERROR_EXIT("Error - bind");
  }
}

dsm_proc_t *create_pool_remote_processes(int num_procs)
{
  dsm_proc_t *pool;

  //Allocate memory
  pool = malloc(num_procs * sizeof(dsm_proc_t));  //possible improvement init the array with NULLs
  for (int i = 0; i < num_procs; i++) {
    pool[i].slave_port = 0;
    pool[i].pid = 0;
    pool[i].hostname = NULL;
    pool[i].hostname_len = 0;
    pool[i].connect_info.rank = i; //give rank in order
    pool[i].connect_info.port = 0;
  }

  return pool;
}

void destroy_remote_process(dsm_proc_t *pool, int num_procs) {
  for (int i = 0; i < num_procs; i++) {
    free(pool[i].hostname); //free each string
  }
  free(pool);  //free array
}

void attach_cli_data(carrier_ev_io_t *watcher_cli, ev_io *watcher, struct sockaddr_in cli_addr) {
  carrier_ev_io_t *carrier_watcher = (carrier_ev_io_t *) watcher;

  //waterfall transfer
  watcher_cli->num_procs = carrier_watcher->num_procs;
  watcher_cli->pool_remote_processes = carrier_watcher->pool_remote_processes;

  //attach port accepted for client
  watcher_cli->slave_port = cli_addr.sin_port;

  //inserts in process information into pool
  insert_pool_proc(watcher_cli->pool_remote_processes, watcher_cli->slave_port, watcher_cli->num_procs);
}

int get_slave_rank(dsm_proc_t *pool_remote_processes, int slave_port, int num_procs) {
  for (int i = 0; i < num_procs; i++) {
    if (pool_remote_processes[i].slave_port == slave_port) {
      return i; //rank found
    }
  }
  return -1; //neutral element for ports, so not defined
}

void insert_pool_proc(dsm_proc_t *pool_remote_processes, int slave_port, int num_procs) {
  for (int i = 0; i < num_procs; i++) {
    if (!pool_remote_processes[i].slave_port) { //empty slot
      pool_remote_processes[i].slave_port = slave_port; //reserved
    }
  }
}

void handle_init_data(char *buffer, dsm_proc_t *dsm_proc, int slave_port, int num_procs) {     //Further: possible to user json-c library to exchange all this data in json, more elegant but less low level
  //Sent in order, cf. note on synchronous function occupying the event loop (even if event-loop has asynchronous IO)

  //Hostname length
  if (!dsm_proc->hostname) {    //ATTENTION: byte order, big/little endian, scope statement states same architecture and no security, so normally careful for endianness, use type punning...etc
    dsm_proc->hostname_len = *(int *) buffer;
    dsm_proc->hostname = malloc( (dsm_proc->hostname_len+1) * sizeof(char));  //free during destroy_remote_process, +1: because string has '\0' element at the end, need clean
    memset(dsm_proc->hostname, '\0', dsm_proc->hostname_len+1);  //clear
    printf("len is %d\n", dsm_proc->hostname_len);
  }

  //Hostname string
  else if(strlen(dsm_proc->hostname) == 0) {
    strncpy(dsm_proc->hostname, buffer, dsm_proc->hostname_len);  //CRITICAL: protection against buffer overflows (non trusted users for length sent and next string)
    printf("string is %s\n", dsm_proc->hostname);
  }

  //Interconnection port for DSM
  else if(!(dsm_proc->connect_info.port)) {
    dsm_proc->connect_info.port = *(int *) buffer;  //idem endian, scope statement supposes it
    printf("co info is %d\n", dsm_proc->connect_info.port);
  }

  //Not expected
  else {
    perror("No more connections here expected from client");
  }
}

int init_status(dsm_proc_t *pool_remote_processes, int num_procs) {
  for (int i = 0; i < num_procs; i++) {
    if (!pool_remote_processes[i].connect_info.port) { //
      return 0; //still initializing
    }
  }
  return 1;
}



/* redirection stdout */ /* fermer les extremites */ /* un seul sens : le pere recoit les infos du fils */

/* redirection stderr */

/* Creation du tableau d'arguments pour le ssh */
//char ssh_tab=[];
/* jump to new prog : */
/* execvp("ssh",newargv); */


/*

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv)
{
  int pipe1[2];
  pid_t pid;

  pipe(pipe1);
  pid = fork();

  if( pid > 0) {
    close(pipe1[1]);
    dup2(pipe1[0],STDIN_FILENO);
    execlp("wc","wc","-l",NULL);
  }
  else if (0 == pid)
    {
      int pipe2[2];
      pid_t pid2;

      close(pipe1[0]);
      pipe(pipe2);
      pid2 = fork();

      if (pid2 > 0) {
	close(pipe2[1]);
	dup2(pipe1[1],STDOUT_FILENO);
	dup2(pipe2[0],STDIN_FILENO);
	execlp("grep","grep","truc",NULL);
      }
      else if ( 0 == pid2 )
	{
	  close(pipe1[1]);
	  close(pipe2[0]);
	  dup2(pipe2[1],STDOUT_FILENO);
	  execlp("cat","cat","toto.txt",NULL);
	}
    }

  return 0;
}


*/
