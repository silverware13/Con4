/* Concurrency 3
 * CS444 Spring2018
 * ----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 5/11/2018
 * -------------------------------
 * There is a resource that may have up
 * to three threads use at the same time.
 * Once three threads are using it at the same
 * time, all three must leave before new
 * threads can use it.
 */

#define THREADS 6
#include "mt19937ar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>

//linked list node
typedef struct node{
	int val;
	struct node* next;
} node;

//global variable(s)
int resource_users = 0;
int resource_full = 0;

//function prototype(s)
void spawn_threads();
void* resource_thread();
int random_range(int min_val, int max_val);

//create mutex lock(s)
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	
	
	//create threads and wait for their completion
	spawn_threads();
	
	//destroy mutex lock(s)	
	pthread_mutex_destroy(&lock);
	
	return 0;
}

/* Function: spawn_threads
 * -----------------------
 * Spawns threads then waits for threads to
 * finish execution and join. Since these threads will run forever, we expect
 * to block here indefinitely.
 */
void spawn_threads()
{
	pthread_t thrd;

	printf("\nCreating resource threads.\n\n");
	
	int i;
	for(i = THREADS; i > 0; i--){
		pthread_create(&thrd, NULL, resource_thread, NULL);
	}

	//join thread (this should never finish)
	pthread_join(thrd, NULL);
}

/* Function: resource_thread
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This thread thinks about using a resource for 1-3 seconds.
 * Then it will access the resource if there are less than 3 users,
 * but only if there wasn't previously 3 users, if there were this thread
 * waits until there are zero.
 * Next it uses the resource for 0-2 seconds.
 * Lastly it leaves the resource.
 */
void* resource_thread()
{
	while(true){
		//think about using the resource
		sleep(random_range(1, 3));
		
		//access the resource
		pthread_mutex_lock(&lock);
		if(resource_users < 3 && !resource_full){
			resource_users++;
			if(resource_users == 3){
				resource_full = 1;
			}
			printf("A process has started using the resource.\nRESOURCE USERS = %d\n", resource_users); 
		} else {
			pthread_mutex_unlock(&lock);
			continue;
		}
		pthread_mutex_unlock(&lock);
		
		//use the resource
		sleep(random_range(0,2));
		
		//leave the resource
		pthread_mutex_lock(&lock);
		resource_users--;
		if(resource_users == 0){
			resource_full = 0;
		}
		printf("A process has finished using the resource.\nRESOURCE USERS = %d\n", resource_users); 
		pthread_mutex_unlock(&lock);
	}
}

/* Function: random_range
 * ----------------------
 * This function finds a random number between a min and max value (inclusive).
 * The random value is created using rdrand x86 ASM on systems that support it,
 * and it uses Mersenne Twister on systems that do not support rdrand.
 *
 * min_val: The lowest possible random number.
 * max_val: The highest possible random number.
 *
 * returns: A random number in the given range. In the case that min_val is
 *          greater than max_val this function returns -1.
 */
int random_range(int min_val, int max_val)
{
	if(min_val > max_val)
		return -1;

	int output;
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	if(ecx & 0x40000000){
		//use rdrand
	__asm__ __volatile__(
	                     "rdrand %0"
                             : "=r"(output)
	                     );
	} else {
		//use mt19937
		output = genrand_int32();
	}

	//get random number in the range requested 
	output = (abs(output) % (max_val + 1 - min_val)) + min_val;
	return output;
}
