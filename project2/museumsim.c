#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

#include "museumsim.h"


//
// In all of the definitions below, some code has been provided as an example
// to get you started, but you do not have to use it. You may change anything
// in this file except the function signatures.
//


struct shared_data {
	// Add any relevant synchronization constructs and shared state here.
	// For example:
	//     pthread_mutex_t ticket_mutex;
	//     int tickets;
	
	pthread_mutex_t lock;
	// pthread_mutex_t guide_mutex;

	int numIn;
	
	
	int tickets;
	int inside_visitor;
	int inside_guide;
	int visitors_waiting;


	sem_t visitors_arrive;
	sem_t guide_enter;

	sem_t visitor_limit;

	sem_t guide_admits;

	sem_t visitor_touring;

	sem_t guide_limit;

	sem_t complete_work;
	sem_t clear_waiting;

	sem_t available_visitors;

	sem_t visitor_waiting2;
	// sem_t visitor_waiting;

	// sem_t available_ticket;
	// sem_t available_guide;
	// sem_t is_visitor_leave;
	// sem_t visitor_limit_per_guide;
	// //for inside museum
	// sem_t guide_limit;
	// sem_t visitor_limit;


	
};

static struct shared_data shared;


/**
 * Set up the shared variables for your implementation.
 * 
 * `museum_init` will be called before any threads of the simulation
 * are spawned.
 */
void museum_init(int num_guides, int num_visitors)
{
	
	
	pthread_mutex_init(&shared.lock,NULL);

	
	
	
	shared.tickets = MIN(VISITORS_PER_GUIDE * num_guides, num_visitors);
	shared.inside_visitor=0;
	shared.inside_guide=0;
	shared.numIn=0;

	shared.visitors_waiting=MIN(VISITORS_PER_GUIDE * num_guides, num_visitors);
	
	

	sem_init(&shared.visitors_arrive,0,0);
	
	sem_init(&shared.guide_enter,0,0);
	
	sem_init(&shared.visitor_limit,0,VISITORS_PER_GUIDE*GUIDES_ALLOWED_INSIDE);
	
	

	
	sem_init(&shared.complete_work,0,0);
	
	sem_init(&shared.guide_limit,0,GUIDES_ALLOWED_INSIDE);
	
	
	sem_init(&shared.visitor_touring,0,0);

	sem_init(&shared.guide_admits,0,0);

	sem_init(&shared.clear_waiting,0,0);
	sem_init(&shared.available_visitors,0,0);

	sem_init(&shared.visitor_waiting2,0,0);
	
	
	



	
}


/**
 * Tear down the shared variables for your implementation.
 * 
 * `museum_destroy` will be called after all threads of the simulation
 * are done executing.
 */
void museum_destroy()
{

	pthread_mutex_destroy(&shared.lock);
	

	
	sem_destroy(&shared.visitors_arrive);
	sem_destroy(&shared.guide_enter);
	sem_destroy(&shared.visitor_limit);
	

	sem_destroy(&shared.guide_limit);
	sem_destroy(&shared.complete_work);
	sem_destroy(&shared.visitor_touring);

	sem_destroy(&shared.guide_admits);
	sem_destroy(&shared.clear_waiting);
	sem_destroy(&shared.available_visitors);

	sem_destroy(&shared.visitor_waiting2);


	
	


}


/**
 * Implements the visitor arrival, touring, and leaving sequence.
 */
void visitor(int id)
{
	/*check if there is available tickets, if not just leave and return*/

	
	pthread_mutex_lock(&shared.lock);
	{
		if(shared.tickets>shared.numIn){
			shared.numIn++;
			visitor_arrives(id);
			pthread_mutex_unlock(&shared.lock);

			sem_post(&shared.visitors_arrive);
			sem_wait(&shared.visitor_limit);
			

			sem_wait(&shared.guide_admits);
			
			pthread_mutex_lock(&shared.lock);
			{
				
				shared.inside_visitor++;
			
				//printf("Visitor side number of visitors_waiting %d \n",shared.visitors_waiting);
				
			}
			pthread_mutex_unlock(&shared.lock);

			visitor_tours(id);

				
				
				visitor_leaves(id);

				pthread_mutex_lock(&shared.lock);
				{
		
					shared.inside_visitor--;
					
					if(shared.inside_visitor==0){
			
					sem_post(&shared.clear_waiting);
					
					}

		
		

				}
				pthread_mutex_unlock(&shared.lock);
			

				

				sem_post(&shared.visitor_limit);


			




		}
		else{
			pthread_mutex_unlock(&shared.lock);
			
			visitor_leaves(id);
			}

	}


	

	
}

/**
 * Implements the guide arrival, entering, admitting, and leaving sequence.
 */
void guide(int id)
{
	guide_arrives(id);

	sem_wait(&shared.guide_limit);
	
	guide_enters(id);

	pthread_mutex_lock(&shared.lock);
	{
		shared.inside_guide++;
	}
	pthread_mutex_unlock(&shared.lock);


	for(int i=0;i<10;i++){


		pthread_mutex_lock(&shared.lock);
		{
			if(shared.visitors_waiting==0){
				shared.inside_guide--;
				// if(shared.inside_guide==0){

				// }
				pthread_mutex_unlock(&shared.lock);
				break;
			}
		printf("number of visitors_waiting %d \n",shared.visitors_waiting);
		printf("number of inside visitor %d \n",shared.inside_visitor);

		}pthread_mutex_unlock(&shared.lock);
		

		
		
		sem_wait(&shared.visitors_arrive);

			guide_admits(id);
		
		pthread_mutex_lock(&shared.lock);
		{
		
			shared.visitors_waiting--;

		}pthread_mutex_unlock(&shared.lock);
		
		
		
		sem_post(&shared.guide_admits);

			

	}


	sem_wait(&shared.clear_waiting);
	guide_leaves(id);
	sem_post(&shared.guide_limit);



	

}
