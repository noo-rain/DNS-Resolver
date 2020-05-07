#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "util.h"
#include "util.c"


#define MAX 100			
pthread_mutex_t buffer_mutex; //mutex for buffer
pthread_mutex_t serviced_lock; //protects serviced.txt
pthread_mutex_t results_lock; //protects results.txt
pthread_mutex_t thread_count; 
pthread_cond_t condc, condp;
int buffer = 0;
char buffer_char[100][50]; //character buffer for the hostnames
char serviced[50]; 
char results[50];

void* producer(char* fileName) { //requester function	
	FILE* inputfp = fopen(fileName, "r"); //this will open up the file we passed in to be read
	pthread_mutex_lock(&thread_count);
	int number_of_threads = 0; //initalizing thread count
	pthread_mutex_unlock(&thread_count);
	char hostName[1025]; //allocating space for the hostname
	pthread_mutex_lock(&buffer_mutex); //buffer is protected now, only one thread can enter critical section
	while(fscanf(inputfp, "%1024s", hostName) != EOF){ //reads the file until we get to the end of the file 
		buffer++; //increments the buffer
		//printf("Here is hostName %s\n", hostName);
		strcpy(buffer_char[buffer], hostName); 
		//adds the hostname into the buffer
		//printf("Buffer size is now %d\n", buffer);
	}

	pthread_mutex_lock(&serviced_lock); //locked access to serviced.txt, entering critical section
	FILE *f = fopen(serviced, "w");
	if(f == NULL) {
		printf("Error: Bad file output path!");
		exit(1);
	}
	pthread_mutex_lock(&thread_count);
		number_of_threads++; //increments thread count
	pthread_mutex_unlock(&thread_count);
	fprintf(f, "Thread:%lu serviced %d files\n", (unsigned long) pthread_self(), number_of_threads); 
	fclose(f);
	pthread_mutex_unlock(&serviced_lock);
	
  pthread_cond_signal(&condc);//signals the consumer 
  pthread_mutex_unlock(&buffer_mutex); 
  pthread_exit(0); //terminates threads
}

void* consumer(void *ptr) {

char hostName[1025]; //allocates space for hostname
char hostNamesArray[30]; //allocates space for hostnames array
char IP[1025]; //allocates space for IP address
 
    pthread_mutex_lock(&buffer_mutex);	//protects the buffer
    while (buffer == 0)			//If there is nothing in the buffer then wait 
      pthread_cond_wait(&condc, &buffer_mutex);
    while(buffer > 0) {
    //printf("Consumer reads %d\n", buffer);
	for(int i=1; i<30; i++){ 
    		strcpy(hostNamesArray, buffer_char[i]); //copying buffer char array into hostnames array
		int flag = 0;
		if(dnslookup(hostNamesArray, IP, sizeof(IP))){
				fprintf(stderr, "invalid hostname: %s \n", hostNamesArray); //this will print to the terminal instead of a file
				flag = 1; 
			
		}

		pthread_mutex_lock(&results_lock);
		FILE *ftpr = fopen(results, "a"); //appends

			if(ftpr == NULL) {
				printf("Error: Bad file output path! \n");
				exit(1);
			}
			if(flag == 1){
				fprintf(ftpr, "invalid hostname: %s \n", hostNamesArray);
			
}
			else {
				//printf("%s, %s \n", hostNamesArray, IP);
				fprintf(ftpr, "%s, %s \n", hostNamesArray, IP);
			}
		fclose(ftpr);
		pthread_mutex_unlock(&results_lock);
		buffer--;

	//fclose(ftpr);


}
    pthread_cond_signal(&condp);	// wake up producer 
    pthread_mutex_unlock(&buffer_mutex);	//release the buffer 
  }
	
  pthread_exit(0); //terminates threads
}




int main(int argc, char **argv) {
	
	struct timeval start, end; //to keep track of how long it takes
	gettimeofday(&start, NULL);
	unsigned long int req;
	unsigned long int res;
	char* req_char;
	char* res_char;

	req = strtol(argv[1], &req_char, 10); //user input->converts the number of requester and resolver threads
	res = strtol(argv[2], &res_char, 10);


	strcpy(serviced, argv[3]); //3rd command line value should be serviced.txt
	strcpy(results, argv[4]); //4th command line value should be results.txt

	//printf("This is the value of the requesters: %d\n", req);
	//printf("This is the value of the resolvers: %d\n", res);

	if(req > 10 || res < 1) { //input validating
		printf("Error: Number of requesters cannot be larger than 10.\n");
		exit(1);
	}
	if(res > 5 || res < 1) {
		printf("Error: Number of resolvers cannot be larger than 5.\n");
		exit(1);
	}	
	if((argc-5) > 10){ //not tested yet
		printf("Error: Number of input files cannot be larger than 10.\n");
		exit(1);	
	}
	
	//here is where I tried to loop through multiple files 
	//for(int t=0; t<(argv-5); t++){
		//char* inputFiles[10]; //char* inputFiles[10];
		//inputFiles[t] = argv[t+5]; //input files [10] = argv[5]
		//char* fileName = inputFiles[t]; //loop through, set filename to each one? 
	
	
	char* inputFiles[1]; 
	inputFiles[0] = argv[5]; 
	char* fileName = inputFiles[0]; 

	pthread_t requester_threads[req]; 
	pthread_t resolver_threads[res];

	//here I initialize the mutex and condition variables
	pthread_mutex_init(&buffer_mutex, NULL);	
	pthread_mutex_init(&results_lock, NULL);
	pthread_mutex_init(&serviced_lock, NULL);
	pthread_cond_init(&condc, NULL); //Initialize consumer condition variable 
	pthread_cond_init(&condp, NULL); //Initialize producer condition variable 
	
	//here I create the threads
	for(int i=0; i < req; i++) {
		pthread_create(&requester_threads, NULL, producer, fileName);
	}

	for(int i=0; i<req; i++){ //waits for requester threads to terminate
		pthread_join(requester_threads[i], NULL);
	}

	for(int i=0; i < res; i++){ //creates resolver threads
		pthread_create(&resolver_threads, NULL, consumer, NULL);
	}
	

	// Wait for the threads to finish
	// Otherwise main might run to the end and kill the entire process when it exits.

	for(int i=0; i < res; i++){ //waits for resolver threads to terminate
		pthread_join(resolver_threads[i], NULL);
	}
	

	pthread_mutex_destroy(&buffer_mutex);	/* Free up the_mutex */
	pthread_mutex_destroy(&results_lock);
	pthread_mutex_destroy(&serviced_lock);
	pthread_mutex_destroy(&thread_count);
	pthread_cond_destroy(&condc);		/* Free up consumer condition variable */
	pthread_cond_destroy(&condp);		/* Free up producer condition variable */
	gettimeofday(&end, NULL);
	double total_time;
	total_time = (end.tv_sec - start.tv_sec) * 1e6;
	total_time = (total_time + (end.tv_usec = start.tv_usec)) * 1e-6;
	printf("Time taken: %f seconds \n", total_time);

}
















