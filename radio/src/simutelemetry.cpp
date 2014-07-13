#include "opentx.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

extern Fifo<512> telemetryFifo;
int sockfd;
pthread_t t1;

void *  readFrskyData(void *){
  char data;
  while (1){
    int n = read(sockfd,&data,1);
    if (n==1)  telemetryFifo.push(data);
    //    fprintf (stderr,"%#04x ",data);
  }
}

void simuTelemetryInit(){
    int  portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = 5763;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
      fprintf (stderr,"ERROR opening socket");
    server = gethostbyname("localhost");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
      fprintf (stderr,"Not connected %d\n",portno);
    fprintf (stdout, "Connected to port %d\n",portno);
    pthread_create(&t1, NULL, &readFrskyData, NULL);
}



