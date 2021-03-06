#include "common.h"
#include <signal.h>
#include <stdio.h>

#define MAG   "\x1B[35m"
#define RESET "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"


char *srv_name = "localhost";
int clt_sock;

int DFLAG;

void sig_handler(int s)
{
  msg_t msg;

  switch (s) 
    {
    case SIGINT:
      msg.code = END_OK;
      msg.size = 0;
      
      send(clt_sock, &msg, HEADSIZE, 0);
  
      close(clt_sock);
      exit(EXIT_SUCCESS);
    default: return;
    }
}

/* Établie une session TCP vers srv_name sur le port srv_port
 * char *srv_name: nom du serveur (peut-être une adresse IP)
 * int srv_port: port sur lequel la connexion doit être effectuée
 *
 * renvoie: descripteur vers le socket
 */ 
 
int connect_to_server(char *srv_name, int srv_port){
  /* Code nécessaires à la création d'une socket en
     écoute : 
     
     - résolution du nom avec gethostbyname
     
     - appel à socket() 
     
     - appel à connect()
     
     avec les bons paramètres et contrôles d'erreurs.

     La fonction retourne l'identifiant de la socket cliente ou -1 en cas d'erreur
  */
  
  int clt_sock=socket(AF_INET, SOCK_STREAM, 0);
  struct hostent *host;
  host=gethostbyname(srv_name);
  struct sockaddr_in socket_addr;
  struct in_addr ip_addr;
  memcpy(&(ip_addr.s_addr), host->h_addr_list[0], host->h_length);
  socket_addr.sin_family = AF_INET;
  socket_addr.sin_port = htons(srv_port);
  socket_addr.sin_addr = ip_addr;
  
  if (connect(clt_sock, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1){
    perror("connect");
  }
  
  return clt_sock;
}

int authenticate(int clt_sock){

  /* Code nécessaire à l'authentification auprès du serveur :

     - attendre un paquet AUTH_REQ

     - répondre avec un paquet AUTH_RESP
     
     - attendre un paquet ACCESS_OK / ACCESS_DENIED / AUTH_REQ

     - agir en conséquence ...

  */
  unsigned char code;
  unsigned char size;
  recv_msg(clt_sock,&code, &size, NULL);
 
  // Si on ne reçoit pas de code d'authentification
  if(code!=AUTH_REQ){
    DEBUG("Pas de requete d'authentication..");
    return -1;
  }
  char login[BUFFSIZE];
  printf("Veuillez entrer votre idenfiant:");
  scanf("%s",login);
  
  send_msg( clt_sock, AUTH_RESP, strlen(login)+1, login);
  //DEBUG("AUTH_RESP envoyé");
  recv_msg(clt_sock, &code, &size, NULL);
  //DEBUG("ACCESS reçu");
  if(code==AUTH_REQ){
    DEBUG("la réponse d'authentification n'a pas été prise en compte..");
    return -1;
  }
  else if(code==ACCESS_DENIED){
    printf("Cet identifiant n'est pas correct!!!! è_é\n");
    return -1;
  }
  else{
    printf("Authentification réussie\n");
    return 1;
  }
  
}

int instant_messaging(int clt_sock){
  
  
  fd_set rset;
  unsigned char code;
  unsigned char size;
  char *data;

  //Ce flag permet d'éviter le problème du message vide initial
  int flag = 0;
  
  while(1){
    

    FD_ZERO(&rset);
    FD_SET(clt_sock, &rset);
    FD_SET(STDIN_FILENO, &rset);
    
    
    /* pour les étapes 2 à 4 se contenter de recevoir les messages
       envoyés par le serveur et les afficher à l'utilisateur
    */

    if (select(clt_sock+1, &rset, NULL, NULL, NULL) < 0){
       PERROR("select");
       exit(EXIT_FAILURE);
     }
    
    if (FD_ISSET(STDIN_FILENO, &rset)){
    /* l'utilisateur a tapé un nouveau message */
       //DEBUG("STDIN_FILENO isset");
       data = malloc(BUFFSIZE);
       
       if (fgets(data, BUFFSIZE, stdin) == NULL){
       /* gérer feof et ferror */

          //   <COMPLÉTER>
    
         return 0;
       }
       strtok(data,"\n");
       size = strlen(data)+1;

       //Le code suivant permet de se débarrasser du message vide initial
       if (size == 2 && flag == 0) {
         flag = 1;
         continue;
       }
      
       send_msg(clt_sock, MESG, size, data);
       //DEBUG("debug du message :%s,%d",data,size);
       free(data);
    }

    if (FD_ISSET(clt_sock, &rset)){
      /* réception d'un message du serveur */
      /* expected: <code><datalen>[<data>] */
      //DEBUG("CLT_SOCK isset");
	  data = malloc(BUFFSIZE);
	  
      recv_msg(clt_sock, &code, &size, &data);
	  
	  //int index = strchr(data,':')-data;
	  char *token;
	  strtok_r(data,":", &token);
	  
	  	
	  printf(KCYN "%s:" RESET "%s\n", data, token);
	  free(data);
	  if (code == END_OK) {
          return 0;
	  }
      
    }
    
  } /* while (1) */

  return 0;
}

int main(int argc, char *argv[]){
  char* srv_name="localhost";
  int srv_port = 4444;

  DFLAG = 1;

  signal(SIGINT, sig_handler);
  
  clt_sock = connect_to_server(srv_name, srv_port);
  if (clt_sock < 0)
    exit(EXIT_FAILURE);

  if (authenticate(clt_sock) < 0){
    close(clt_sock);
    eprintf("connexion closed\n");
    exit(EXIT_FAILURE);
  }

  if (instant_messaging(clt_sock) < 0){
    close(clt_sock);
    eprintf("connexion closed\n");
    exit(EXIT_FAILURE);
    }
  unsigned char code;
  unsigned char size;
  char* body;
  body=malloc(BUFFSIZE);
  ssize_t messagerecu=recv_msg(clt_sock, &code, &size, &body);
  if (messagerecu==-1)
    DEBUG("message non recu\n");
  printf("%s\n",body);
  free(body);
  //chatroom();
  close(clt_sock);
  eprintf("connexion closed\n");
  exit(EXIT_SUCCESS);
}
