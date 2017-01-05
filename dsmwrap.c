#include "common_impl.h"

int main(int argc, char **argv)
{

  /* /!\ pas de variables en milieu de bloc */
  /* /!\ pas de sizeof de tab */
  char machine_name[1024] ; //1024 = length max of the machine name
  int num_dsm_port; // number of the dsm port
  /* processus intermediaire pour "nettoyer" */
  /* la liste des arguments qu'on va passer */
  /* a la commande a executer vraiment */

  /* creation d'une socket pour se connecter au */
  /* au lanceur et envoyer/recevoir les infos */
  /* necessaires pour la phase dsm_init */

  struct hostent *addr_srv;
  //struct in_addr addr;
  struct addrinfo hints;

  addr_srv= gethostbyname(argv[1]); //first agrument of the struct hostent obtained (char *h_name)
  //addr.s_addr = *(unsigned long *) addr_srv->h_addr_list[0]; //recuperer dans une structure in_addr qui correspond au nom de la machine distante

  memset(&hints, 0, sizeof(struct addrinfo)); // initialisation structure addrinfo
  hints.ai_socktype=SOCK_STREAM; // type : socket TCP
  //struct addrinfo* infos_returned;

  /* Envoi du nom de machine au lanceur */
  gethostname(machine_name,1024);
  printf("%s\n",machine_name);

  /* Envoi du pid au lanceur */
  pid_t pid = getpid();
  printf("%d\n", pid);

  /* Creation de la socket d'ecoute pour les */
  /* connexions avec les autres processus dsm */
  int sock_dsm=create_socket(0,0);
  if(listen(sock_dsm, BACKLOG) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
   }

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */
   num_dst_port=ntohs(addr_srv.sin_port);
   printf("%d\n",num_dst_port);
   /* on execute la bonne commande */
   return 0;
}
