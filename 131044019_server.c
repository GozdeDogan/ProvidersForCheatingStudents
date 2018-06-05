/**
 * Gozde DOGAN
 * 131044019
 * Sistem Programlama
 * FINAL PROJESI
 *
 * 131044019_server.c
 */

#include "131044019_server.h"

int main(int argc, char const *argv[])
{
	if(argc != 4){
		usage();
		exit(EXIT_FAILURE);
	}

	iPortAdress = atoi(argv[1]);
	sprintf(sProviderFileName, "%s", argv[2]);
	sprintf(sLogFileName, "%s", argv[3]);

	#ifdef DEBUG
		fprintf(stderr, "iPortAdress: %d \t sProviderFileName: %s \t sLogFileName: %s\n", iPortAdress, sProviderFileName, sLogFileName);
	#endif

	fprintf(stderr, "Logs kept at %s\n", sLogFileName);


    /**************************** SINYAL HAZIRLIGI ****************************/
	sa.sa_handler = signalHandler;
    sa.sa_flags = SA_RESTART;
    /**************************************************************************/

    fpLogFile = fopen(sLogFileName, "w+");
    server();
	
	return 0;
}

void server(){
	int stateOfPool=0,task;

	findNumOfpProviders();

	pProviders = (Providers *)calloc(iSizeOfProviders, sizeof(Providers));

	readFile();

	#ifdef DEBUG 
		printProviders();
	#endif

	pPriorityQ = (PriorityOfProviders *)calloc(iSizeOfProviders, sizeof(PriorityOfProviders));
	pPriorityC = (PriorityOfProviders *)calloc(iSizeOfProviders, sizeof(PriorityOfProviders));
	pPriorityT = (PriorityOfProviders *)calloc(iSizeOfProviders, sizeof(PriorityOfProviders));
	
	determinePriorityQueue();
	initialize();

	#ifdef DEBUG
		fprintf(stderr, "PERFORMANCE - QUALITY >>>> \n");
		printPriority(pPriorityQ, iSizeOfProviders);

		fprintf(stderr, "COST - PRICE >>>> \n");
		printPriority(pPriorityC, iSizeOfProviders);

		fprintf(stderr, "TIME - DURATION >>>> \n");
		printPriority(pPriorityT, iSizeOfProviders);
	#endif


	fprintf(stderr, "\n%d provider threads created\n", iSizeOfProviders);
	fprintf(fpLogFile, "\n%d provider threads created\n", iSizeOfProviders);
	printProviders();

	for (int i = 0; i < iSizeOfProviders; ++i){
		fprintf(stderr, "Provider %s waiting for tasks\n", pProviders[i].ProviderName);
		fprintf(fpLogFile, "Provider %s waiting for tasks\n", pProviders[i].ProviderName);
	}

    fprintf(stderr, "Server is waiting for client connections at port %d\n", iPortAdress);
    fprintf(fpLogFile, "Server is waiting for client connections at port %d\n", iPortAdress);


    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0){
        fprintf(stderr, "ERROR opening socket\n");
        fprintf(fpLogFile, "ERROR opening socket\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &servaddr, sizeof(servaddr));

     servaddr.sin_family = AF_INET;
     servaddr.sin_addr.s_addr = INADDR_ANY;
     servaddr.sin_port = htons(iPortAdress);


	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "ERROR on binding\n");
        fprintf(fpLogFile, "ERROR on binding\n");
        exit(EXIT_FAILURE);
    }

	listen(listenfd, LISTENQ);


    if((mkfifo(FIFO_THREAD, 0666)) == -1){
    	fprintf(stderr, "Not create fifo\n");
    	exit(EXIT_FAILURE);
    }

	if(stateOfPool==0){
		int poolSay = iSizeOfProviders;
		
		if ((pool = threadpool_init(poolSay)) == NULL) {
			fprintf(stderr, "Error! Failed to create a thread pool struct.\n");
			fprintf(fpLogFile, "Error! Failed to create a thread pool struct.\n");
			exit(EXIT_FAILURE);
		}
		stateOfPool=1;
	}

	if(sigaction(SIGINT, &sa, NULL) == -1){
		perror("\tCan't SIGINT");
		fprintf(fpLogFile, "\tCan't SIGINT");
		//exit(EXIT_FAILURE);
	}

	while(1){
		clilen = sizeof(cliaddr);
    	connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
     	if (connfd < 0){ 
        	fprintf(stderr, "ERROR on accept");
        	fprintf(fpLogFile, "ERROR on accept");
        	exit(EXIT_FAILURE);
        }
        int value;
    	//fprintf(stderr, "AA\n");
    	do{
	    	value = read(connfd, &client, sizeof(client));
			if(value < 0)
				perror("reading stream message");
			else if (value == 0){
				//fprintf(stderr, "Ending connection\n");
				fprintf(fpLogFile, "Ending connection\n");
			}
			else{

				if (pthread_mutex_init(&mutex, NULL) != 0)  //mutex initialize edildi. 
			    {
			        fprintf(stderr, "mutex init has failed\n");
			        //exit(EXIT_FAILURE);
			    }

				childPIDs[indexChildPIDs] = client.pid;
				indexChildPIDs = indexChildPIDs + 1;

				fprintf(fpLogFile, "Client %s connected server. ClientPid: %d\n", client.ClientName, client.pid);

				fprintf(stderr, "Client %s is requesting %c %d from server %d\n\n", client.ClientName, client.Priority, client.Degree, iPortAdress);
				fprintf(fpLogFile, "Client %s is requesting %c %d from server %d\n\n", client.ClientName, client.Priority, client.Degree, iPortAdress);
				

				Client *threadArg = malloc(sizeof *threadArg);
				threadArg->pid = client.pid;
				sprintf(threadArg->ClientName, "%s", client.ClientName);
				threadArg->Priority = client.Priority;
				threadArg->Degree = client.Degree;

				task = threadpool_add_task(pool, ThreadServer, (void*)threadArg,0);
				if (task == -1) {
					fprintf(stderr,"Error! New task couldn't add.\n");
					fprintf(fpLogFile,"Error! New task couldn't add.\n");
					free(threadArg);
					exit(EXIT_FAILURE);
				}

				/*int res = pthread_create(&thread, 0, ThreadServer, (void *)&client);
				if(res != 0){
					fprintf(stderr, "pthread_create couldn't do\n");
					exit(EXIT_FAILURE);
				}	
				if (pthread_detach(thread)){
	            	printf("Thread detached failes!!!\n");
	            	exit(EXIT_FAILURE);
				}

				pthread_join(thread, NULL);*/

				while(((fifo_thread = open(FIFO_THREAD, O_RDONLY)) < 0) && (errno == EINTR));
				if(fifo_thread == -1){
					fprintf(stderr, "not open fifo for read\n");
					exit(EXIT_FAILURE);
				}

				SendInformation sendInfo;
				if (read(fifo_thread, &sendInfo, sizeof(sendInfo)) == -1){
					fprintf(stderr, "write() error\n");
				}
			    
			    #ifdef DEBUG
					fprintf(stderr, "Task completed by %s in %d seconds, cos(x)=%.2f, cost = %d\n"
	    				, sendInfo.providerName, sendInfo.spendTime, sendInfo.homeworkResult, sendInfo.cost);
				#endif
				write(connfd, &sendInfo, sizeof(sendInfo));
				
				/*int OKEY = 1;
				write(connfd, &OKEY, sizeof(OKEY));*/

				#ifdef DEBUG
					fprintf(stderr, "Task completed by %s in %d seconds, cos(x)=%.2f, cost = %d\n"
	    				, sendInfo.providerName, sendInfo.spendTime, sendInfo.homeworkResult, sendInfo.cost);
				#endif
			}
		} while(value>0);
	    if(sigaction(SIGINT, &sa, NULL) == -1){
   			perror("\tCan't SIGINT");
   			fprintf(fpLogFile, "\tCan't SIGINT");
   			//exit(EXIT_FAILURE);
   		}

   		close(connfd);
    }

	//close socket of the server
    close(listenfd);

    fprintf(stderr, "Termination signal received\n");
    fprintf(fpLogFile, "Termination signal received\n");
    fprintf(stderr, "Terminating all clients\n");
    fprintf(fpLogFile, "Terminating all clients\n");
    fprintf(stderr, "Terminating all providers\n");
    fprintf(fpLogFile, "Terminating all providers\n");

	fprintf(stderr, "\nStatistics\n");
	fprintf(fpLogFile, "\nStatistics\n");
	fprintf(stderr, " Name\t\tNumber of clients served\n");
	fprintf(stderr, "---------------------------------------------\n");
	fprintf(fpLogFile, " Name\t\tNumber of clients served\n");
	fprintf(fpLogFile, "---------------------------------------------\n");
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		fprintf(stderr, " %s %20d\n", pProviders[i].ProviderName, pProviders[i].numOfClients);
		fprintf(fpLogFile, " %s %20d\n", pProviders[i].ProviderName, pProviders[i].numOfClients);
	}

	fprintf(stderr, "\nGoodbye.\n");
	fprintf(fpLogFile, "\nGoodbye.\n");

	close(connfd);
	close(listenfd);
	fclose(fpLogFile);
	unlink(FIFO_THREAD);
	freeArr();
	threadpool_free(pool, 1);
	exit(EXIT_SUCCESS);

}

