#ifndef __CHATROOM_H_
#define __CHATROOM_H_

#define MAX_CLIENTS 5
#define IP_LENGTH 16

void initialize_chat_room();
void stop_chat_room();
int login_chatroom(int clt_sock, char *ip, int port);

#endif /* __CHATROOM_H_ */
