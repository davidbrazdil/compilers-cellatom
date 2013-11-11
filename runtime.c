#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 2

// Prototype.  The real function will be inserted by the JIT.
int16_t cell(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height, int16_t x, int16_t y, int16_t v, int16_t *g);

struct thread_data {
	int thread_id;
	pthread_t thread_ref;

	int16_t *oldgrid, *newgrid;
	int16_t width, height;
	int16_t x_start, x_end;

	sem_t cond_ready, cond_done;
};

void *automaton_thread(void *threadarg) {
	struct thread_data *data_this = (struct thread_data *) threadarg;

//	printf("Thread #%d: %d - %d\n", data_this->thread_id, data_this->x_start, data_this->x_end - 1);

	while (1) {

		sem_wait(&data_this->cond_ready);
		printf("Running thread #%d: %d - %d\n", data_this->thread_id, data_this->x_start, data_this->x_end - 1);

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

		sem_post(&data_this->cond_done);

	}
}


int THREADS_INITIALIZED = 0;
struct thread_data THREADS[NUM_THREADS];

void automaton(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height) {
	// reuse threads!
	// consider global/local registers (lock)
	// better distribute columns

	if (!THREADS_INITIALIZED) {

		int x_start = 0;
		int x_step = width / NUM_THREADS;
		if (x_step <= 0) x_step = 1;

		int res;

		int t;
		for (t = 0; x_start < width; t++) {
			THREADS[t].thread_id = t;

			THREADS[t].width = width;
			THREADS[t].height = height;

			sem_init(&THREADS[t].cond_ready, 0, 0);
			sem_init(&THREADS[t].cond_ready, 0, 0);

			THREADS[t].x_start = x_start;
			if (t == NUM_THREADS - 1)  THREADS[t].x_end = width;
			else                       THREADS[t].x_end = x_start + x_step;
			x_start = THREADS[t].x_end;

			res = pthread_create(&THREADS[t].thread_ref, NULL, automaton_thread, (void*) &THREADS[t]);
			if (res) {
				printf("ERROR; return code from pthread_create() is %d\n", res);
				exit(-1);
			}
		}

		THREADS_INITIALIZED = t;
	}

	int t = THREADS_INITIALIZED;
	while (t > 0) {
		t--;
		THREADS[t].oldgrid = oldgrid;
		THREADS[t].newgrid = newgrid;

 		printf("Signalling thread #%d\n", THREADS[t].thread_id);
		sem_post(&THREADS[t].cond_ready);
	}

	t = THREADS_INITIALIZED;
	while (t > 0) {
		t--;
		sem_wait(&THREADS[t].cond_done);
// 		printf("Joined thread #%d\n", data[t].thread_id);
	}
}
