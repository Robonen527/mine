#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// Bounded Buffer
/*
cd /mnt/c/Users/Roy/Desktop/CS/OperSys/ex3
gcc ex3.c -pthread
*/ 

typedef struct {
    int size;
    char** buffer;
    int start;
    int end;
    sem_t mutex;
    sem_t empty;
    sem_t full;
} Bounded_Buffer;

void Bounded_Buffer_init(Bounded_Buffer* bb, int size) {
    bb->size = size;
    bb->buffer = (char**)malloc(size * sizeof(char*));
    bb->start = 0;
    bb->end = 0;
    sem_init(&bb->mutex, 0, 1);
    sem_init(&bb->empty, 0, size);
    sem_init(&bb->full, 0, 0);
}

void Bounded_Buffer_insert(Bounded_Buffer* bb, char* s) {
    sem_wait(&bb->empty);
    sem_wait(&bb->mutex);

    bb->buffer[bb->end] = strdup(s);
    bb->end = (bb->end + 1) % bb->size;

    sem_post(&bb->mutex);
    sem_post(&bb->full);
}

char* Bounded_Buffer_remove(Bounded_Buffer* bb) {
    sem_wait(&bb->full);
    sem_wait(&bb->mutex);

    char* s = bb->buffer[bb->start];
    bb->start = (bb->start + 1) % bb->size;

    sem_post(&bb->mutex);
    sem_post(&bb->empty);

    return s;
}

// Producer

typedef struct {
    int id;
    char type[10];
    int num_products;
    Bounded_Buffer* producer_queue;
} Producer;

void* producer_thread(void* arg) {
    Producer* producer = (Producer*)arg;
    for (int i = 0; i < producer->num_products; i++) {
        char product[50];
        sprintf(product, "producer %d %s %d", producer->id, producer->type, i);
        Bounded_Buffer_insert(producer->producer_queue, product);
    }
    Bounded_Buffer_insert(producer->producer_queue, "DONE");
    pthread_exit(NULL);
}

// Dispatcher

typedef struct {
    Bounded_Buffer* producer_queues[3];
    Bounded_Buffer* dispatcher_queues[3];
} Dispatcher;

void* dispatcher_thread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    int producer_idx = 0;

    while (1) {
        Bounded_Buffer* producer_queue = dispatcher->producer_queues[producer_idx];
        char* message = Bounded_Buffer_remove(producer_queue);

        if (strcmp(message, "DONE") == 0) {
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[0], "DONE");
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[1], "DONE");
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[2], "DONE");
            producer_idx = (producer_idx + 1) % 3;
            continue;
        }

        if (strcmp(message, "SPORTS") == 0) {
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[0], message);
        } else if (strcmp(message, "NEWS") == 0) {
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[1], message);
        } else if (strcmp(message, "WEATHER") == 0) {
            Bounded_Buffer_insert(dispatcher->dispatcher_queues[2], message);
        }

        producer_idx = (producer_idx + 1) % 3;
    }
}

// Co-Editor

typedef struct {
    char type[10];
    Bounded_Buffer* dispatcher_queue;
    Bounded_Buffer* coeditor_queue;
} Co_Editor;

void* coeditor_thread(void* arg) {
    Co_Editor* coeditor = (Co_Editor*)arg;
    while (1) {
        char* message = Bounded_Buffer_remove(coeditor->dispatcher_queue);

        if (strcmp(message, "DONE") == 0) {
            Bounded_Buffer_insert(coeditor->coeditor_queue, "DONE");
            break;
        }

        // Simulate editing process
        usleep(100000);  // 0.1 seconds

        Bounded_Buffer_insert(coeditor->coeditor_queue, message);
    }
    pthread_exit(NULL);
}

// Screen Manager

typedef struct {
    Bounded_Buffer* coeditor_queue;
    int done_count;
} Screen_Manager;

void* screen_manager_thread(void* arg) {
    Screen_Manager* screen_manager = (Screen_Manager*)arg;
    while (1) {
        char* message = Bounded_Buffer_remove(screen_manager->coeditor_queue);

        if (strcmp(message, "DONE") == 0) {
            screen_manager->done_count++;
            if (screen_manager->done_count == 3) {
                printf("DONE\n");
                break;
            }
        } else {
            printf("%s\n", message);
        }
    }
    pthread_exit(NULL);
}

