#include<stdio.h>
#include<unistd.h>	//getpid()
#include<sys/socket.h> 	//socket()
#include<sys/types.h> 	//SOCK_STREAM
#include<signal.h>	//SIGSTOP
#include<string.h> 	//memset()
#include<stdlib.h>	//exit()
#include"netinet/in.h"	//INADDR_ANY

#define BUF_SIZE 1024
#define PORT 4444
#define CLIENT_ADDR_LEN 128
#define SERVER_ADDR_LEN 128
#define NRM  "\x1B[0m"
#define RED  "\x1B[1;31m"
#define GRN  "\x1B[1;32m"
#define MOVEUP "\033[A"
#define CLRLINE "\r\033[K"

int receiveMsg(int sockfd, int pid) 
{
	int ret;
	char buf[BUF_SIZE+1];
	while((ret = read(sockfd,buf,BUF_SIZE))>0)
	{
		if(strcmp(buf,"exit\n") == 0 || strcmp(buf,"Exit\n")==0)
		{	
			printf("Received exit command. Stopping...\n");
			kill(pid,SIGSTOP);					
			break;
		}
		printf("%s%sYour mate : %s%s", CLRLINE, RED, NRM, buf);
	}
	if (ret < 0) 
	{    
		perror("read");
		return -1;
	}
	return 0;
}

int main(int argc, char* argv[])
{
	struct sockaddr_in server_addr, client_addr;
	int server_sockfd, client_sockfd;
	
	pid_t child_pid;
	
	//int socket(int domain, int type, int protocol);
	if((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		printf("Error creating server socket!\n");
		exit(EXIT_FAILURE);
	}
	printf("Socket with file descriptor no. %d created successfully...\n",server_sockfd);	

	int true = 1;
	//int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
	if(setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
		
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;  
	server_addr.sin_addr.s_addr = INADDR_ANY;  
	server_addr.sin_port = PORT;
	
	char server_addr_str[SERVER_ADDR_LEN+1];
	inet_ntop(AF_INET, &(server_addr.sin_addr), server_addr_str, SERVER_ADDR_LEN);
	printf("Server ran on %s:%d\n",server_addr_str,server_addr.sin_port);
	 
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if((bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)))<0)
	{
		perror("bind");
		printf("Error in binding socket with file %d!\n", server_sockfd);
		exit(EXIT_FAILURE);
	}
	
	printf("Binded file descriptor no. %d with socket successfully...\n",server_sockfd);
	printf("Waiting for message...\n");
	//int listen(int sockfd, int backlog); //backlog=sizeof(queue)
	if(listen(server_sockfd, 5)<0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	
	int cl_addr_len = sizeof(client_addr);
	memset(&client_addr, 0, sizeof(client_addr));
		
	if((client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_addr, &cl_addr_len)) < 0)
	{
		perror("accept");
		printf("Cannot accept connection : Error creating client socket!\n");
		exit(EXIT_FAILURE);
	}
	
	true = 1;
	if(setsockopt(client_sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	char client_addr_str[CLIENT_ADDR_LEN+1];
	inet_ntop(AF_INET, &(client_addr.sin_addr), client_addr_str, CLIENT_ADDR_LEN);
	printf("Connection accepted from IP adrress : %s in file with descriptor no. %d.\n", client_addr_str,client_sockfd);

	char buf[BUF_SIZE+1];
	memset(buf, 0, sizeof(buf));  
	printf("Enter your messages one by one and press 'Enter' key:\n"); 
	
	//Creating new process for receiving messages
	if((child_pid = fork()) < 0)
	{
		perror("fork");
		printf("ERROR : Cannot create child process.\n");
		exit(EXIT_FAILURE);
	}
	else if(child_pid == 0)
	{
		if((receiveMsg(client_sockfd,getppid())) < 0)
		{
			perror("receiveMsg");
			printf("Error receiving data! Stopping...\n");
			exit(EXIT_FAILURE);	
		}
	}
	else if(child_pid > 0)
	{
		printf("Created new process with PID %d for receiving messages...\n",(int)child_pid);				

		while (fgets(buf, BUF_SIZE, stdin) != NULL)
		{
			printf("%s%s%sYou : %s%s", MOVEUP, CLRLINE, GRN, NRM, buf);	//'\033[1A' = Move one line up, '\033[K' = Remove all till end of line
			if ((write(client_sockfd, buf, BUF_SIZE)) < 0)
			{
				perror("write");
				printf("Error sending data! Stopping child process...\n");
				if(kill(child_pid,SIGSTOP)<0)
				{
					perror("kill");
					printf("Error! Cannot kill child process with PID %d.\n",child_pid);
				}
				exit(EXIT_FAILURE);
			}
			if(strcmp(buf,"exit\n") == 0 || strcmp(buf,"Exit\n")==0)
			{	
				printf("Received exit command. Stopping server...\n");
				if(kill(child_pid,SIGSTOP)<0)
				{
					perror("kill");
					printf("Error! Cannot kill child process with PID %d.\n",child_pid);
				}
				break;
			}
		}
		printf("Child process(receiver) over, conversation over... Stopping server...\n");
	}
	close(client_sockfd);
	close(server_sockfd);
	return 0;
}

