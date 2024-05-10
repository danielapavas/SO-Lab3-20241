/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation using 2 threads
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Daniela Pavas C.C. 1192741700
 * @author     Giovani Cardona C.C. 1035913434
 * @date       2024
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

// Estructura para pasar argumentos a los hilos
struct thread_args {
    int start_index;
    int end_index;
	int max_iters;
    double *X;
    double *Y;
	double *Y_avg;
    double a;
};

// Función que realiza la operación SAXPY en un fragmento de los vectores X e Y
void *saxpy_thread(void *args) {
    struct thread_args *t_args = (struct thread_args *)args;
	int counter;

	for (int it = 0; it < t_args->max_iters; it++) {
		counter = 0;
		
		for (int i = t_args->start_index; i < t_args->end_index; i++) {
			counter++;
			t_args->Y[i] = t_args->Y[i] + t_args->a * t_args->X[i];
			t_args->Y_avg[it] += t_args->Y[i];
		}
		t_args->Y_avg[it] = t_args->Y_avg[it] / counter;
	}

    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	unsigned int seed = 1;
  	int p = 10000000;
  	int n_threads = 2;
  	int max_iters = 1000;
  	// Variables to perform SAXPY operation
	double* X;
	double a;
	double* Y;
	double* Y_avgs;
	int i;
	// Variables to get execution time
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
		switch(opt){  
			case 'p':  
			printf("vector size: %s\n", optarg);
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt);  
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-i <maximum itertions>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}  
	}  
	srand(seed);

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	

	// initializing data
	X = (double*) malloc(sizeof(double) * p);
	Y = (double*) malloc(sizeof(double) * p);
	Y_avgs = (double*) malloc(sizeof(double) * max_iters);

	// Variables para manejo de hilos
    pthread_t threads[n_threads];
    struct thread_args t_args[n_threads];
    int chunk_size = p / n_threads;

	for(i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	for(i = 0; i < max_iters; i++){
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

#ifdef DEBUG
	printf("vector X= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ",X[i]);
	}
	printf("%f ]\n",X[p-1]);

	printf("vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("a= %f \n", a);	
#endif

	/*
	 *	Function to parallelize 
	 */
	gettimeofday(&t_start, NULL);

	// Crear y ejecutar los hilos
	int thread = 0;

	t_args[thread].start_index = thread * chunk_size;
	t_args[thread].end_index = (thread == n_threads - 1) ? p : (thread + 1) * chunk_size;
	t_args[thread].max_iters = max_iters;
	t_args[thread].X = X;
	t_args[thread].Y = Y;
	t_args[thread].Y_avg = (double*) malloc(sizeof(double) * max_iters);
	t_args[thread].a = a;

	pthread_create(&threads[thread], NULL, saxpy_thread, (void *)&t_args[thread]);

	thread = 1;

	t_args[thread].start_index = thread * chunk_size;
	t_args[thread].end_index = (thread == n_threads - 1) ? p : (thread + 1) * chunk_size;
	t_args[thread].max_iters = max_iters;
	t_args[thread].X = X;
	t_args[thread].Y = Y;
	t_args[thread].Y_avg = (double*) malloc(sizeof(double) * max_iters);
	t_args[thread].a = a;

	pthread_create(&threads[thread], NULL, saxpy_thread, (void *)&t_args[thread]);

    // Esperar a que todos los hilos terminen
    pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);
    
	for (int it = 0; it < t_args->max_iters; it++) {
		Y_avgs[it] = (t_args[0].Y_avg[it] + t_args[1].Y_avg[it]) / n_threads;
	}

	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("RES: final vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("RES: final vector Y_avgs= [ ");
	for(i = 0; i < max_iters-1; i++){
		printf("%f, ", Y_avgs[i]);
	}
	printf("%f ]\n", Y_avgs[max_iters-1]);
#endif
	
	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);
	return 0;
}	