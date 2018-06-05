/**
 * Gozde DOGAN
 * 131044019
 * Sistem Programlama
 * FINAL PROJESI
 *
 * 131044019_client.c
 */

#include "131044019_client.h"


int main(int argc, char const *argv[])
{
	if(argc != 6){
		usage();
		exit(EXIT_FAILURE);
	}

	sprintf(client.ClientName, "%s", argv[1]);
	client.Priority = argv[2][0];
	client.Degree = atoi(argv[3]);
	client.pid = getpid();

	sprintf(sServerAddress, "%s", argv[4]);
	iPortAdress = atoi(argv[5]);


	#ifdef DEBUG
		fprintf(stderr, "ClientName: %s \t Priority: %c \t Degree: %d \t sServerAddress: %s \t PortAdress: %d\n"
			, client.ClientName, client.Priority, client.Degree, sServerAddress, iPortAdress);
	#endif
	
	fprintf(stderr, "\nClient %s is requesting %c %d from server %s:%d\n\n", client.ClientName, client.Priority, client.Degree, sServerAddress, iPortAdress);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        fprintf(stderr, "ERROR opening socket\n");

    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(iPortAdress);
    

    if(inet_pton(AF_INET, sServerAddress, &servaddr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }
   
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
        fprintf(stderr, "ERROR connecting\n");

    
    gettimeofday(&sTime,NULL); // basladigi zamani bulundu	
    write(sockfd, &client, sizeof(client));

    //sleep(3);

    //read(sockfd, &recvInfo, sizeof(recvInfo));
    int value;
    while((value = read(sockfd, &recvInfo, sizeof(recvInfo))) <= 0);

   /* int OKEY = 0;
    read(sockfd, &OKEY, sizeof(OKEY));

    fprintf(stderr, "OKEY= %d\n", OKEY);*/

    gettimeofday(&lastTime,NULL); // bitis zamani

    fprintf(stderr, "%s’s task completed by %s in %.2f seconds, cos(%d)=%.2f, cost is %dTL, total time spent %.2f seconds.\n"
    	, client.ClientName, recvInfo.providerName, recvInfo.spendTime, client.Degree, recvInfo.homeworkResult, recvInfo.cost, calculateTimeDifference(sTime,lastTime));
    

    // close socket for client
    close(sockfd);

	return 0;
}

void usage(){
	fprintf(stderr, "----------------------------------------------------------------------------------------\n");
	fprintf(stderr, "\t./homeworkClient \"ClientName\" Priority Degree ServerAddress PortAdress\"\n");
	fprintf(stderr, "----------------------------------------------------------------------------------------\n");
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
