#include "common.h"
#include "chatroom.h"
#include <signal.h>
#include <pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#define MAX_CONN 10            // nombre maximal de requêtes en attente


#include <netinet/ip.h>



int DFLAG;
int srv_sock;

void sig_handler(int s)
{
  switch (s) 
    {
    case SIGINT:
      stop_chat_room();
      sleep(1); /* wait for client to close */
      close(srv_sock);
      exit(0);
    case SIGPIPE: 
      break;
    default: break;
    }
}

int create_a_listening_socket(int srv_port, int maxconn){
  /* Code nécessaires à la création d'une socket en
     écoute : 

     - appel à socket() 

     - appel à bind()

     - appel à listen()

     avec les bons paramètres et contrôles d'erreurs.

     La fonction retourne l'identifiant de la socket serveur ou -1 en
     cas d'erreur.
  */
  int srv_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (srv_sock == -1) {
        DEBUG("Cant create Socket");
  }
  
  struct sockaddr_in socket_addr;
  struct in_addr ip_addr;
  socket_addr.sin_family = AF_INET;
  socket_addr.sin_port = htons(srv_port);
  ip_addr.s_addr = INADDR_ANY;
  socket_addr.sin_addr = ip_addr;
  
  int bind_succes=bind(srv_sock, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
  
  if (bind_succes==-1)
  {
    DEBUG("Couldnt bind\n");
    return -1;
  }
  
  listen(srv_sock, maxconn);
  return srv_sock;
}

int accept_clt_conn(int srv_sock, struct sockaddr_in *clt_sockaddr){
  /* Code nécessaire à l'acception d'une connexion sur
     la socket en écoute (passée en argument via le paramètre srv_sock :
     
     - appel à accept()
     
     avec les bons paramètres et contrôles d'erreurs.
     
     La fonction retourne l'identifiant de la socket cliente ou -1 en
     cas d'erreur.
     
  */
  int clt_socket;
  //struct sockaddr_in clt_socket_addr;
  socklen_t len;
  clt_socket=accept(srv_sock, (struct sockaddr *)&clt_sockaddr, &len);
  
  if (clt_socket==-1)
    {
    perror("accept");
    return -1;
    }
    
  DEBUG("connexion accepted");

  return clt_socket;
}

int main(void) 
{
  signal(SIGINT, sig_handler);
  signal(SIGPIPE, sig_handler);
  
  DFLAG = 1;

  /* create a listening socket */
  srv_sock = create_a_listening_socket(SRV_PORT, MAX_CONN);
  if (srv_sock < 0) 
    {
      DEBUG("failed to create a listening socket");
      exit(EXIT_FAILURE);
    }
  
  /* initialize the chat room with no client */
  initialize_chat_room();
    
  while (1){
    int clt_sock;
    struct sockaddr_in clt_sockaddr;
    char *clt_ip;
    int clt_port;
    
    /* wait for new incoming connection */
    if ((clt_sock = accept_clt_conn(srv_sock, &clt_sockaddr)) < 0 ) 
      {
	perror("accept_clt_conn");	
	exit(EXIT_FAILURE);
      }
    
    clt_ip = inet_ntoa(clt_sockaddr.sin_addr);
    clt_port = ntohs(clt_sockaddr.sin_port);
    
    /* register new buddies in the chat room */
    if ( login_chatroom(clt_sock, clt_ip, clt_port) != 0 ) 
      {
	DEBUG("client %s:%d not accepted", clt_ip, clt_port);	
	close(clt_sock);
	DEBUG("close clt_sock %s:%d", clt_ip, clt_port);
      }
    
    /*printf("lalala");
    char *t="tralalalalalalalalalalala youpla ";
    unsigned char code=31;
    ssize_t message=send_msg(clt_sock, code, strlen(t), t);*/
    
    /*close(clt_sock);
	DEBUG("close clt_sock %s:%d", clt_ip, clt_port);*/
    
  } /* while */

  return EXIT_SUCCESS;
}
