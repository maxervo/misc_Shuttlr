#include "common_impl.h"

struct hostent* get_server(const char *host_target);
void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no);

int main(int argc, char **argv)
{
  /*
  if (argc < 3)
  {
    usage();
  }*/


   /* processus intermediaire pour "nettoyer" */
   /* la liste des arguments qu'on va passer */
   /* a la commande a executer vraiment */

   /* creation d'une socket pour se connecter au */
   /* au lanceur et envoyer/recevoir les infos */
   /* necessaires pour la phase dsm_init */
   /*
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




*/


   /* Envoi du nom de machine au lanceur */

   /* Envoi du pid au lanceur */
   //pid_t pid = getpid();
   //printf("%d\n", pid);

   /* Creation de la socket d'ecoute pour les */
   /* connexions avec les autres processus dsm */
   //sock_dsm=creer_socket();
   //if(listen(sock_dsm, BACKLOG) < 0) {
    //perror("listen");
    //exit(EXIT_FAILURE);
  //}

   /* Envoi du numero de port au lanceur */
   /* pour qu'il le propage à tous les autres */
   /* processus dsm */

   /* on execute la bonne commande */
   //return 0;


  // for server
  int serv_sockfd;
  int port_no;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  //Verifying arguments
  if (argc < 3) {
    fprintf(stderr,"Program %s needs arguments regarding target server: hostname, port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  port_no = atoi(argv[2]);

  /*   SERVER - CLIENT */
  //Preparing
  serv_sockfd = create_socket();
  server = get_server(argv[1]);
  init_serv_address(server, &serv_addr, port_no);

  //Connect to server
  if ( connect(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {
    ERROR_EXIT("Error - connection");
  }

  char buffer[BUFFER_SIZE] = "Hello test";
  write(serv_sockfd, buffer, BUFFER_SIZE);
  
}

struct hostent* get_server(const char *host_target) {
  struct hostent *server = gethostbyname(host_target);	//Later on: use addrinfo (cf. gethostbyname considered deprecated, and for ipv6...etc)
  if (server == NULL) {
    fprintf(stderr, "Error: No such host\n");
    exit(EXIT_FAILURE);
  }

  return server;
}

void init_serv_address(struct hostent* server, struct sockaddr_in* serv_addr_ptr, int port_no) {
  memset(serv_addr_ptr, 0, sizeof(struct sockaddr_in));
  serv_addr_ptr->sin_family = AF_INET;
  memcpy(server->h_addr, &(serv_addr_ptr->sin_addr.s_addr), server->h_length);
  serv_addr_ptr->sin_port = htons(port_no);  //convert to network order
}