// Main function

int main() {
    printf("a\n");
    FILE* config_file = fopen("config.txt", "r");
    if (config_file == NULL) {
        printf("Error opening config file.\n");
        return 1;
    }

    // Read configuration file
    int num_producers;
    int producer_num_products[3];
    int producer_queue_sizes[3];
    int coeditor_queue_size;
    fscanf(config_file, "%d", &num_producers);
    for (int i = 0; i < num_producers; i++) {
        fscanf(config_file, "%d", &producer_num_products[i]);
        fscanf(config_file, "%*s %*s = %d", &producer_queue_sizes[i]);
    }
    fscanf(config_file, "%*s = %d", &coeditor_queue_size);
    fclose(config_file);

    // Create and initialize the bounded buffer for producer queues
    Bounded_Buffer producer_queues[3];
    for (int i = 0; i < num_producers; i++) {
        Bounded_Buffer_init(&producer_queues[i], producer_queue_sizes[i]);
    }

    // Create and initialize the bounded buffer for dispatcher queues
    Bounded_Buffer dispatcher_queues[3];
    for (int i = 0; i < 3; i++) {
        Bounded_Buffer_init(&dispatcher_queues[i], -1);  // Unbounded queues
    }

    // Create and initialize the bounded buffer for co-editor queue
    Bounded_Buffer coeditor_queue;
    Bounded_Buffer_init(&coeditor_queue, coeditor_queue_size);

    // Create and initialize the bounded buffer for screen manager queue
    Bounded_Buffer screen_manager_queue;
    Bounded_Buffer_init(&screen_manager_queue, -1);  // Unbounded queue

    // Create producer threads
    pthread_t producer_threads[num_producers];
    Producer producers[num_producers];
    for (int i = 0; i < num_producers; i++) {
        producers[i].id = i + 1;
        strcpy(producers[i].type, "SPORTS");
        producers[i].num_products = producer_num_products[i];
        producers[i].producer_queue = &producer_queues[i];
        pthread_create(&producer_threads[i], NULL, producer_thread, (void*)&producers[i]);
    }

    // Create dispatcher thread
    pthread_t dispatcher_thread_id;
    Dispatcher dispatcher;
    for (int i = 0; i < num_producers; i++) {
        dispatcher.producer_queues[i] = &producer_queues[i];
    }
    for (int i = 0; i < 3; i++) {
        dispatcher.dispatcher_queues[i] = &dispatcher_queues[i];
    }
    pthread_create(&dispatcher_thread_id, NULL, dispatcher_thread, (void*)&dispatcher);

    // Create co-editor threads
    pthread_t coeditor_threads[3];
    Co_Editor coeditors[3];
    for (int i = 0; i < 3; i++) {
        strcpy(coeditors[i].type, "SPORTS");
        coeditors[i].dispatcher_queue = &dispatcher_queues[i];
        coeditors[i].coeditor_queue = &coeditor_queue;
        pthread_create(&coeditor_threads[i], NULL, coeditor_thread, (void*)&coeditors[i]);
    }

    // Create screen manager thread
    pthread_t screen_manager_thread_id;
    Screen_Manager screen_manager;
    screen_manager.coeditor_queue = &coeditor_queue;
    screen_manager.done_count = 0;
    pthread_create(&screen_manager_thread_id, NULL, screen_manager_thread, (void*)&screen_manager);

    // Wait for all threads to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(producer_threads[i], NULL);
    }
    pthread_join(dispatcher_thread_id, NULL);
    for (int i = 0; i < 3; i++) {
        pthread_join(coeditor_threads[i], NULL);
    }
    pthread_join(screen_manager_thread_id, NULL);

    // Free memory
    for (int i = 0; i < num_producers; i++) {
        free(producer_queues[i].buffer);
    }
    free(coeditor_queue.buffer);
    free(screen_manager_queue.buffer);

    return 0;
}