void *ThreadServer(void *arguman) {
	Client *datainfo = arguman;

	SendInformation sendInfo;

	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	providerDoHomework(*datainfo, &sendInfo);

	while(((fifo_thread = open(FIFO_THREAD, O_WRONLY)) < 0) && (errno == EINTR));
	if(fifo_thread == -1){
		fprintf(stderr, "not open fifo for write\n");
		exit(EXIT_FAILURE);
	}

	if (write(fifo_thread, &sendInfo, sizeof(sendInfo)) == -1){
		fprintf(stderr, "write() error\n");
	}
          
	//write(connfd, &sendInfo, sizeof(sendInfo));
	
	#ifdef DEBUG
		fprintf(stderr, "%s’s task completed by %s, cos(%d)=%.2f, cost = %d\n"
	    	, datainfo->ClientName, sendInfo.providerName, datainfo->Degree, sendInfo.homeworkResult, sendInfo.cost);

		fprintf(stderr, "SEND INFORMATION\n");
	#endif

	free(datainfo);
	
	pthread_exit(NULL);
}


void providerDoHomework(Client datainfo, SendInformation *sendInfo){
	gettimeofday(&sTime,NULL); // basladigi zamani bulundu
	
	int choose = chooseProvider(datainfo);
	int taskNumber = -1;

	if(choose == -1){
		fprintf(stderr, "NO PROVIDER IS AVAILABLE\n");
		fprintf(fpLogFile, "NO PROVIDER IS AVAILABLE\n");
		///exit(EXIT_SUCCESS)
	}
	else{
		sprintf(pProviders[choose].queue[pProviders[choose].indexQueue], "%s", datainfo.ClientName);
		fprintf(stderr, "Provider %s is processing task number %d: %d\n", pProviders[choose].ProviderName, pProviders[choose].indexQueue, datainfo.Degree);
		fprintf(fpLogFile, "Provider %s is processing task number %d: %d\n", pProviders[choose].ProviderName, pProviders[choose].indexQueue, datainfo.Degree);
	
		//fprintf(stderr, "\n\n****pProviders[choose].ProviderName: %s\n", pProviders[choose].ProviderName);

		sprintf(sendInfo->providerName, "%s", pProviders[choose].ProviderName);

		//fprintf(stderr, "****sendInfo->providerName: %s\n\n\n", sendInfo->providerName);

		sendInfo->cost = pProviders[choose].price;
		sendInfo->homeworkResult = solveHomework(datainfo.Degree);	

		pProviders[choose].indexQueue = pProviders[choose].indexQueue + 1;
		
		//pProviders[choose].indexQueue = pProviders[choose].indexQueue - 1;
		
		//fprintf(stderr, "************pProviders[choose].indexQueue: %d\n", pProviders[choose].indexQueue);

		taskNumber = pProviders[choose].indexQueue - 1;
		//taskNumber = pProviders[choose].indexQueue + 1;
		
		pProviders[choose].numOfClients = pProviders[choose].numOfClients + 1;
			
		gettimeofday(&lastTime,NULL); // bitis zamani

		sendInfo->spendTime = calculateTimeDifference(sTime,lastTime);

		fprintf(stderr, "Provider %s completed task number %d: cos(%d)=%.2f in  %d seconds.\n", sendInfo->providerName, taskNumber, datainfo.Degree, sendInfo->homeworkResult, sendInfo->spendTime);
		fprintf(fpLogFile, "Provider %s completed task number %d: cos(%d)=%.2f in  %d seconds.\n", sendInfo->providerName, taskNumber, datainfo.Degree, sendInfo->homeworkResult, sendInfo->spendTime);
	
		#ifdef DEBUG
			fprintf(stderr, "%s’s task completed by %s in %d seconds, cos(%d)=%.2f, cost = %d\n"
    			, datainfo.ClientName, sendInfo->providerName, sendInfo->spendTime, datainfo.Degree, sendInfo->homeworkResult, sendInfo->cost);
		#endif

		for (int i = 0; i < indexChildPIDs; ++i)
		{
			if(childPIDs[i] == datainfo.pid)
				childPIDs[i] = -1;
		}
	}
}


