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
#define SERVER_ADDR_LEN 128
#define CLIENT_ADDR_LEN 128
#define NRM  "\x1B[0m"
#define RED  "\x1B[1;31m"
#define GRN  "\x1B[1;32m"
#define MOVEUP "\e[A"
#define CLRLINE "\r\e[K"

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
	if(argc<2)
	{
		printf("usage: client <server-ip-address>\n");
		exit(EXIT_FAILURE);
	}
	
	struct sockaddr_in client_addr,server_addr;	//same as server address
	int server_sockfd, client_sockfd;
	
	char* server_addr_str = argv[1];
	
	pid_t child_pid;
	
	//int socket(int domain, int type, int protocol);
	if((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		printf("Error creating server socket!\n");
		exit(EXIT_FAILURE);
	}
	printf("Socket with file descriptor no. %d created successfully...\n",client_sockfd);
	
	//int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
	int true = 1;
	if(setsockopt(client_sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) < 0)
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
		
	client_addr.sin_family = AF_INET;  
	client_addr.sin_addr.s_addr = inet_addr(server_addr_str);
	client_addr.sin_port = PORT;
	
	char client_addr_str[CLIENT_ADDR_LEN+1];
	inet_ntop(AF_INET, &(client_addr.sin_addr), client_addr_str, CLIENT_ADDR_LEN);  
	
	printf("Client ran on %s:%d\n",client_addr_str,client_addr.sin_port);
	
	//int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
	if((connect(client_sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr)))<0)
	{
		perror("connect");
		printf("Error in connecting to server!\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Connected file with descriptor no. %d to server successfully...\n",client_sockfd);
	printf("Waiting for message...\n");
	
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
			if ((write(client_sockfd, buf, BUF_SIZE))<0) 
			{
				perror("write");
				printf("Error sending data! Stopping.\n");
				kill(child_pid,SIGSTOP);				
				exit(EXIT_FAILURE);
			}
			if(strcmp(buf,"exit\n") == 0 || strcmp(buf,"Exit\n")==0)
			{	
				printf("Received exit command. Stopping...\n");
				kill(child_pid,SIGSTOP);				
				break;
			}
		}
		printf("Child process(receiver) over, conversation over... Stopping server...\n");
	}
	close(client_sockfd);
	return 0;
}

