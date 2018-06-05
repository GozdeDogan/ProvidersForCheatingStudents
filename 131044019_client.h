/**
 * Gozde DOGAN
 * 131044019
 * Sistem Programlama
 * FINAL PROJESI
 *
 * 131044019_client.h
 */

#ifndef H_131044019_CLIENT
#define H_131044019_CLIENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


//#define DEBUG
#define MAX_STRING_SIZE 50

#define LISTENQ 20



/********************************************* STRUCT VARIABLES *********************************************/
typedef struct {
	char ClientName[MAX_STRING_SIZE];
	char Priority;
	int Degree;
	pid_t pid;
} Client;


typedef struct 
{
	char providerName[MAX_STRING_SIZE];
	double homeworkResult;
	double spendTime;
	int cost;
} ReceiveInformation;

/**************************************************************************************************************/


/********************************************* GLOBAL VARIABLES *********************************************/
Client client;
ReceiveInformation recvInfo;

char sServerAddress[MAX_STRING_SIZE];
int iPortAdress;

int sockfd;
    
int listenfd;
int connfd;

socklen_t clilen;
struct sockaddr_in cliaddr, servaddr;

struct timeval sTime; 							// programin baslangic zamani
struct timeval lastTime; 						// programin bitis zamani

/**************************************************************************************************************/


/********************************************* FUNCTION PROTOTYPE *********************************************/
void usage();
long calculateTimeDifference(struct timeval start, struct timeval finish);

/**************************************************************************************************************/


#endif