int chooseProvider(Client datainfo){
	pthread_mutex_lock(&mutex);

	int choose = -1;
	int index = -1;

		for (int i = 0; i < iSizeOfProviders; ++i)
		{	
			if(datainfo.Priority == 'Q' || datainfo.Priority == 'q'){
				index = findProvider(pPriorityQ[i].provider.ProviderName);
			}
			else if(datainfo.Priority == 'T' || datainfo.Priority == 't'){
				index = findProvider(pPriorityT[i].provider.ProviderName);
			}
			else if(datainfo.Priority == 'C' || datainfo.Priority == 'c'){
				index = findProvider(pPriorityC[i].provider.ProviderName);
			}

			if(pProviders[index].indexQueue < MAX_QUEUE_SIZE){
				choose = index;
			}
		}		


	fprintf(stderr, "Client %s (%c %d) connected, forwarded to provider %s\n", datainfo.ClientName, datainfo.Priority, datainfo.Degree, pProviders[choose].ProviderName);
	fprintf(fpLogFile, "Client %s (%c %d) connected, forwarded to provider %s\n", datainfo.ClientName, datainfo.Priority, datainfo.Degree, pProviders[choose].ProviderName);
	
	pthread_mutex_unlock(&mutex);
	return choose;
}

int findProvider(char providerName[MAX_STRING_SIZE]){
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		if(strcmp(providerName, pProviders[i].ProviderName) == 0)
			return i;
	}

	return -1;
}


