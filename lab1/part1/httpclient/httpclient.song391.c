#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 200

int open_clientfd(char *hostname, int port) 
{ 
  int clientfd; 
  struct hostent *hp; 
  struct sockaddr_in serveraddr; 
 
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; /* check errno for cause of error */ 
 
  /* Fill in the server's IP address and port */ 
  if ((hp = gethostbyname(hostname)) == NULL) 
    return -2; /* check h_errno for cause of error */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET; 
  bcopy((char *)hp->h_addr,  
        (char *)&serveraddr.sin_addr.s_addr, hp->h_length); 
  serveraddr.sin_port = htons(port); 
 
  /* Establish a connection with the server */ 
  if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
  return clientfd; 
} 


/* usage: ./echoclient host port */
int main(int argc, char **argv)
{ 
  int clientfd, port, respcod; 
  char *host, buf[MAXLINE], prebuf[MAXLINE];
  char http[] = "HTTP/1.1";
  char code[] = "200";
  FILE *fp;
    
  host = argv[1]; 
  port = atoi(argv[2]);
  
 
  clientfd = open_clientfd(host, port);

  if (clientfd < 0){
    printf("Error opening connection \n");
    exit(0);
  }

  sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", argv[3]);

     fp = fopen("Output.txt", "w");
  write(clientfd, buf, strlen(buf)); 
  read(clientfd, buf, MAXLINE-1);
  while(1)
    {
      fputs(buf, stdout); 
      fprintf(fp, "%s", buf);
      bzero(buf,MAXLINE);
      read(clientfd, buf, MAXLINE-1);
      if(strlen(buf) < 1)
 	break;
    }
  fclose(fp);
  bzero(buf,MAXLINE);
  bzero(prebuf,MAXLINE);
  fp = fopen("Output.txt","r");
  while(1)
    {
      fscanf(fp, "%s", buf);
      if(strcmp(prebuf, http) == 0)
	if(strcmp(buf, code) != 0)
	  exit(0);
      if(feof(fp))
	break;
      strcpy(prebuf, buf);
      bzero(buf,MAXLINE);
     }
  fclose(fp);

  clientfd = open_clientfd(host, port);

  if (clientfd < 0){
    printf("Error opening connection \n");
    exit(0);
  }
  sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", prebuf);
  fp = fopen("Output2.txt","w");
  write(clientfd, buf, strlen(buf)); 
  read(clientfd, buf, MAXLINE-1);
  while(1)
    {
      fputs(buf, stdout); 
      fprintf(fp, "%s", buf);
      bzero(buf,MAXLINE);
      read(clientfd, buf, MAXLINE-1);
      if(strlen(buf) < 1)
 	break;
    }
  printf("\n");
  fclose(fp);
  exit(0); 
} 



