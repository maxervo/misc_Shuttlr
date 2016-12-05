#include "common_impl.h"

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    usage();
  }


   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */

  struct hostent {
  char    *h_name;
  char   **h_aliases;
  int      h_addrtype;
  int      h_length;
  char   **h_addr_list;
  }
  #define h_addr  h_addr_list[0]
  };

  struct hostent *addr_srv;
  struct in_addr addr;
  addr_srv = gethostbyname(argv[1]);// recuperer dans une structure hostent l'addresse ip du lanceur a partir de son nom
  addr.s_addr = *(u_long *) addr_srv->h_addr_list[0]; //recuperer dans une structure in_addr qui correspond au nom de la machine distante
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo)); // initialisation structure addrinfo
  hints.ai_socktype = SOCK_STREAM; // type : socket TCP
  struct addrinfo* infos_returned;







   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */
   pid_t pid = getpid();
   printf("%d\n", pid);

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   sock_dsm=creer_socket();
   if(listen(sock_dsm, BACKLOG) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   return 0;
}
