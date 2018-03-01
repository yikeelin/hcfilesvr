#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define SERVER_PORT 5555
#define	MAXLINE 1024 

int main(int argc, char *argv[])
{
	int sockfd;
	int i;
	struct sockaddr_in serveraddr, clientaddr;
	char buf[MAXLINE];
	char ipstr[INET_ADDRSTRLEN];
	socklen_t clientlen;
	ssize_t len;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; /*IPv4*/
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVER_PORT);	
	bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	while(1){

		clientlen = sizeof(clientaddr);
		len = recvfrom(sockfd,buf, sizeof(buf), 0, 
			(struct sockaddr *)&clientaddr,	&clientlen);

		printf("client IP %s\t PORT %d\n", inet_ntop(AF_INET, 
			&clientaddr.sin_addr.s_addr, ipstr, sizeof(ipstr)), 
			ntohs(clientaddr.sin_port));

		i=0;
		while(i < len){
			buf[i] = toupper(buf[i]);
			i++;
		}	
		buf[i]=0;

		sendto(sockfd, buf, len, 0, (struct sockaddr*)&clientaddr,
			sizeof(clientaddr));


	}

	close(sockfd);
	return 0;	
}