void determinePriorityQueue(){
	Providers pTemp;

	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		sprintf(pPriorityQ[i].provider.ProviderName, "%s", pProviders[i].ProviderName);
		pPriorityQ[i].provider.performance = pProviders[i].performance;
		pPriorityQ[i].provider.price = pProviders[i].price;
		pPriorityQ[i].provider.duration = pProviders[i].duration;

		sprintf(pPriorityC[i].provider.ProviderName, "%s", pProviders[i].ProviderName);
		pPriorityC[i].provider.performance = pProviders[i].performance;
		pPriorityC[i].provider.price = pProviders[i].price;
		pPriorityC[i].provider.duration = pProviders[i].duration;

		sprintf(pPriorityT[i].provider.ProviderName, "%s", pProviders[i].ProviderName);
		pPriorityT[i].provider.performance = pProviders[i].performance;
		pPriorityT[i].provider.price = pProviders[i].price;
		pPriorityT[i].provider.duration = pProviders[i].duration;
	}

/*quality'e gore siralama*/
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		for (int j = i+1; j < iSizeOfProviders; ++j)
		{
			if (pPriorityQ[i].provider.performance < pPriorityQ[j].provider.performance) 
            {
            	sprintf(pTemp.ProviderName, "%s", pPriorityQ[i].provider.ProviderName);
                pTemp.performance = pPriorityQ[i].provider.performance;
                pTemp.price = pPriorityQ[i].provider.price;
                pTemp.duration = pPriorityQ[i].provider.duration;

                sprintf(pPriorityQ[i].provider.ProviderName, "%s", pPriorityQ[j].provider.ProviderName);
                pPriorityQ[i].provider.performance = pPriorityQ[j].provider.performance;
                pPriorityQ[i].provider.price = pPriorityQ[j].provider.price;
                pPriorityQ[i].provider.duration = pPriorityQ[j].provider.duration;


                sprintf(pPriorityQ[j].provider.ProviderName, "%s", pTemp.ProviderName);
                pPriorityQ[j].provider.performance = pTemp.performance;
                pPriorityQ[j].provider.price = pTemp.price;
                pPriorityQ[j].provider.duration = pTemp.duration;
            }
		}
	}


