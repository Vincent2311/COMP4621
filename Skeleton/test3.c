#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "chatroom.h"
#include <poll.h>


int main(){
	// char c;
	// int n = 0;
    // char name[5];
    // char buffer[20];
	// while ((c = getchar()) != '\n' && n < C_NAME_LEN)
	// 		name[n++] = c;
    // name[n] = '\0';
    // printf("%s %d\n",name,strlen(name));
    // bzero(buffer, sizeof(buffer));
	// strcpy(buffer,"REGISTER");
	// strcat(buffer, name);
    // printf("%s %d\n",buffer,strlen(buffer));

    // char recname[20];
    // bzero(recname,sizeof(recname));
    // // int idx = 8;
    // // for (;buffer[idx] !='\0';idx++){
    // //     recname[idx-8] = buffer[idx];
    // // }
    // char  * p = buffer;
    // p += 8;
    // strcpy(recname,p);
    // printf("The name of the user of incoming connection is: %s\n",recname); 
    char buffer[100];
    char name[100];
	scanf("%s",name);
	while (strlen(name) > C_NAME_LEN)
	{
		printf("Input name exceeds max name lenght");
		scanf("%s",name);
	}
	bzero(buffer, sizeof(buffer));
	strcpy(buffer,"REGISTER");
	strcat(buffer, name);

    char recname[C_NAME_LEN+1];
    bzero(recname,sizeof(recname));
    char  *p = buffer;
    p += 8;
    strcpy(recname,p);
    
    printf("The name of the user of incoming connection is: %s\n",recname); 
    printf("success\n");
}