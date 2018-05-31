/* Concurrency 4
 * CS444 Spring2018
 * ----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 5/30/2018
 * -------------------------------
 * The barbershop problem
 *
 * A barbershop consists of a waiting room with n chairs,
 * and the barber room containing the barber chair. If there
 * are no customers to be served, the barber goes to sleep. If
 * a customer enters the barbershop and all chairs are occupied,
 * then the customer leaves the shop. If the barber is busy, but
 * chairs are available, then the customer sits in one of the free
 * chairs. If the barber is asleep, the customer wakes up the barber
 */

#include "mt19937ar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <semaphore.h>

//global variable(s)
int customers_waiting = 0;
int chairs_waiting = 0;

//function prototype(s)
void spawn_threads(int chairs);
void* customer_thread();
void* barber_thread();
void get_hair_cut();
void cut_hair();
int random_range(int min_val, int max_val);

//create semaphore(s)
sem_t waiting_chairs;
sem_t sleeping_barber;
sem_t barber_chair;
sem_t barber_tools;

int main(int argc, char **argv)
{
	//user must enter correct number of args
	if(argc < 2 || argc > 2){
		printf("USEAGE: %s [number of chairs]\n", argv[0]);
		return 0;
	}

	//user must enter number of chairs as an unisgned int
	if(!isdigit(*argv[1])){
		printf("Please enter argument as unsigned integer.\n");
		return 0;
	}

	//define chairs	
	int chairs;
	chairs = strtol(argv[1], NULL, 10);
	if(chairs < 0) chairs = 0; //no negative chairs.	
	chairs_waiting = chairs - 1;

	//initalize semaphore(s)
	sem_init(&waiting_chairs, 0, 1);
	sem_init(&sleeping_barber, 0, 1);
	sem_init(&barber_chair, 0, 1);
	sem_init(&barber_tools, 0, 1);

	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	
	
	//create threads and wait for their completion
	spawn_threads(chairs);
	
	//destroy semaphore(s)	
	sem_destroy(&waiting_chairs);	
	sem_destroy(&sleeping_barber);	
	sem_destroy(&barber_chair);	
	sem_destroy(&barber_tools);	
	
	return 0;
}

/* Function: spawn_threads
 * -----------------------
 * Spawns threads then waits for threads to
 * finish execution and join. Since these threads will run forever, we expect
 * to block here indefinitely.
 */
void spawn_threads(int chairs)
{
	pthread_t thrd;

	printf("\nThe barber enters his barbershop.\n");
	
	if(chairs){
		printf("Inside his shop is %d waiting chairs, and 1 barber chair.\n\n", chairs - 1);
	} else {
		printf("Inside his shop there are no waiting chairs, and no barber chair.\n");
		printf("How does he stay in business?\n\n");
	}

	//we have five more customer than possible chairs
	int i;
	for(i = chairs + 4; i > 0; i--){
		pthread_create(&thrd, NULL, customer_thread, NULL);
	}

	//we create the barber	
	pthread_create(&thrd, NULL, barber_thread, NULL);	

	//join thread (this should never finish)
	pthread_join(thrd, NULL);
}

/* Function: barber_thread
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * Sleeps until customers show up.
 * When awake, cuts a customers hair.
 * When done looks for next customer to cut hair, if none exist sleep.
 */
void* barber_thread()
{
	while(true){
		sem_wait(&waiting_chairs);
		if(!customers_waiting){
			printf("The barber has fallen asleep\n");
			sem_post(&waiting_chairs);
			sem_wait(&sleeping_barber);
			while(sem_trywait(&sleeping_barber) == -1){
				sleep(1);	
			}
			printf("The barber has been woken up by a customer.\n");
			sem_post(&sleeping_barber);
			//give a haircut
			wait(&barber_tools);
			while(sem_trywait(&barber_chair) != -1){
				sem_post(&barber_chair);
			}
			cut_hair();
		} else {
			sem_post(&waiting_chairs);
			//give a haircut
			wait(&barber_tools);
			while(sem_trywait(&barber_chair) != -1){
				sem_post(&barber_chair);
			}
			cut_hair();
		}
	}
}

/* Function: customer_thread
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * Waits a random amount of time.
 * Enters barbershop.
 * If barbershop is full leaves.
 * If chair is avaliable, but barber is busy. sits down.
 * If barber asleep, wakes barber
 */
void* customer_thread()
{
	while(true){
		sleep(random_range(1, 30));
		printf("Customer entered barbershop.\n");
		//if the barber is asleep the customer wakes him up
		sem_post(&sleeping_barber);
		//enters barbershop, take a seat and wait (or get a haircut if empty)
		sem_wait(&waiting_chairs);
		if(customers_waiting < chairs_waiting){
			//if the barber is not busy try to get a haircut
			if(sem_trywait(&barber_chair) == -1){
				customers_waiting++;	
				printf("Customer started waiting. %d of %d waiting chairs filled.\n", customers_waiting, chairs_waiting);
				sem_post(&waiting_chairs);
				sem_wait(&barber_chair)
			} else {
				//get a haircut
				printf("Customer sat down in the barber chair");
				sem_post(&waiting_chairs);
				while(sem_trywait(&barber_tools) != -1){
					sem_post(&barber_tools);
				}
				get_hair_cut();
				continue;
			}
		} else {
			printf("All waiting chairs are full. Customer left the barbershop.\n");
			sem_post(&waiting_chairs);
			continue;	
		}
		//get a haircut	
		printf("Customer sat down in the barber chair");
		sem_wait(&waiting_chairs);
		customers_waiting--;	
		sem_post(&waiting_chairs);
		while(sem_trywait(&barber_tools) != -1){
			sem_post(&barber_tools);
		}
		get_hair_cut();
	}
}

void get_hair_cut()
{
	printf("A customer is getting their haircut.");
	sleep(random_range(5));
	printf("A customer finished getting their haircut and left the barbershop.");
}

void cut_hair()
{
	printf("The barber started cutting a customers hair.");
	sleep(random_range(5));
	printf("The barber finished cutting a customers hair.");
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
