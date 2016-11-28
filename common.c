#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


int creer_socket(int prop, int *port_num)
{
  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  /* renvoie le numero de descripteur */
  /* et modifie le parametre port_num */
   int fd = 0;
   struct sockaddr_in serv_addr;
   memset(&serv_addr,0,sizeof(serv_addr));


   serv_addr.sin_port=htons(0);
   serv_addr.sin_family=AF_INET;
   serv_addr.sin_addr.s_addr=INADDR_ANY;

   fd=socket(AF_INET,SOCK_STREAM,0);
   bind(fd, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr_in));
   *port_num=serv_addr.sin_port;
   return fd;
}

/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
