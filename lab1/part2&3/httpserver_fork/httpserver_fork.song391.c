#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXLINE 500

int open_listenfd(int port)  
{ 
  int listenfd, optval=1; 
  struct sockaddr_in serveraddr; 
   
  /* Create a socket descriptor */ 
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; 
  
  /* Eliminates "Address already in use" error from bind. */ 
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,  
		 (const void *)&optval , sizeof(int)) < 0) 
    return -1; 
 
  /* Listenfd will be an endpoint for all requests to port 
     on any IP address for this host */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET;  
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
  serveraddr.sin_port = htons((unsigned short)port);  
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
 
  /* Make it a listening socket ready to accept 
     connection requests */ 
  if (listen(listenfd, LISTENQ) < 0) 
    return -1; 
 
  return listenfd; 
} 

void echo(int connfd)  
{ 
  size_t n;
  int shift_num;
  int cyphernum;
  int i = 0;
  char *shift_char;
  char *path;
  char buf[MAXLINE];
  char ok[] = "HTTP/1.0 200 OK\r\n\r\n";
  char not_found[] = "HTTP/1.0 404 Not Found\r\n\r\n";
  char forbidden[] = "HTTP/1.0 403 Forbidden\r\n\r\n";
  FILE *target;
    
 
  // read GET commend, assume it is in same style  
  read(connfd, buf, MAXLINE);
  path = strtok(buf, " ");
  path = strtok(NULL, " ");
  shift_char = strtok(NULL, " ");
  shift_num = atoi(shift_char);
  shift_num = shift_num % 26;

  if(access(path, F_OK) == 0){
    if(access(path, R_OK) == 0){
      write(connfd, ok, strlen(ok));
      target = fopen(path, "r");
      while(!feof(target)){
        bzero(buf,MAXLINE);
	fread(buf, sizeof(char), MAXLINE-1, target);
	while(buf[i] != '\0' &&  i < strlen(buf)){
	  if(isalpha(buf[i])){
	    if(((int)buf[i]) > 96)
	      cyphernum = (((int)buf[i] - 71 - shift_num) % 26 + 97);
	    else
	      cyphernum = (((int)buf[i] - 39 - shift_num) % 26 + 65);
	    buf[i] = (char)(cyphernum);
	  }
	  i++;
	}
	i = 0;
	write(connfd, buf, strlen(buf));
      }
      fclose(target); 
    }
    else{
      write(connfd, forbidden, strlen(forbidden));
    }
  }
  else{
    write(connfd, not_found, strlen(not_found));
  }
} 


int main(int argc, char **argv) {
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;

  port = atoi(argv[1]); /* the server listens on a port passed 
			   on the command line */
  listenfd = open_listenfd(port); 

  while (1) {
    clientlen = sizeof(clientaddr); 
    connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);

    if(fork() == 0){
      close(listenfd);
      echo(connfd);
      close(connfd);
      exit(0);
    }
    close(connfd);
  }
}

