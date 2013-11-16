#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <dispatch/dispatch.h>

#define NUM_THREADS 2

// Prototype.  The real function will be inserted by the JIT.
int16_t cell(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height, int16_t x, int16_t y, int16_t v, int16_t *g);



struct thread_data {
	int thread_id;
	dispatch_queue_t queue_ref;

	int16_t *oldgrid, *newgrid;
	int16_t width, height;
	int16_t x_start, x_end;
};

int THREADS_INITIALIZED = 0;
struct thread_data THREADS[NUM_THREADS];
dispatch_semaphore_t SEMAPHORE;


void *automaton_thread(void *threadarg) {
	struct thread_data *data_this = (struct thread_data *) threadarg;

#ifdef DEBUG
	printf("Running thread #%d: %d - %d\n", data_this->thread_id, data_this->x_start, data_this->x_end - 1);
#endif

	int16_t g[10] = {0};
	int16_t i = data_this->x_start * data_this->height;
	for (int16_t x = data_this->x_start; x < data_this->x_end ; x++) {
		for (int16_t y = 0; y < data_this->height ; y++,i++) {
			data_this->newgrid[i] = cell(
				data_this->oldgrid,
				data_this->newgrid,
				data_this->width,
				data_this->height,
				x, y,
				data_this->oldgrid[i],
				g);
		}
	}

#ifdef DEBUG
	printf("Finished thread #%d: %d - %d\n", data_this->thread_id, data_this->x_start, data_this->x_end - 1);
#endif
	dispatch_semaphore_signal(SEMAPHORE);

	return 0;
}


void automaton(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height) {
	// consider global/local registers (lock)
	// better distribute columns

	if (!THREADS_INITIALIZED) {

		// split the grid into a resonable number of chunks
		int x_start = 0;
		int x_step = width / NUM_THREADS;
		if (x_step <= 0) x_step = 1;

		int res;

		int t;
		for (t = 0; x_start < width; t++) {
			THREADS[t].thread_id = t;

			THREADS[t].width = width;
			THREADS[t].height = height;

			THREADS[t].x_start = x_start;
			if (t == NUM_THREADS - 1)  THREADS[t].x_end = width;
			else                       THREADS[t].x_end = x_start + x_step;
			x_start = THREADS[t].x_end;

			// create new libdispatch queue
			THREADS[t].queue_ref = dispatch_queue_create(NULL, NULL);
		}

		// create semaphore that signals end of iteration
		SEMAPHORE = dispatch_semaphore_create(t);

		THREADS_INITIALIZED = t;
	}

	// schedule the threads

	int t = THREADS_INITIALIZED;
	while (t > 0) {
		t--;
		THREADS[t].oldgrid = oldgrid;
		THREADS[t].newgrid = newgrid;

#ifdef DEBUG
 		printf("Scheduling thread #%d\n", THREADS[t].thread_id);
#endif
		dispatch_async(THREADS[t].queue_ref, ^{ automaton_thread(&THREADS[t]); });
		dispatch_semaphore_wait(SEMAPHORE, DISPATCH_TIME_FOREVER);
	}

	// wait for the threads to finish

	dispatch_semaphore_wait(SEMAPHORE, DISPATCH_TIME_FOREVER);
	dispatch_semaphore_signal(SEMAPHORE);

#ifdef DEBUG
	printf("Finished iteration\n");
#endif
}
