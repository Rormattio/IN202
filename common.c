#include "common.h"
#include <errno.h>

/* send_msg send a message on socket sock
   sock: the socket
   code: message's protocol code
   size: message's size
   msg: message to be sent
*/
int send_msg(int sock, unsigned char code, unsigned char size, char *body) 
{
  msg_t msg;
  
  msg.code = code;
  msg.size = size;
  
  ssize_t send1=send(sock, &msg, sizeof(msg_t), 0);
  printf("Oh dear, something went wrong with send1()! %s\n", strerror(errno));
  
  //DEBUG("send1: code:%d,size:%d", code, size);
  if (body==NULL) {
        //DEBUG("le message envoyé est vide");
		return 0;
  }
  
  //ssize_t send2=send(sock, body, size, 0);
  char * mes = "fgfgf11";
  DEBUG("JE PRINT LE BODYYYY %s", body);
  ssize_t send2=send(sock, body, size, 0);
   printf("Oh dear, something went wrong with send2()! %s\n", strerror(errno));
  if (send1==-1 || send2==-1){
    DEBUG("Message 2 non envoyé");
    return -1;
  }
  return 0;
}

/* recv_msg recv a message from the socket sock
   sock: the socket
   code: message's protocol code
   size: message's size
   msg: message to be received
*/
int recv_msg(int sock, unsigned char *code, unsigned char *size, char **body) 
{
  msg_t msg;
  ssize_t rcv1=recv(sock, &msg, sizeof(msg_t), 0);
  
  if (rcv1 == -1) {
    DEBUG("message 1 non reçu");
    return -1;
  }
  
  *code=msg.code;
  
  if (body==NULL || size==NULL) {
        DEBUG("le message reçu est vide");
		return 0;
  }
  
  *size=msg.size;
  *body = malloc(*size * sizeof(char));
  
  ssize_t rcv2=recv(sock, *body, BUFFSIZE, 0);
  DEBUG("RCV BODY     %s\n", *body);
  if (rcv2 == -1) {
    DEBUG("message 2 non reçu");
    return -1;
  }
    
  if (rcv1==-1 || rcv2==-1){
    return -1;
  }
      
  return 0;
}
