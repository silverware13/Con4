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
 * the agent hands out two ingredients and waits for smokers
 * with the third ingredient to collect.
 */

#define PAPER 1
#define MATCHES 2
#define TOBACCO 3
#define NUM_SMOKERS 3
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

//function prototype(s)
void spawn_threads();
void* smoker_thread();
void* agent_thread_A();
void* agent_thread_B();
void* agent_thread_C();
int random_range(int min_val, int max_val);

//create semaphore(s)
sem_t paper;
sem_t tobacco;
sem_t matches;

int main(int argc, char **argv)
{
	//initalize semaphore(s)
	sem_init(&paper, 0, 1);
	sem_init(&tobacco, 0, 1);
	sem_init(&matches, 0, 1);

	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	
	
	//create threads and wait for their completion
	spawn_threads();
	
	//destroy semaphore(s)	
	sem_destroy(&paper);	
	sem_destroy(&tobacco);	
	sem_destroy(&matches);	
	
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

	printf(ANSI_COLOR_CYAN "\n There is 1 agent and 3 smokers." ANSI_COLOR_RESET "\n\n");
	
	for(i = NUM_SMOKERS; i > 0; i--){
		pthread_create(&thrd, NULL, smoker_thread, NULL);
	}

	//we create the agent	
	pthread_create(&thrd, NULL, agent_thread_A, NULL);	
	pthread_create(&thrd, NULL, agent_thread_B, NULL);	
	pthread_create(&thrd, NULL, agent_thread_C, NULL);	

	//join thread (this should never finish)
	pthread_join(thrd, NULL);
}

/* Function: agent_thread_A
 * -------------------------
 * This function is called by a new resource thread when it is created.
 */
void* agent_thread_A()
{
	while(true){
			
	}
}

/* Function: agent_thread_B
 * -------------------------
 * This function is called by a new resource thread when it is created.
 */
void* agent_thread_B()
{
	while(true){
			
	}
}

/* Function: agent_thread_C
 * -------------------------
 * This function is called by a new resource thread when it is created.
 */
void* agent_thread_C()
{
	while(true){
			
	}
}

/* Function: smoker_thread
 * -------------------------
 * This function is called by a new resource thread when it is created.
 */
void* smoker_thread()
{
	while(true){
		sleep(random_range(3, 30));
		printf(ANSI_COLOR_YELLOW "Customer entered agentshop." ANSI_COLOR_RESET "\n");
		//if the agent is asleep the smoker wakes him up
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
		if(smokers_waiting < chairs_waiting){
			//we wait in a chair to get a haircut
			smokers_waiting++;	
			printf(ANSI_COLOR_YELLOW "Customer started waiting. %d of %d waiting chairs filled." 
				ANSI_COLOR_RESET "\n", smokers_waiting, chairs_waiting);
			sem_post(&waiting_chairs);
			sem_wait(&agent_chair);
			//get a haircut	
			printf(ANSI_COLOR_YELLOW "Customer sat down in the agent chair." ANSI_COLOR_RESET "\n");
			sem_wait(&waiting_chairs);
			smokers_waiting--;	
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
