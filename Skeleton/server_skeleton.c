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

#define MAX 1024					 // max buffer size
#define PORT 6789					 // server port number
#define MAX_USERS 50				 // max number of users
static unsigned int users_count = 0; // number of registered users

static user_info_t *listOfUsers[MAX_USERS] = {0}; // list of users

/* Add user to userList */
void user_add(user_info_t *user);
/* Get user name from userList */
char *get_username(int sockfd);
/* Get user sockfd by name */
int get_sockfd(char *name);

/* Add user to userList */
void user_add(user_info_t *user)
{
	if (users_count == MAX_USERS)
	{
		printf("sorry the system is full, please try again later\n");
		return;
	}
	/***************************/
	/* add the user to the list */
	/**************************/
	listOfUsers[users_count] = user;
	users_count++;
}

/* Determine whether the user has been registered  */
int isNewUser(char *name)
{
	int i;
	int flag = -1;
	/*******************************************/
	/* Compare the name with existing usernames */
	/*******************************************/
	for (unsigned int i = 0; i < users_count; i++)
	{
		if (!strcmp(name, listOfUsers[i]->username))
		{
			flag = 0;
			break;
		}
	}

	return flag;
}

/* Get user name from userList */
char *get_username(int ss)
{
	int i;
	static char uname[MAX];
	/*******************************************/
	/* Get the user name by the user's sock fd */
	/*******************************************/
	for (unsigned int i = 0; i < users_count; i++)
	{
		if (!strcmp(ss, listOfUsers[i]->sockfd))
		{
			strcpy(uname, listOfUsers[i]->username);
			break;
		}
	}

	printf("get user name: %s\n", uname);
	return uname;
}

/* Get user sockfd by name */
int get_sockfd(char *name)
{
	int i;
	int sock = -1;
	/*******************************************/
	/* Get the user sockfd by the user name */
	/*******************************************/
	for (unsigned int i = 0; i < users_count; i++)
	{
		if (!strcmp(name, listOfUsers[i]->username))
		{
			sock = listOfUsers[i]->sockfd;
			break;
		}
	}

	return sock;
}
// The following two functions are defined for poll()
// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
	// If we don't have room, add more space in the pfds array
	if (*fd_count == *fd_size)
	{
		*fd_size *= 2; // Double it

		*pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

	(*fd_count)++;
}
// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
	// Copy the one from the end over this one
	pfds[i] = pfds[*fd_count - 1];

	(*fd_count)--;
}

