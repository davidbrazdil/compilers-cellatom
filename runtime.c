#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4

// Prototype.  The real function will be inserted by the JIT.
int16_t cell(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height, int16_t x, int16_t y, int16_t v, int16_t *g);


struct thread_data {
	int thread_id;

	int16_t *oldgrid, *newgrid;
	int16_t width, height;
	int16_t x_start, x_end;
};

void *automaton_thread(void *threadarg) {
	struct thread_data *data_this = (struct thread_data *) threadarg;

	printf("Thread #%d: %d - %d\n", data_this->thread_id, data_this->x_start, data_this->x_end - 1);

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
}


void automaton(int16_t *oldgrid, int16_t *newgrid, int16_t width, int16_t height) {
	// reuse threads!
	// consider global/local registers (lock)
	// better distribute columns

	int x_start = 0;
	int x_step = width / NUM_THREADS;
	if (x_step <= 0) x_step = 1;

	pthread_t threads[NUM_THREADS];
	struct thread_data data[NUM_THREADS];
	int res;

	int t;
	for (t = 0; x_start < width; t++) {
		data[t].thread_id = t;

		data[t].oldgrid = oldgrid;
		data[t].newgrid = newgrid;
		data[t].width = width;
		data[t].height = height;

		data[t].x_start = x_start;
		if (t == NUM_THREADS - 1)  data[t].x_end = width;
		else                       data[t].x_end = x_start + x_step;
		x_start = data[t].x_end;

		res = pthread_create(&threads[t], NULL, automaton_thread, (void*) &data[t]);
		if (res) {
			printf("ERROR; return code from pthread_create() is %d\n", res);
			exit(-1);
		}
	}

	while (t > 0) {
		t--;
		pthread_join(threads[t], NULL);
		printf("Joined thread #%d\n", data[t].thread_id);
	}
}