/*time'a gore siralama*/
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		for (int j = i+1; j < iSizeOfProviders; ++j)
		{
			if (pPriorityT[i].provider.duration < pPriorityT[j].provider.duration) 
            {
            	sprintf(pTemp.ProviderName, "%s", pPriorityT[i].provider.ProviderName);
                pTemp.performance = pPriorityT[i].provider.performance;
                pTemp.price = pPriorityT[i].provider.price;
                pTemp.duration = pPriorityT[i].provider.duration;

                sprintf(pPriorityT[i].provider.ProviderName, "%s", pPriorityT[j].provider.ProviderName);
                pPriorityT[i].provider.performance = pPriorityT[j].provider.performance;
                pPriorityT[i].provider.price = pPriorityT[j].provider.price;
                pPriorityT[i].provider.duration = pPriorityT[j].provider.duration;


                sprintf(pPriorityT[j].provider.ProviderName, "%s", pTemp.ProviderName);
                pPriorityT[j].provider.performance = pTemp.performance;
                pPriorityT[j].provider.price = pTemp.price;
                pPriorityT[j].provider.duration = pTemp.duration;
            }
		}
	}

/*Cost 'a gore siralama*/
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		for (int j = i+1; j < iSizeOfProviders; ++j)
		{
			if (pPriorityC[i].provider.price < pPriorityC[j].provider.price) 
            {
            	sprintf(pTemp.ProviderName, "%s", pPriorityC[i].provider.ProviderName);
                pTemp.performance = pPriorityC[i].provider.performance;
                pTemp.price = pPriorityC[i].provider.price;
                pTemp.duration = pPriorityC[i].provider.duration;

                sprintf(pPriorityC[i].provider.ProviderName, "%s", pPriorityC[j].provider.ProviderName);
                pPriorityC[i].provider.performance = pPriorityC[j].provider.performance;
                pPriorityC[i].provider.price = pPriorityC[j].provider.price;
                pPriorityC[i].provider.duration = pPriorityC[j].provider.duration;


                sprintf(pPriorityC[j].provider.ProviderName, "%s", pTemp.ProviderName);
                pPriorityC[j].provider.performance = pTemp.performance;
                pPriorityC[j].provider.price = pTemp.price;
                pPriorityC[j].provider.duration = pTemp.duration;
            }
		}
	}
}

double solveHomework(int CalculateValue){
  double radians;   // value of x in radians
  double cosine;    // cosine of x

  radians = CalculateValue*PI/180.0;
  cosine = 1-calculateTaylorSeries(radians);

  return cosine;
}

double calculateTaylorSeries(double radians)
{
	int i, x;
	double cosine;
	x=0;
	i=0;
	cosine = 0;
	while(i<NUMBER_OF_TERMS-1)
	{
		i++;
		x=x+2;
		if(i%2 == 0){
			cosine = cosine - (power(radians, x)/factorial(x));
		}
		else if(i%2 != 0){
			cosine = cosine + (power(radians, x)/factorial(x));
		}

	}
	return cosine;
}

double factorial(double x)
{
      double i, total;
      i=x;
      total=x;
      while(i>1)
      {
         i--;
         total = total * i;
      }
      return total;
}

double power(double x, double y)
{
      double i, z;
      i=0;
      z = x;
      while (i<(y-1))
      {
            i++;
            x = x * z;
      }
      return x;
}

void initialize(){
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		pPriorityQ[i].provider.indexQueue = 0;
		pPriorityC[i].provider.indexQueue = 0;
		pPriorityT[i].provider.indexQueue = 0;
	}
}


void readFile(){
	FILE *fp = fopen(sProviderFileName, "r");
	char tempStr[COLUMN_SIZE];
	int i=0;


	fgets(tempStr, COLUMN_SIZE, fp);
	while(!feof(fp)){
		
		fscanf(fp, "%s %d %d %d", &pProviders[i].ProviderName, &pProviders[i].performance, &pProviders[i].price, &pProviders[i].duration);
		//fprintf(stderr, "name: %s\t performance: %d\t price: %d\t duration: %d\n",  pProviders[i].ProviderName, pProviders[i].performance, pProviders[i].price, pProviders[i].duration);

		i++;
	}


	fclose(fp);
}

