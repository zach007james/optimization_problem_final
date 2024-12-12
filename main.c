//
// Wolenski Final Project Submission
// (Ⓒ Zachary R. James)
// Multithreaded implementation with pthreads (C is faster than Python :) )
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "tb_sysinfo.h"

double dot_product(double *v1, double *v2, int size)
{
    double result = 0.0;
    for (int i = 0; i < size; ++i) { result += v1[i] * v2[i]; } 
    return result;
}

// E (expected payoff)
double expected_payoff(double **payoff_matrix, double *strategy_p1, double *strategy_p2, int size)
{
    double *temp = (double *)malloc(size * sizeof(double));
    for (int i = 0; i < size; ++i) { temp[i] = dot_product(payoff_matrix[i], strategy_p1, size); } // sum payoffs
    double result = dot_product(temp, strategy_p2, size); // dot
    free(temp); // free pointer struct 
    return result;
}

// each thread has its own copy of its own data
struct thread_data
{
    int thread_id; 
    double **payoff_matrix;
    double *strategy;
    int size;
    int is_player1;
    int start_iter;
    int end_iter;
};

// optimize strategry method, used by threads
void *optimize_strategy_thread(void *arg)
{ // *optimize_strategy_thread //
    struct thread_data *data = (struct thread_data *)arg; // init thread data struct

    for (int iter = data->start_iter; iter < data->end_iter; ++iter)
    { // f //
        for (int i = 0; i < data->size; ++i)
        { // f //
            double best_payoff = expected_payoff(data->payoff_matrix, data->strategy, data->strategy, data->size);
            double *best_strategy = (double *)malloc(data->size * sizeof(double));

            for (int k = 0; k < data->size; ++k) { best_strategy[k] = data->strategy[k]; }

            for (int j = 0; j < data->size; ++j)
            {
                if (i == j) continue;
                double *new_strategy = (double *)malloc(data->size * sizeof(double));
                for (int k = 0; k < data->size; ++k) { new_strategy[k] = data->strategy[k]; }
                new_strategy[i] += 0.01;
                new_strategy[j] -= 0.01;
                if (new_strategy[j] < 0) { free(new_strategy); continue; }

                double new_payoff = data->is_player1 ?
                    expected_payoff(data->payoff_matrix, new_strategy, data->strategy, data->size) :
                    expected_payoff(data->payoff_matrix, data->strategy, new_strategy, data->size);

                if (data->is_player1 ? new_payoff > best_payoff : new_payoff < best_payoff)
                {
                    best_payoff = new_payoff;
                    free(best_strategy);
                    best_strategy = new_strategy;
                } else { free(new_strategy); }
            }

            for (int k = 0; k < data->size; ++k) { data->strategy[k] = best_strategy[k]; }
            free(best_strategy);
        }
    }
    pthread_exit(NULL); // exit the thead now that the strategy optimization is done
}

int main()
{
    // grabbing system info
    struct sys_info sys_info = print_sys_info();
    int NUM_THREADS = sys_info.threads; 

    // Start timing
    clock_t start_time = clock();

    // filling out the payoff matrix
    //  0  1 -2 
    // -2 -1  3
    //  1 -1  1
    double **payoff_matrix = (double **)malloc(3 * sizeof(double *));
    for (int i = 0; i < 3; ++i) { payoff_matrix[i] = (double *)malloc(3 * sizeof(double)); }
    payoff_matrix[0][0] = 0; payoff_matrix[0][1] = 1; payoff_matrix[0][2] = -2;
    payoff_matrix[1][0] = -2; payoff_matrix[1][1] = -1; payoff_matrix[1][2] = 3;
    payoff_matrix[2][0] = 1; payoff_matrix[2][1] = -1; payoff_matrix[2][2] = 1;

    // allocate memtory for strategies structs
    double *strategy_p1 = (double *)malloc(3 * sizeof(double));
    double *strategy_p2 = (double *)malloc(3 * sizeof(double));

    pthread_t threads[NUM_THREADS];
    struct thread_data thread_data_array[NUM_THREADS];
    int iterations_per_thread = 1000 / NUM_THREADS;

    // Multiple iterations for stability
    int num_iterations = 10; // You can adjust this value
    double avg_value_of_the_game = 0.0;

    for (int iteration = 0; iteration < num_iterations; ++iteration) {
        // init strategies as default the same (1/3)
        for (int i = 0; i < 3; ++i) { strategy_p1[i] = 1.0 / 3; strategy_p2[i] = 1.0 / 3; }

        // make threads for P1 strategy
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            thread_data_array[i].thread_id = i;
            thread_data_array[i].payoff_matrix = payoff_matrix;
            thread_data_array[i].strategy = strategy_p1;
            thread_data_array[i].size = 3;
            thread_data_array[i].is_player1 = 1;
            thread_data_array[i].start_iter = i * iterations_per_thread;
            thread_data_array[i].end_iter = (i == NUM_THREADS - 1) ? 1000 : (i + 1) * iterations_per_thread; 
            pthread_create(&threads[i], NULL, optimize_strategy_thread, (void *)&thread_data_array[i]);
        }
        for (int i = 0; i < NUM_THREADS; ++i) { pthread_join(threads[i], NULL); } // join threads

        // make threads for P2 strategy 
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            thread_data_array[i].thread_id = i;
            thread_data_array[i].payoff_matrix = payoff_matrix;
            thread_data_array[i].strategy = strategy_p2; // Use strategy_p2
            thread_data_array[i].size = 3;
            thread_data_array[i].is_player1 = 0; // Set is_player1 to 0 for P2
            thread_data_array[i].start_iter = i * iterations_per_thread;
            thread_data_array[i].end_iter = (i == NUM_THREADS - 1) ? 1000 : (i + 1) * iterations_per_thread; 
            pthread_create(&threads[i], NULL, optimize_strategy_thread, (void *)&thread_data_array[i]);
        }
        for (int i = 0; i < NUM_THREADS; ++i) { pthread_join(threads[i], NULL); } // join p2 threads

        // call methods to get the result
        avg_value_of_the_game += expected_payoff(payoff_matrix, strategy_p1, strategy_p2, 3);
    }

    avg_value_of_the_game /= num_iterations;

    // End timing
    clock_t end_time = clock();
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // printing results:
    printf("P1's optimal strategy: %f %f %f\n", strategy_p1[0], strategy_p1[1], strategy_p1[2]);
    printf("P2's optimal strategy: %f %f %f\n", strategy_p2[0], strategy_p1[1], strategy_p2[2]);
    printf("Average Value of the game: %f\n", avg_value_of_the_game);

    printf("Runtime: %f seconds\n", cpu_time_used);
    printf("Cores: %d, Threads: %d\n", sys_info.cores, sys_info.threads);

    // free strucures (good memory management practice)
    for (int i = 0; i < 3; ++i) { free(payoff_matrix[i]); } // free allocated memory
    free(payoff_matrix); free(strategy_p1); free(strategy_p2);

    return 0; 
}