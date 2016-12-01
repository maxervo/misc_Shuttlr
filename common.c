#include "common_impl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<errno.h>

int creer_socket(int prop, int *port_num)
{
  /* fonction de creation et d'attachement */
  /* d'une nouvelle socket */
  /* renvoie le numero de descripteur */
  /* et modifie le parametre port_num */
  int fd = 0;
  if(prop==1)
  {
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_port=htons(*port_num);
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    fprintf(stdout,"Trying to connect with port_num %d\n",*port_num);
    fd=socket(AF_INET,SOCK_STREAM,0);
    if (fd == -1)
    {
      perror("socket");
      exit(EXIT_FAILURE);
    }
  }

  return fd;
 }


 /*while(connect(fd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) == -1)
   printf("Socket %d connected in TCP/IP mode \n", fd);
 {
   perror("connect");


 /*{ //Listenning socket
   struct sockaddr_in addr_listen;
   fd = socket(AF_INET, SOCK_STREAM, 0);
   fprintf(stdout,"La socket %d est maintenant ouverte en mode TCP/IP\n", fd);


   addr_listen.sin_family = AF_INET;
   addr_listen.sin_port = htons(*port_num);
   addr_listen.sin_addr.s_addr = INADDR_ANY;

   while (bind(fd, (struct sockaddr *) &addr_listen, sizeof(addr_listen)) == -1)
   {
   perror("bind");
   }
   fprintf(stdout,"bind success\n");
   if (listen(fd,listen_backlog) == -1)
   {
   perror("listen");
   return -1;
   }
   fp









/* Vous pouvez ecrire ici toutes les fonctions */
/* qui pourraient etre utilisees par le lanceur */
/* et le processus intermediaire. N'oubliez pas */
/* de declarer le prototype de ces nouvelles */
/* fonctions dans common_impl.h */
