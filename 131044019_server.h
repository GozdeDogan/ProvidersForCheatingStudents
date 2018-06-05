/**
 * Gozde DOGAN
 * 131044019
 * Sistem Programlama
 * FINAL PROJESI
 *
 * 131044019_server.h
 */


#ifndef H_131044019_SERVER
#define H_131044019_SERVER



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
#include <sys/un.h>
#include <sys/shm.h>
#include <errno.h>

#include "threadpool.h"

//#define DEBUG
#define MAX_STRING_SIZE 50
#define COLUMN_SIZE 100 	// dosyadaki satir sayisini bulmak icin fgets'e verilen parametre

#define MAX_QUEUE_SIZE 2

#define HIGH_PERFORMANCE 5
#define LOW_PERFORMANCE 1

#define LISTENQ 5

#define PI 3.141592653589
#define NUMBER_OF_TERMS 10

#define FIFO_THREAD "fifo_thread"

/********************************************* STRUCT VARIABLES *********************************************/
typedef struct 
{
	char ProviderName[MAX_STRING_SIZE];
	int performance;
	int price;
	int duration;	
	int numOfClients;
	char queue[MAX_QUEUE_SIZE][MAX_STRING_SIZE];
	int indexQueue;
}Providers;

typedef struct{
	Providers provider;
} PriorityOfProviders;

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
} SendInformation;

/*************************************************************************************************************/

/********************************************* GLOBAL VARIABLES *********************************************/
char sProviderFileName[MAX_STRING_SIZE];
char sLogFileName[MAX_STRING_SIZE];
int iPortAdress;

int listenfd;
int connfd;

socklen_t clilen;
struct sockaddr_in cliaddr, servaddr;


PriorityOfProviders *pPriorityQ; //for performance
PriorityOfProviders *pPriorityT; //for time
PriorityOfProviders *pPriorityC; //for price

Providers *pProviders;
int iSizeOfProviders = 0;

Client client;
//SendInformation sendInfo;

FILE *fpLogFile;

int opt = 1;

struct timeval sTime; 							// programin baslangic zamani
struct timeval lastTime; 						// programin bitis zamani

struct sigaction sa;								/* SIGINT sinyali icin */
static int doneFlag = 0;

int fifo_thread;
char fifos[LISTENQ][MAX_STRING_SIZE];
int indexFifos = -1;

pid_t childPIDs[LISTENQ];
int indexChildPIDs = 0;

pthread_t thread;

struct threadpool *pool;
pthread_mutex_t mutex;	
/**************************************************************************************************************/

/********************************************* FUNCTION PROTOTYPE *********************************************/
void usage();
void findNumOfpProviders();
void server();
void *ThreadServer(void *arguman);
void readFile();
void determinePriorityQueue();
void providerDoHomework(Client datainfo, SendInformation *sendInfo);
int chooseProvider(Client datainfo);
int findProvider(char providerName[MAX_STRING_SIZE]);
void initialize();
void freeArr();
void printProviders();
void printPriority(PriorityOfProviders *p, int size);
double solveHomework(int CalculateValue);
double calculateTaylorSeries(double radians);
double factorial(double x);
double power(double x, double y);
long calculateTimeDifference(struct timeval start, struct timeval finish);
void signalHandler(int sig);
/**************************************************************************************************************/

#endif