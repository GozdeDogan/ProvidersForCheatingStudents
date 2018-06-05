all: 

	clear
	gcc 131044019_server.c threadpool.c -o HomeworkServer -lm -lpthread 
	gcc 131044019_client.c -o HomeworkClient


clean:
	rm *.o HomeworkServer
	rm *.o HomeworkClient