int main()
{
	int listener;  // listening socket descriptor
	int newfd;	   // newly accept()ed socket descriptor
	int addr_size; // length of client addr
	int client_socket;
	struct sockaddr_in server_addr, client_addr;

	char buffer[MAX]; // buffer for client data
	int nbytes;
	int fd_count = 0;
	int fd_size = 5;
	struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

	int yes = 1; // for setsockopt() SO_REUSEADDR, below //TODO:
	int i, j, u, rv;

	/**********************************************************/
	/*create the listener socket and bind it with server_addr*/
	/**********************************************************/
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1)
	{
		printf("Listening Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");

	bzero(&server_addr, sizeof(server_addr));
	// asign IP, PORT
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	// Binding newly created socket to given IP and verification
	if ((bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0)
	{
		printf("Listening Socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");
	// Now server is ready to listen and verification
	if ((listen(listener, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(3);
	}
	else
		printf("Server listening..\n");

	// Add the listener to set
	pfds[0].fd = listener;
	pfds[0].events = POLLIN; // Report ready to read on incoming connection
	fd_count = 1;			 // For the listener

	// main loop
	for (;;)
	{
		/***************************************/
		/* use poll function */
		/**************************************/
		int poll_count = poll(pfds, fd_count, -1);
		if (poll_count == -1)
		{
			perror("poll");
			exit(1);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i < fd_count; i++)
		{
			if (pfds[i].revents & POLLIN)
			{ // we got one!!
				if (pfds[i].fd == listener)
				{
					/**************************/
					/* we are the listener and we need to handle new connections from clients */
					/****************************/
					addr_size = sizeof(client_addr);
					client_socket = accept(listener, (struct sockaddr *)&client_addr, &addr_size);

					if (client_socket == -1)
					{
						perror("accept");
					}
					else
					{
						add_to_pfds(&pfds, client_socket, &fd_count, &fd_size);
						printf("pollserver: new connection from %s on socket %d\n",inet_ntoa(client_addr.sin_addr),client_socket);
						// send welcome message
						bzero(buffer, sizeof(buffer));
						strcpy(buffer, "Welcome to the chat room!\nPlease enter a nickname.\n");
						if (send(newfd, buffer, sizeof(buffer), 0) == -1)
							perror("send");
					}
				}
				else
				{
					// handle data from a client
					bzero(buffer, sizeof(buffer));
					if ((nbytes = recv(pfds[i].fd, buffer, sizeof(buffer), 0)) <= 0)
					{
						// got error or connection closed by client
						if (nbytes == 0)
						{
							// connection closed
							printf("pollserver: socket %d hung up\n", pfds[i].fd); // TODO:Indeed?
						}
						else
						{
							perror("recv");
						}
						close(pfds[i].fd); // Bye!
						del_from_pfds(pfds, i, &fd_count);
					}
					else
					{
						// we got some data from a client
						if (strncmp(buffer, "REGISTER", 8) == 0)
						{
							printf("Got register/login message\n");
							/********************************/
							/* Get the user name and add the user to the userlist*/
							/**********************************/
							char *name;
    						name = (char *)buffer + 8;
							printf("The name of the user of incoming connection is: %s\n",name); 
							if (isNewUser(name) == -1)
							{
								/********************************/
								/* it is a new user and we need to handle the registration*/
								/**********************************/
								user_info_t *new_user;
								new_user->sockfd = pfds[i].fd;
								new_user->state = 1;
								snprintf(new_user->username, sizeof(new_user->username), "%s", name);
								user_add(new_user);
								printf("add user name: %s\n",name);
								/********************************/
								/* create message box (e.g., a text file) for the new user */
								/**********************************/
								FILE *fp;
								char file_name[C_NAME_LEN + 10];
								strcpy(file_name,name);
								strcat(file_name, ".txt");
								fp  = fopen (file_name, "w");  //TODO: correct?
								fclose (fp);
								
								// broadcast the welcome message (send to everyone except the listener)
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "Welcome ");
								strcat(buffer, name);
								strcat(buffer, " to join the chat room!\n");
								/*****************************/
								/* Broadcast the welcome message*/
								/*****************************/
								for (unsigned int j = 1; j < users_count;j++){
									if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
								}
								/*****************************/
								/* send registration success message to the new user*/
								/*****************************/
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "A new account has been created.");
								if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
							}
							else
							{
								/********************************/
								/* it's an existing user and we need to handle the login. Note the state of user,*/
								/**********************************/
								user_info_t *user;
								for (unsigned int j = 0; j < users_count; j++)
								{
									if (!strcmp(name, listOfUsers[j]->username))
									{
										user = listOfUsers[j];
										break;
									}
								}
								user->state = 1;
								/********************************/
								/* send the offline messages to the user and empty the message box*/
								/**********************************/
								FILE* fp;
								char file_name[C_NAME_LEN + 10];
								strcpy(file_name,name);
								strcat(file_name, ".txt");
								fp  = fopen (file_name, "r"); 
								
								char * line = NULL;
    							size_t len = 0;
								while (getline(&line, &len, fp) != -1) {
        							if (send(user->sockfd, line, sizeof(line), 0) == -1)
										perror("send");
								}
								fclose (fp);
								remove(file_name); 
								
								// broadcast the welcome message (send to everyone except the listener)
								bzero(buffer, sizeof(buffer));
								strcat(buffer, name);
								strcat(buffer, " is online!\n");
								/*****************************/
								/* Broadcast the welcome message*/
								/*****************************/
								for (unsigned int j = 1; j < users_count;j++){
									if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
								}
							}
						}
						else if (strncmp(buffer, "EXIT", 4) == 0)
						{
							printf("Got exit message. Removing user from system\n");
							// send leave message to the other members
							bzero(buffer, sizeof(buffer));
							strcpy(buffer, get_username(pfds[i].fd));
							strcat(buffer, " has left the chatroom\n");
							/*********************************/
							/* Broadcast the leave message to the other users in the group*/
							/**********************************/
							for (unsigned int j = 1; j < users_count;j++){
									if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
							}
							/*********************************/
							/* Change the state of this user to offline*/
							/**********************************/
							char* name = get_username(pfds[i].fd);
							user_info_t *user;
							for (unsigned int j = 0; j < users_count; j++)
							{
								if (!strcmp(name, listOfUsers[j]->username))
								{
									user = listOfUsers[j];
									break;
								}
							}
							user->state = 0;
							// close the socket and remove the socket from pfds[]
							close(pfds[i].fd);
							del_from_pfds(pfds, i, &fd_count);
						}
						else if (strncmp(buffer, "WHO", 3) == 0)
						{
							// concatenate all the user names except the sender into a char array
							printf("Got WHO message from client.\n");
							char ToClient[MAX];
							bzero(ToClient, sizeof(ToClient));
							/***************************************/
							/* Concatenate all the user names into the tab-separated char ToClient and send it to the requesting client*/
							/* The state of each user (online or offline)should be labelled.*/
							/***************************************/
							char* user_name_state[C_NAME_LEN + 10];
							for (unsigned int j = 0; j < users_count; j++)
							{
								if(j==i) continue;
								strcat(user_name_state,listOfUsers[j]->username);
								if(listOfUsers[j]->state)
									strcat(user_name_state,"*");
								strcat(ToClient, user_name_state);
								strcat(ToClient,"\t");
							}
							if (send(pfds[i].fd, ToClient, sizeof(ToClient), 0) == -1)
										perror("send");
							
							bzero(buffer, sizeof(buffer));
							strcpy(buffer, "* means this user online");
							if (send(pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
						}
						else if (strncmp(buffer, "#", 1) == 0)
						{
							// send direct message
							// get send user name:
							printf("Got direct message.\n");
							// get which client sends the message
							char sendname[MAX];
							// get the destination username
							char destname[MAX];
							// get dest sock
							int destsock;
							// get the message
							char msg[MAX];
							/**************************************/
							/* Get the source name xx, the target username and its sockfd*/
							/*************************************/
							strcpy(sendname, get_username(pfds[i].fd));
							size_t idx = 1;
							for (; idx < sizeof(buffer); idx++)
							{
								if(buffer[idx]==':') break;
								destname[idx] = buffer[idx];	
							}
							destsock = get_sockfd(destname);
							for (size_t j = idx+1; j < sizeof(buffer); j++)
							{
								msg[j-idx-1] = buffer[j];	
							}

							if (destsock == -1)
							{
								/**************************************/
								/* The target user is not found. Send "no such user..." messsge back to the source client*/
								/*************************************/
								bzero(buffer, sizeof(buffer));
								strcpy(buffer, "There is no such user. Please check your input format.\n");
								if (send(pfds[i].fd, buffer, sizeof(buffer), 0) == -1)
										perror("send");
							}
							else
							{
								// The target user exists.
								// concatenate the message in the form "xx to you: msg"
								char sendmsg[MAX];
								strcpy(sendmsg, sendname);
								strcat(sendmsg, " to you: ");
								strcat(sendmsg, msg);

								/**************************************/
								/* According to the state of target user, send the msg to online user or write the msg into offline user's message box*/
								/* For the offline case, send "...Leaving message successfully" message to the source client*/
								/*************************************/
								user_info_t *tar_user;
								for (unsigned int j = 0; j < users_count; j++)
								{
									if (!strcmp(destname, listOfUsers[j]->username))
									{
										tar_user = listOfUsers[j];
										break;
									}
								}

								if(tar_user->state){  //online
									if (send(destsock, sendmsg, sizeof(sendmsg), 0) == -1)
										perror("send");
								}
								else{
									FILE* fp;
									char file_name[C_NAME_LEN + 10];
									strcpy(file_name,destname);
									strcat(file_name, ".txt");
									fp  = fopen (file_name, "w"); 
									fputs(sendmsg, fp);
									fclose (fp);

									char offine_message[MAX];
									strcpy(offine_message, destname);
									strcat(offine_message," is offline. Leaving message successfully.");
									if (send(pfds[i].fd, offine_message, sizeof(offine_message), 0) == -1)
										perror("send");
								}
							}
						}
						else
						{
							printf("Got broadcast message from user\n");
							/*********************************************/
							/* Broadcast the message to all users except the one who sent the message*/
							/*********************************************/
							for (unsigned int j = 1; j < users_count;j++){
								if(i==j) continue;
								if (send(pfds[j].fd, buffer, sizeof(buffer), 0) == -1)
									perror("send");
							}
						}
					} 
				}	  // end handle data from client
			}		// end got new incoming connection  
		}// end looping through file descriptors	  
	}// end for(;;)
	return 0;
}
