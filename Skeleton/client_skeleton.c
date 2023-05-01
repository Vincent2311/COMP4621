#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chatroom.h"

#define MAX 1024 // max buffer size
#define PORT 6789  // port number

static int sockfd;

void generate_menu(){
	printf("Hello dear user pls select one of the following options:\n");
	printf("EXIT\t-\t Send exit message to server - unregister ourselves from server\n");
    printf("WHO\t-\t Send WHO message to the server - get the list of current users except ourselves\n");
    printf("#<user>: <msg>\t-\t Send <MSG>> message to the server for <user>\n");
    printf("Or input messages sending to everyone in the chatroom.\n");
}

void recv_server_msg_handler() {
    /********************************/
	/* receive message from the server and desplay on the screen*/
	/**********************************/
	while (1)
	{
		char buffer[MAX];
		bzero(buffer, sizeof(buffer));
		if (recv(sockfd, buffer, sizeof(buffer), 0)==-1){
			perror("recv");
		}
		printf("%s\n", buffer);
	}
}

int main(){
    int n;
	int nbytes;
	struct sockaddr_in server_addr, client_addr;
	char buffer[MAX];
	
	/******************************************************/
	/* create the client socket and connect to the server */
	/******************************************************/
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1){
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created...\n");
		
	bzero(&server_addr, sizeof(server_addr));
	
	// assign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	
	// connect the client socket to the server socket
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server...\n");

	generate_menu();
	// recieve welcome message to enter the nickname
    bzero(buffer, sizeof(buffer));
    if (nbytes = recv(sockfd, buffer, sizeof(buffer), 0)==-1){
        perror("recv");
    }
    printf("%s\n", buffer);

	/*************************************/
	/* Input the nickname and send a message to the server */
	/* Note that we concatenate "REGISTER" before the name to notify the server it is the register/login message*/
	/*******************************************/
	char name[MAX];
	if (fgets(name, sizeof(name), stdin) == NULL) {
        	printf("Fail to read the input stream\n");
    }
    else {
        name[strcspn(name, "\n")] = '\0';
    	}
	while (strlen(name) > C_NAME_LEN)
	{
		printf("Input name exceeds max name lenght\n");
		scanf("%s",name);
	}
	bzero(buffer, sizeof(buffer));
	strcpy(buffer,"REGISTER");
	strcat(buffer, name);
	if (send(sockfd, buffer, sizeof(buffer), 0)<0){
		puts("Sending registration failed");
		exit(1);
	}
	printf("Register/Login message sent to server.\n");



    // receive welcome message "welcome xx to joint the chatroom. A new account has been created." (registration case) or "welcome back! The message box contains:..." (login case)
    bzero(buffer, sizeof(buffer));
    if (recv(sockfd, buffer, sizeof(buffer), 0)==-1){
        perror("recv");
    }
    printf("%s\n", buffer);

    /*****************************************************/
	/* Create a thread to receive message from the server*/
	/* pthread_t recv_server_msg_thread;*/
	/*****************************************************/
	pthread_t recv_server_msg_thread;
	if(pthread_create( &recv_server_msg_thread, NULL, (void*)recv_server_msg_handler, NULL)!=0)
    {
        printf("Create pthread error!\n");
		exit(1);
    }
    
	// chat with the server
	for (;;) {
		bzero(buffer, sizeof(buffer));
		n = 0;
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        	printf("Fail to read the input stream\n");
    	}
    	else {
        	buffer[strcspn(buffer, "\n")] = '\0'; 
    	}
		if ((strncmp(buffer, "EXIT", 4)) == 0) {
			printf("Client Exit...\n"); //TODO: ?
			/********************************************/
			/* Send exit message to the server and exit */
			/* Remember to terminate the thread and close the socket */
			/********************************************/
			if (send(sockfd, buffer, sizeof(buffer), 0)<0){
				puts("Sending MSG_EXIT failed");
				exit(1);
			}
			// pthread_join(recv_server_msg_thread,NULL);
			close(sockfd);
			printf("It's OK to Close the Window Now OR enter ctrl+c\n");
		}
		else if (strncmp(buffer, "WHO", 3) == 0) {
			printf("Getting user list, pls hold on...\n");
			if (send(sockfd, buffer, sizeof(buffer), 0)<0){
				puts("Sending MSG_WHO failed");
				exit(1);
			}
			printf("If you want to send a message to one of the users, pls send with the format: '#username:message'\n");
		}
		else if (strncmp(buffer, "#", 1) == 0) {
			// If the user want to send a direct message to another user, e.g., aa wants to send direct message "Hello" to bb, aa needs to input "#bb:Hello"
			printf("send direct message\n");
			if (send(sockfd, buffer, sizeof(buffer), 0)<0){
				printf("Sending direct message failed...\n");
				exit(1);
			}
		}
		else {
			/*************************************/
			/* Sending broadcast message. The send message should be of the format "username: message"*/
			/**************************************/
			char msg[MAX+10];
			bzero(msg, sizeof(msg));  
			strcpy(msg,name);//TODO: check edge case
			strcat(msg,": ");
			strcat(msg,buffer);
			if (send(sockfd, msg, sizeof(msg), 0)<0){
				printf("Sending broadcast message failed...\n");
				exit(1);
			}
		}
	}
	return 0;
}

