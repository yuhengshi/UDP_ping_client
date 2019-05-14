//Yuheng Shi
//4519641
//I used 3 websites as my source of writing this particular lab:
/*
http://www.linuxhowtos.org/data/6/client_udp.c
https://www.geeksforgeeks.org/udp-server-client-implementation-c/
https://github.com/rehassachdeva/UDP-Pinger
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>


int getSeqNum(const char *msg){
  int foundSeq = 0;
  int seq = 0;
  for(int i = 0; i < 256; i++){
    if(msg[i] == ' '){
      if(foundSeq) return seq;
      foundSeq = 1;
    }else if(foundSeq){
      seq *= 10;
      seq += msg[i]-'0';
    }
  }
  return seq;
}

int main(int argc, char *argv[]){
  int sock;
  unsigned int length;
  struct sockaddr_in server, from;
  struct hostent *hp;
  char buffer[256];
  if (argc != 3){
    perror("Invalid Arguments");
    exit(0);
  }
  sock= socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0){
    perror("socket failed");
    exit(1);
  }


  //setting up the socket
  server.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  if (hp==0){
    perror("Unknown host");
    exit(1);
  }
  bcopy((char *)hp->h_addr, (char *)&server.sin_addr, hp->h_length);
  server.sin_port = htons(atoi(argv[2]));
  length=sizeof(struct sockaddr_in);

  //this is for execution time
  struct timeval timeout;
  timeout.tv_sec = 1; //seconds
  timeout.tv_usec = 0;  //microseco
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
    perror("set socket failed");
    exit(1);
  }
  //initializing 10 times of sending
  int sendings = 10, received = 0, PCT = 0;
  double MIN = 1000, AVG = 0, MAX = 0, TTL = 0; 
  int seqNum = 0;
  struct timeval begin_time, end_time;
  int seq = 0;

  for(int i=0; i<10; i++){
    gettimeofday(&begin_time, 0);
    sprintf(buffer, "PING %i %i", seqNum,(int)(begin_time.tv_sec));
    if(sendto(sock,buffer, strlen(buffer),0,(const struct sockaddr *)&server,length) < 0){
      perror("Send to server failed");
      exit(1);
    }
    bzero(buffer,256);
    if(recvfrom(sock, buffer,256, 0, (struct sockaddr *) &from, &length) < 0){
      printf("PING timed out\n");
    }else{
      gettimeofday(&end_time, 0);
      int foundSeq = 0;
      for(int i = 0; i < sizeof(buffer); i++){
        if(buffer[i] == ' '){
          if(foundSeq)
            break;
          foundSeq = 1;
        }else if(foundSeq){
          seq *= 10;
          seq += buffer[i]-'0';
        }
      }
      double time = (end_time.tv_sec - begin_time.tv_sec)*1000+(end_time.tv_usec-begin_time.tv_usec)/1000.0;
      if(time < MIN)
        MIN = time;
      if(time > MAX)
        MAX = time;
      TTL += time;
      received++;
      printf("PING received from %s: seq#=%i time=%f ms\n",argv[1], seq, time);
    }
    seqNum++;
    seq = 0;
  }
  PCT = (int)((sendings - received)*1.0/ sendings * 100);
  AVG = TTL/sendings;
  printf("--- ping statistics --- %i packets transmitted, %i received, %i%% packet loss rtt min/avg/max = %f %f %f ms\n", sendings, received, PCT, MIN, AVG, MAX);
  close(sock);
  return 0;
}

