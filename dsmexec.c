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

#define MAX_PAGE_NUMBER
#define MAX_PAGE_SIZE

#define ARG_USAGE_MOD 2     //-2: because bin/dsmexec machine_file
#define ARG_EXECVP_NAME 1        //+3: because ssh @addr cmd_argv  NULL
#define ARG_SSH_TARGET 1
#define ARG_EXECVP_NULL 1

void child_actor(int pipefd[], char *target, int cmd_argc, char *cmd_argv[]);

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

     char *machine_filename = argv[1];
     //int j;
     //char *pointer;

     /* Mise en place d'un traitant pour recuperer les fils zombies */
     /* XXX.sa_handler = sigchld_handler; */


     /* lecture du fichier de machines et number of proc to launch */
     pool_hosts = create_pool_hosts(machine_filename, &num_procs);
     printf("Procs %s\n", pool_hosts[0]);

     /* creation de la socket d'ecoute */
     // ?

    /* creation des fils */
    int *pipe_array = malloc(2*num_procs*sizeof(int *));   //2: pipefd[2]
    int *pipefd = NULL;

    for(i = 0; i < num_procs ; i++) {
      pipefd = pipe_array+2*i;
      pipe(pipefd);

      /* creation du tube pour rediriger stdout */
      pid = fork();
      if(pid == -1)
      {
        ERROR_EXIT("fork");

      } else if (pid == 0) { /* child */
        printf("child is %s\n", pool_hosts[i]);
        child_actor(pipefd, pool_hosts[i], argc-ARG_USAGE_MOD, argv+ARG_USAGE_MOD); //becomes writer

      } else if(pid > 0) { /* parent */
        /* fermeture des extremites des tubes non utiles */
        close(pipefd[1]); //becomes reader
        num_procs_creat++;
        wait(NULL);
      }
    }
    destroy_pool_hosts(pool_hosts, num_procs);


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



void child_actor(int pipefd[], char *target, int cmd_argc, char *cmd_argv[]) {
  //Var
  char **ssh_tab = NULL;
  int len_tab = cmd_argc + ARG_EXECVP_NAME + ARG_SSH_TARGET + ARG_EXECVP_NULL;   //+3: because ssh @addr cmd_argv  NULL

  //Becoming writer
  close(pipefd[0]);
  //dup2(pipefd[1],STDOUT_FILENO);    //redirect stdout
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
  execvp(ssh_tab[0], ssh_tab);
  //printf("Commande %s, then %s, %s, %s, %s\n", ssh_tab[0], ssh_tab[1], ssh_tab[2], ssh_tab[3], ssh_tab[4]);

  //Clean
  free(ssh_tab);
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
