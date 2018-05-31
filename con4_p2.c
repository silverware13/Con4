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
node* list_head = NULL;
int switch_num = 0;
int list_length = 0;

//function prototype(s)
void spawn_threads(int insert, int search, int delete);
void* search_thread();
void* insert_thread();
void* delete_thread();
int random_range(int min_val, int max_val);

//create mutex lock(s)
pthread_mutex_t insert_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t search_switch = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t delete_lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	//first make sure the user entered numbers for
	//insert, search and delete threads
	if(argc < 4){
		printf("USEAGE: %s [number of inserter threads] [number of searcher threads] [number of deleter threads]\n", argv[0]);
		return 0;
	}		

	//make sure the user has entered digits
	if(!isdigit(*argv[1]) || !isdigit(*argv[2]) || !isdigit(*argv[3])){
		printf("Please enter arguments as unsigned integers.\n");
		return 0;
	}

	//convert arguments to int
	int insert, search, delete;
	insert = strtol(argv[1], NULL, 10);	
	search = strtol(argv[2], NULL, 10);
	delete = strtol(argv[3], NULL, 10);

	//seed random number generation
	init_genrand(time(NULL));
	srand(time(NULL));	

	//malloc list head
	list_head = malloc(sizeof(list_head));
	
	//create threads and wait for their completion
	spawn_threads(insert, search, delete);
	
	//destroy mutex lock(s)	
	pthread_mutex_destroy(&insert_lock);
	pthread_mutex_destroy(&search_switch);
	pthread_mutex_destroy(&delete_lock);
	
	return 0;
}

/* Function: spawn_threads
 * -----------------------
 * Spawns inserter, searcher, and deleter threads, then waits for threads to
 * finish execution and join. Since these threads will run forever, we expect
 * to block here indefinitely.
 */
void spawn_threads(int insert, int search, int delete)
{
	pthread_t thrd;

	//if there are no threads to make we just return	
	if(!insert && !search && !delete)
		return;

	printf("\nCreating %d inserter, %d searcher, and %d deleter threads.\n\n", insert, search, delete);
	printf("NOTE:\n>>[THREAD] Represents a thread starting to access the list.\n<<[THREAD] Represents a thread leaving the list.\n\n");
	sleep(4);
	
	while(insert || search || delete){
		if(insert){
			pthread_create(&thrd, NULL, insert_thread, NULL);
			insert--;
		}

		if(search){
			pthread_create(&thrd, NULL, search_thread, NULL);
			search--;
		}
		
		if(delete){
			pthread_create(&thrd, NULL, delete_thread, NULL);
			delete--;
		}
	}

	//join thread (this should never finish)
	pthread_join(thrd, NULL);
}

/* Function: search_thread
 * -------------------------
 * This function is called by a new search thread when it is created.
 *
 * It accesses the list and prints out the contents. It may access the list
 * at the same time as other searchers as well as at the same time as an insert thread.
 * First it actives the switch saying that there are searchers accesssing the list.
 * Then if there is not a deleter accessing the list it prints the contents.
 * When the last searcher leaves the list it turns off the searcher switch. 
 */
void* search_thread()
{
	while(true){
		//turn on search switch and confirm that delete is not running.
		int lock_value;
		pthread_mutex_trylock(&search_switch);
		lock_value = pthread_mutex_trylock(&delete_lock);
		if(lock_value){
			sleep(1);
			continue;	
		} else {
			pthread_mutex_unlock(&delete_lock);
			switch_num++;
		}
		printf(">>[SEARCH]\n");

		//print the contents of the list.	
		char buffer[250000] = {0};
		int first_number = 1;
		node* current = list_head;
		sprintf(buffer, "[SEARCH] Here are the contents of the list: ");
		while(current->next != NULL){
			current = current->next;
			if(first_number){
				sprintf(buffer + strlen(buffer), "%d", current->val);
				first_number = 0;
			} else {
				sprintf(buffer + strlen(buffer), ", %d", current->val);
			}	
		}
		printf("%s.\n", buffer);

		//turn off switch if this is the last search thread to exit.
		printf("<<[SEARCH]\n");
		switch_num--;
		if(switch_num < 1){
			pthread_mutex_unlock(&search_switch);
		}	
		sleep(1);
	}
}

/* Function: insert_thread
 * -------------------------
 * This function is called by a new insert thread when it is created.
 *
 * Inserters may not access the list at the same time as another
 * inserter or deleter. They add a random number to the end of the list.
 * They start by locking the insert mutex. When they leave the list the unlock it.
 *
 */
void* insert_thread()
{
	while(true){
		//get the insert lock
		pthread_mutex_lock(&insert_lock);
		printf(">>[INSERT]\n");
		
		//insert a new value at the end of the list
		int insert_val = random_range(1, 99);
		node* current = list_head;
		while(current->next != NULL){
			current = current->next;	
		}
		current->next = malloc(sizeof(node));
		current->next->val = insert_val;
		current->next->next = NULL;
		list_length++;
		printf("[INSERT] Added %d to end of list.\n", insert_val);

		//unlock the insert lock
		printf("<<[INSERT]\n");
		pthread_mutex_unlock(&insert_lock);		
		sleep(1);
	}
}

/* Function: delete_thread
 * -------------------------
 * This function is called by a new delete thread when it is created.
 *
 * The thread grabs the delete and insert locks. Then it confirms that
 * no search threads are running. If they are, it releases the locks
 * and restarts. If there are no threads running it then deletes a
 * random node from the linked list before unlocking the locks. 
 */
void* delete_thread()
{
	while(true){
		//grab locks and confirm that no search thread(s) running.
		int lock_value;
		pthread_mutex_lock(&delete_lock);
		pthread_mutex_lock(&insert_lock);
		lock_value = pthread_mutex_trylock(&search_switch);
		if(lock_value){
			pthread_mutex_unlock(&delete_lock);	
			pthread_mutex_unlock(&insert_lock);	
			sleep(1);
			continue;	
		} else {
			pthread_mutex_unlock(&search_switch);
		}	
	
		//delete a random node from the linked list
		int remove_node = random_range(1, list_length);	
		int current_node = 1;
		printf(">>[DELETE]\n");
		node* previous = list_head;
		node* current = list_head->next;
		if(current == NULL){
			printf("[DELETE] Nothing to delete.\n");
			printf("<<[DELETE]\n");
			pthread_mutex_unlock(&delete_lock);	
			pthread_mutex_unlock(&insert_lock);	
			sleep(1);
			continue;	
		}

		while(current->next != NULL){
			if(remove_node == current_node){
				break;
			}
			previous = current;	
			current = current->next;	
			current_node++;
		}
		printf("[DELETE] Removed %d from the list.\n", current->val);
		previous->next = current->next;
		free(current);
		list_length--;
	
		//unlock the locks	
		printf("<<[DELETE]\n");
		pthread_mutex_unlock(&delete_lock);	
		pthread_mutex_unlock(&insert_lock);	
		sleep(1);
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
