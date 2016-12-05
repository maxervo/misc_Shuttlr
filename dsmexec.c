#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define MAX_PAGE_NUMBER
#define MAX_PAGE_SIZE
#define HOSTS_POOL_FILENAME "machinefile"

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
     //int j;
     //char *pointer;

     /* Mise en place d'un traitant pour recuperer les fils zombies*/
     /* XXX.sa_handler = sigchld_handler; */


     /* lecture du fichier de machines et number of proc to launch */

     pool_hosts = create_pool_hosts(HOSTS_POOL_FILENAME, &num_procs);
     printf("Num procs %s\n", pool_hosts[0]);
     destroy_pool_hosts(pool_hosts, num_procs);

     /* creation de la socket d'ecoute */
     // ?

     /* creation des fils */
     for(i = 0; i < num_procs ; i++) {

	/* creation du tube pour rediriger stdout */

  /*
  int pipe1[1];

  if(pipe1[1]!= 0)
  {
    fprintf(stderr, "Can't create the pipe.\n");
    return EXIT_FAILURE;
  }
  else
  {
    dup2(pipe[1], STDOUT_FILENO);
  }
  */

  /* creation du tube pour rediriger stderr */
  /*
  int pipe2[2];
  if(pipe2[2]!=0)
  {
    fprintf(stderr, "Can't create the pipe.\n")
  }
  else
  {
      dup2(pipe2[2], STDERR_FILENO);
  }*/

  pid = fork();
	if(pid == -1)
  {
    ERROR_EXIT("fork");
	}
	if (pid == 0) { /* fils */

	   /* redirection stdout */ /* fermer les extremites */ /* un seul sens : le pere recoit les infos du fils */

	   /* redirection stderr */

	   /* Creation du tableau d'arguments pour le ssh */
	   //char ssh_tab=[];
	   /* jump to new prog : */
	   /* execvp("ssh",newargv); */

	} else  if(pid > 0) { /* pere */
	   /* fermeture des extremites des tubes non utiles */
	   num_procs_creat++;
	}
     }


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