/*
 * Programin ne kadar calistigini hesaplamak icin kullanildi.
 */
long calculateTimeDifference(struct timeval start, struct timeval finish){
	long int finTime = finish.tv_sec;
	long int ufinTime = finish.tv_usec;
	long int startTime = start.tv_sec;
	long int ustartTime = start.tv_usec;
	long int result = (finTime - startTime)*1000.0f + (ufinTime-ustartTime)/1000.0f;
    return result;
}

void usage(){
	fprintf(stderr, "--------------------------------------------------------------------------------\n");
	fprintf(stderr, "\t./homeworkServer iPortAdress \"sProviderFileName\" \"sLogFileName\"\n");
	fprintf(stderr, "--------------------------------------------------------------------------------\n");
}

/*
 * Dosyadaki provider sayisinin bulundugu fonksiyon
 */
void findNumOfpProviders(){
	FILE *fp = fopen(sProviderFileName, "r");
	char tempStr[COLUMN_SIZE];

	do{
		/*result = fscanf(fp, "%c", &ch);
		if(ch == '\n')
			size++;*/
		fgets(tempStr, COLUMN_SIZE, fp);
		iSizeOfProviders++;
		//fprintf(stderr, "ch: -%c-\n", ch);
	}while(!feof(fp));

	iSizeOfProviders--;

	#ifdef DEBUG
		fprintf(stderr, "iSizeOfProviders: %d\n", iSizeOfProviders);
	#endif

	fclose(fp);
}

void freeArr(){
	free(pPriorityC);
	free(pPriorityQ);
	free(pPriorityT);
	free(pProviders);
}

void printProviders(){
	fprintf(stderr, "Name \t Performance \t Price \t Duration\n");
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		fprintf(stderr, "%s \t     %d \t\t  %d \t    %d\n", pProviders[i].ProviderName, pProviders[i].performance, pProviders[i].price, pProviders[i].duration);
	}
	fprintf(stderr, "\n");
}

void printPriority(PriorityOfProviders *p, int size){
	for (int i = 0; i < size; ++i)
		fprintf(stderr, "name: %s \t performance: %d \t price: %d \t duration: %d \n", p[i].provider.ProviderName, p[i].provider.performance, p[i].provider.price, p[i].provider.duration);
	
	fprintf(stderr, "\n");
}

void signalHandler(int sig){
	fprintf(stderr, "\nSERVER SHUTDOWN\n\n");
	fprintf(fpLogFile, "\nSERVER SHUTDOWN\n\n");

	for (int i = 0; i < indexChildPIDs; ++i)
	{
		if(childPIDs[i] != -1)
			kill(sig, childPIDs[i]);
	}

	doneFlag = 1;

	fprintf(stderr, "Termination signal received\n");
    fprintf(fpLogFile, "Termination signal received\n");
    fprintf(stderr, "Terminating all clients\n");
    fprintf(fpLogFile, "Terminating all clients\n");
    fprintf(stderr, "Terminating all providers\n");
    fprintf(fpLogFile, "Terminating all providers\n");

	fprintf(stderr, "\nStatistics\n");
	fprintf(fpLogFile, "\nStatistics\n");
	fprintf(stderr, " Name\t\tNumber of clients served\n");
	fprintf(stderr, "---------------------------------------------\n");
	fprintf(fpLogFile, " Name\t\tNumber of clients served\n");
	fprintf(fpLogFile, "---------------------------------------------\n");
	for (int i = 0; i < iSizeOfProviders; ++i)
	{
		fprintf(stderr, " %s %20d\n", pProviders[i].ProviderName, pProviders[i].numOfClients);
		fprintf(fpLogFile, " %s %20d\n", pProviders[i].ProviderName, pProviders[i].numOfClients);
	}

	fprintf(stderr, "\nGoodbye.\n");
	fprintf(fpLogFile, "\nGoodbye.\n");

	close(connfd);
	close(listenfd);
	fclose(fpLogFile);
	unlink(FIFO_THREAD);
	freeArr();
	threadpool_free(pool, 1);

	_exit(EXIT_SUCCESS);
}