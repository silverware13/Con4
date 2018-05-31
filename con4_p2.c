/* Concurrency 4
 * CS444 Spring2018
 * ----------------
 * Name: Zachary Thomas
 * Email: thomasza@oregonstate.edu
 * Date: 5/30/2018
 * -------------------------------
 * Cigarette smokers problem
 *
 * An agent and three smokers. The smokers loop
 * forever, first waiting for ingredients, then
 * making and smoking cigarettes. The ingredients
 * are tobacco, paper, and matches. We assume that
 * the agent has an infinite supply of all three ingredients,
 * and each smoker has an infinite supply of one of the ingredients.
 */

#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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
void* agent_thread();
void get_hair_cut();
void cut_hair();
int random_range(int min_val, int max_val);

//create semaphore(s)
sem_t waiting_chairs;
sem_t sleeping_agent;
sem_t agent_chair;
sem_t agent_tools;

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
	chairs_waiting = chairs - 1;

	//initalize semaphore(s)
	sem_init(&waiting_chairs, 0, 1);
	sem_init(&sleeping_agent, 0, 1);
	sem_init(&agent_chair, 0, 1);
	sem_init(&agent_tools, 0, 1);

	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	
	
	//create threads and wait for their completion
	spawn_threads(chairs);
	
	//destroy semaphore(s)	
	sem_destroy(&waiting_chairs);	
	sem_destroy(&sleeping_agent);	
	sem_destroy(&agent_chair);	
	sem_destroy(&agent_tools);	
	
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

	printf(ANSI_COLOR_CYAN "\nThe agent enters his barbershop." ANSI_COLOR_RESET "\n");
	
	if(chairs){
		printf(ANSI_COLOR_CYAN "Inside his shop is %d waiting chairs, and 1 agent chair." ANSI_COLOR_RESET "\n\n", chairs - 1);
	} else {
		printf(ANSI_COLOR_CYAN "Inside his shop there are no waiting chairs, and no agent chair." ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_CYAN "How does he stay in business?" ANSI_COLOR_RESET "\n\n");
	}

	//we have five more customer than possible chairs
	int i;
	for(i = chairs + 4; i > 0; i--){
		pthread_create(&thrd, NULL, customer_thread, NULL);
	}

	//we create the agent	
	pthread_create(&thrd, NULL, agent_thread, NULL);	

	//join thread (this should never finish)
	pthread_join(thrd, NULL);
}

/* Function: agent_thread
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * Sleeps until customers show up.
 * When awake, cuts a customers hair.
 * When done looks for next customer to cut hair, if none exist sleep.
 */
void* agent_thread()
{
	while(true){
		if(sem_trywait(&agent_chair) != -1){
			sem_post(&agent_chair);
		} else {
			//give a haircut
			sem_wait(&agent_tools);
			while(sem_trywait(&agent_chair) != -1){
				sem_post(&agent_chair);
			}
			cut_hair();
			continue;
		}
		sem_wait(&waiting_chairs);
		if(!customers_waiting){
			printf(ANSI_COLOR_CYAN "The agent has fallen asleep." ANSI_COLOR_RESET "\n");
			sem_post(&waiting_chairs);
			sem_wait(&sleeping_agent);
			while(sem_trywait(&sleeping_agent) == -1){
				sleep(1);	
			}
			printf(ANSI_COLOR_CYAN "The agent has been woken up by a customer." ANSI_COLOR_RESET "\n");
			sem_post(&sleeping_agent);
			//give a haircut
			sem_wait(&agent_tools);
			while(sem_trywait(&agent_chair) != -1){
				sem_post(&agent_chair);
			}
			cut_hair();
		} else {
			sem_post(&waiting_chairs);
			//give a haircut
			sem_wait(&agent_tools);
			while(sem_trywait(&agent_chair) != -1){
				sem_post(&agent_chair);
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
 * Enters agentshop.
 * If agentshop is full leaves.
 * If chair is avaliable, but agent is busy. sits down.
 * If agent asleep, wakes barber
 */
void* customer_thread()
{
	while(true){
		sleep(random_range(3, 30));
		printf(ANSI_COLOR_YELLOW "Customer entered agentshop." ANSI_COLOR_RESET "\n");
		//if the agent is asleep the customer wakes him up
		sem_post(&sleeping_agent);
		//if there are no chairs just leave
		if(chairs_waiting == -1){
			printf(ANSI_COLOR_YELLOW "There aren't any chairs in this agentshop. Customer left the barbershop." ANSI_COLOR_RESET "\n");
			continue;
		}
		//if the agent chair is empty sit in it
		if(sem_trywait(&agent_chair) != -1){
				//get a haircut
				printf(ANSI_COLOR_YELLOW "Customer sat down in the agent chair." ANSI_COLOR_RESET "\n");
				while(sem_trywait(&agent_tools) != -1){
					sem_post(&agent_tools);
				}
				get_hair_cut();
				continue;
		}
		//if a waiting chair is empty sit in it
		sem_wait(&waiting_chairs);
		if(customers_waiting < chairs_waiting){
			//we wait in a chair to get a haircut
			customers_waiting++;	
			printf(ANSI_COLOR_YELLOW "Customer started waiting. %d of %d waiting chairs filled." 
				ANSI_COLOR_RESET "\n", customers_waiting, chairs_waiting);
			sem_post(&waiting_chairs);
			sem_wait(&agent_chair);
			//get a haircut	
			printf(ANSI_COLOR_YELLOW "Customer sat down in the agent chair." ANSI_COLOR_RESET "\n");
			sem_wait(&waiting_chairs);
			customers_waiting--;	
			sem_post(&waiting_chairs);
			while(sem_trywait(&agent_tools) != -1){
				sem_post(&agent_tools);
			}
			get_hair_cut();
			continue;
		} else {
			//we leave, no chairs to wait in
			printf(ANSI_COLOR_YELLOW "All waiting chairs are full. Customer left the agentshop." ANSI_COLOR_RESET "\n");
			sem_post(&waiting_chairs);
			continue;	
		}
	}
}

void get_hair_cut()
{
	printf(ANSI_COLOR_YELLOW "A customer is getting their haircut." ANSI_COLOR_RESET "\n");
	sleep(5);
	printf(ANSI_COLOR_YELLOW "A customer finished getting their haircut and left the agentshop." ANSI_COLOR_RESET"\n");
	sem_post(&agent_chair);
}

void cut_hair()
{
	printf(ANSI_COLOR_CYAN "The agent started cutting a customers hair." ANSI_COLOR_RESET "\n");
	sleep(5);
	printf(ANSI_COLOR_CYAN "The agent finished cutting a customers hair." ANSI_COLOR_RESET "\n");
	sem_post(&agent_tools);
	sleep(1);
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
