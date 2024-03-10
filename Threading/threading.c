#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

struct thread_data {
    pthread_mutex_t *mutex;
    int wait_to_obtain_ms;
    int wait_to_release_ms;
    bool thread_complete_success;
};

void *thread_function(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;

    // Sleep for wait_to_obtain_ms milliseconds
    usleep(data->wait_to_obtain_ms * 1000);

    // Obtain the mutex
    pthread_mutex_lock(data->mutex);

    // Sleep for wait_to_release_ms milliseconds
    usleep(data->wait_to_release_ms * 1000);

    // Release the mutex
    pthread_mutex_unlock(data->mutex);

    // Set thread completion status to true
    data->thread_complete_success = true;

    // Return the thread_data structure
    return arg;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms) {
    // Allocate memory for thread_data structure
    struct thread_data *data = malloc(sizeof(struct thread_data));
    if (data == NULL) {
        // Memory allocation failed
        return false;
    }

    // Fill in the thread_data structure
    data->mutex = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    data->thread_complete_success = false;

    // Create the thread
    int result = pthread_create(thread, NULL, thread_function, (void *)data);
    if (result != 0) {
        // pthread_create failed
        free(data); // Free allocated memory
        return false;
    }

    return true;
}

int main() {
    pthread_t thread;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int wait_to_obtain_ms = 1000; // 1 second
    int wait_to_release_ms = 2000; // 2 seconds

    if (start_thread_obtaining_mutex(&thread, &mutex, wait_to_obtain_ms, wait_to_release_ms)) {
        printf("Thread started successfully.\n");

        // You can continue with other operations here without blocking for the thread to complete
    } else {
        printf("Failed to start thread.\n");
    }

    // You can join the thread and free memory after your operations are done
    // pthread_join(thread, NULL);
    // pthread_mutex_destroy(&mutex);

    return 0;
}
