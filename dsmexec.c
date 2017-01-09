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
ev_io remote_process_watcher;
ev_io init_transmit_watcher;

static void remote_process_cb(EV_P_ ev_io *watcher, int revents);
static void init_interact_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
static void init_accept_cb(EV_P_ ev_io *watcher, int revents);
void child_actor(int pipefd[], char *target, int cmd_argc, char *cmd_argv[]);
void monitor_loop(int master_sockfd, int pipe_array[], int num_procs);
void init_serv_address(struct sockaddr_in *serv_addr_ptr, int port_no);
void do_bind(int sockfd, struct sockaddr_in *serv_addr_ptr);

//creer processus et tubes pour le 28/11
// récuperation de l'entrée des processus

//processus fils->processus distant

/* variables globales */

/* un tableau gerant les infos d'identification */
/* des processus dsm */
dsm_proc_t *proc_array = NULL;

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

        //launch monitoring loop after last one filled
        if (i == num_procs-1) {
          sleep(1); //TODO

          /*
          printf("ok ready\n");
          char buffer[50] = {'\0'};
          read(master_sockfd, buffer, 50);
          printf("here %s\n", buffer);
          sleep(50);*/

          monitor_loop(master_sockfd, pipe_array, num_procs);
        }
      }
    }

    destroy_pool_hosts(pool_hosts, num_procs);
    //ev_loop_destroy (EV_DEFAULT_UC);
    //free(pipe_array);


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

void monitor_loop(int master_sockfd, int pipe_array[], int num_procs) {

  //default libev loop
  struct ev_loop *loop = EV_DEFAULT;

  //Exchanging data through transmission canals at initialization
  ev_io_init(&init_transmit_watcher, init_accept_cb, master_sockfd, EV_READ);
  ev_io_start(loop, &init_transmit_watcher);

  for (int i = 0; i < num_procs; i++) {                    //STDIN_FILENO
    //init watcher on stdout/stderr pipe                  //pipe_array[2*i] TODO
    ev_io_init(&remote_process_watcher, remote_process_cb, pipe_array[2*i], EV_READ);   //pipe_array[0], pipe_array[2]...etc : because parent is reader so reading fd is monitored
    ev_io_start(loop, &remote_process_watcher);
  }

  //waiting loop for events
  ev_run(loop, 0);

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
  struct ev_io *watcher_cli = (struct ev_io*) malloc (sizeof(struct ev_io));

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

  //Init and start watcher to read client requests TODO write
  ev_io_init(watcher_cli, init_interact_cb, cli_sockfd, EV_READ);
  ev_io_start(loop, watcher_cli);

  printf("Successfully connected to client.\n");
}

/* Read client message */
static void init_interact_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  char buffer[BUFFER_SIZE]; memset(buffer, 0, BUFFER_SIZE);
  int reception_control;

  if(EV_ERROR & revents)
  {
    perror("got invalid event");
    return;
  }

  // Receive message from client socket
  reception_control = recv(watcher->fd, buffer, BUFFER_SIZE, 0); //TODO do_recv
  if(reception_control < 0)
  {
    perror("read error");
    return;
  }

  //Socket closing
  if(reception_control == 0)
  {
    // Stop and free watchet if client socket is closing
    ev_io_stop(loop,watcher);
    free(watcher);
    perror("peer might closing");
    return;
  }

  //Receive data
  else
  {
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
  //dup2(pipefd[1],STDERR_FILENO);    //redirect stderr

  //Building exec ssh arguments
  ssh_tab = malloc(len_tab*sizeof(char *));
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
  sleep(2000);  //TODO
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
