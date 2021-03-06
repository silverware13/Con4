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
 * -------------------------------------
 * Cite refrence:
 * Little book of semaphores. Chapter 4.5
 */

#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
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
bool hand_tobacco = 0;
bool hand_paper = 0;
bool hand_matches = 0;

//function prototype(s)
void spawn_threads();
void* smoker_thread_matches();
void* smoker_thread_tobacco();
void* smoker_thread_paper();
void* agent_thread_A();
void* agent_thread_B();
void* agent_thread_C();
void* pusher_thread_A();
void* pusher_thread_B();
void* pusher_thread_C();

//create semaphore(s)
sem_t agent_sem;
sem_t pusher_sem;
sem_t paper;
sem_t tobacco;
sem_t matches;
sem_t mutex_paper;
sem_t mutex_tobacco;
sem_t mutex_matches;

int main(int argc, char **argv)
{
	//initalize semaphore(s)
	sem_init(&agent_sem, 0, 1);
	sem_init(&pusher_sem, 0, 1);
	sem_init(&paper, 0, 0);
	sem_init(&tobacco, 0, 0);
	sem_init(&matches, 0, 0);
	sem_init(&mutex_paper, 0, 0);
	sem_init(&mutex_tobacco, 0, 0);
	sem_init(&mutex_matches, 0, 0);
	
	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	
	
	//create threads and wait for their completion
	spawn_threads();
	
	//destroy semaphore(s)	
	sem_destroy(&agent_sem);	
	sem_destroy(&pusher_sem);	
	sem_destroy(&paper);	
	sem_destroy(&tobacco);	
	sem_destroy(&matches);	
	sem_destroy(&mutex_paper);	
	sem_destroy(&mutex_tobacco);	
	sem_destroy(&mutex_matches);	
	
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

	printf(ANSI_COLOR_MAGENTA "\nThere is 1 agent and 3 smokers." ANSI_COLOR_RESET "\n\n");
	
	//we create three smokers
	pthread_create(&thrd, NULL, smoker_thread_matches, NULL);	
	pthread_create(&thrd, NULL, smoker_thread_tobacco, NULL);	
	pthread_create(&thrd, NULL, smoker_thread_paper, NULL);	
	
	//we create three pushers	
	pthread_create(&thrd, NULL, pusher_thread_A, NULL);	
	pthread_create(&thrd, NULL, pusher_thread_B, NULL);	
	pthread_create(&thrd, NULL, pusher_thread_C, NULL);	

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
 *
 * This thread acts as the agent handing out tobacco and paper.
 */
void* agent_thread_A()
{
	while(true){
		sem_wait(&agent_sem);
		printf(ANSI_COLOR_CYAN "The agent holds out tobacco and paper." 
			ANSI_COLOR_RESET "\n");
		sem_post(&tobacco);
		sem_post(&paper);
	}
}

/* Function: agent_thread_B
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This thread acts as the agent handing out matches and paper.
 */
void* agent_thread_B()
{
	while(true){
		sem_wait(&agent_sem);
		printf(ANSI_COLOR_CYAN "The agent holds out matches and paper." 
			ANSI_COLOR_RESET "\n");
		sem_post(&matches);
		sem_post(&paper);
	}
}

/* Function: agent_thread_C
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This thread acts as the agent handing out matches and tobacco.
 */
void* agent_thread_C()
{
	while(true){
		sem_wait(&agent_sem);
		printf(ANSI_COLOR_CYAN "The agent holds out matches and tobacco." 
			ANSI_COLOR_RESET "\n");
		sem_post(&matches);
		sem_post(&tobacco);
	}
}

/* Function: smoker_thread_matches
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This smoker has matches. 
 */
void* smoker_thread_matches()
{
	while(true){
		sem_wait(&mutex_matches);
		printf(ANSI_COLOR_YELLOW "The smoker with matches takes the tobacco and paper." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker makes a cigarette." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker smokes the cigarette." 
			ANSI_COLOR_RESET "\n");
		sleep(3);
		sem_post(&agent_sem);
		sleep(3);
	}
}

/* Function: smoker_thread_tobacco
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This smoker has tobacco. 
 */
void* smoker_thread_tobacco()
{
	while(true){
		sem_wait(&mutex_tobacco);
		printf(ANSI_COLOR_YELLOW "The smoker with tobacco takes the matches and paper." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker makes a cigarette." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker smokes the cigarette." 
			ANSI_COLOR_RESET "\n");
		sleep(3);
		sem_post(&agent_sem);
		sleep(3);
	}
}

/* Function: smoker_thread_paper
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This smoker has paper. 
 */
void* smoker_thread_paper()
{
	while(true){
		sem_wait(&mutex_paper);
		printf(ANSI_COLOR_YELLOW "The smoker with paper takes the matches and tobacco." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker makes a cigarette." 
			ANSI_COLOR_RESET "\n");
		printf(ANSI_COLOR_YELLOW "The smoker smokes the cigarette." 
			ANSI_COLOR_RESET "\n");
		sleep(3);
		sem_post(&agent_sem);
		sleep(3);
	}
}

/* Function: pusher_thread_A
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This pusher finds a smoker without tobacco. 
 */
void* pusher_thread_A()
{
	while(true){
		sem_wait(&tobacco);
		sem_wait(&pusher_sem);
		if(hand_paper){
			hand_paper = 0;
			sem_post(&mutex_matches);
		} else if(hand_matches){
			hand_matches = 0;
			sem_post(&mutex_paper);
		} else {
			hand_tobacco = 1;
		}
		sem_post(&pusher_sem);
	}
}

/* Function: pusher_thread_B
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This pusher finds a smoker without matches. 
 */
void* pusher_thread_B()
{
	while(true){
		sem_wait(&matches);
		sem_wait(&pusher_sem);
		if(hand_paper){
			hand_paper = 0;
			sem_post(&mutex_tobacco);
		} else if(hand_tobacco){
			hand_tobacco = 0;
			sem_post(&mutex_paper);
		} else {
			hand_matches = 1;
		}
		sem_post(&pusher_sem);
	}
}

/* Function: pusher_thread_C
 * -------------------------
 * This function is called by a new resource thread when it is created.
 *
 * This pusher finds a smoker without paper. 
 */
void* pusher_thread_C()
{
	while(true){
		sem_wait(&paper);
		sem_wait(&pusher_sem);
		if(hand_matches){
			hand_matches = 0;
			sem_post(&mutex_tobacco);
		} else if(hand_tobacco){
			hand_tobacco = 0;
			sem_post(&mutex_matches);
		} else {
			hand_paper = 1;
		}
		sem_post(&pusher_sem);
	}
}
