#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define INITSIZEUB 8
typedef enum {Sports, News, Weather, Done} productType;

const char* getType(productType type) 
{
   switch (type) 
   {
      case Sports: return "SPORTS";
      case News: return "NEWS";
      case Weather: return "WEATHER";
   }
   return "";
}

typedef struct {
    int producerID;
    productType type;
    int produced;
} article;

void copyArt( article src, article* dst) {
    dst->producerID = src.producerID;
    dst->type = src.type;
    dst->produced = src.produced;
}

typedef struct {
    int size;
    article* articles;
    int start;
    int end;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
} Bounded;

void boundedInit(Bounded* bb, int size) {
    bb->size = size;
    bb->articles = (article*)calloc(size, sizeof(article));
    bb->start = 0;
    bb->end = 0;
    if (pthread_mutex_init(&bb->mutex, NULL) != 0) {
		printf("\n mutex init has failed\n");
		return;
	}
    sem_init(&bb->empty, 0, size);
    sem_init(&bb->full, 0, 0);
}

void boundedInsert(Bounded* bb, article art) {
    copyArt(art, &(bb->articles[bb->end]));
    bb->end = (bb->end + 1) % bb->size;
}

article boundedRemove(Bounded* bb) {
    article art;
    copyArt(bb->articles[bb->start], &art);
    bb->start = (bb->start + 1) % bb->size;

    return art;
}

typedef struct {
    article* articles;
    int size;
    int start;
    int last;
    sem_t thereIs;
    pthread_mutex_t mutex;
} UnBounded;

void unBoudedInit(UnBounded* ub) {
    ub->size = INITSIZEUB;
    ub->articles = (article*)calloc(ub->size, sizeof(article));
    ub->start = 0;
    ub->last = 0;
    if (pthread_mutex_init(&ub->mutex, NULL) != 0) {
		printf("\n mutex init has failed\n");
		return;
	}
    sem_init(&ub->thereIs, 0, 0);
}

void unBoundedInsert(UnBounded* ub, article art) {
    copyArt(art, &(ub->articles[ub->last]));
    ub->last++;
    if (ub->last == ub->size) {
        ub->size += 2;
        ub->articles = (article*)realloc(ub->articles, ub->size * sizeof(article));
    }
}

article unBoudedRemove(UnBounded* ub) {
    article art;
    copyArt(ub->articles[ub->start], &art);
    ub->start = ub->start + 1;
    return art;
}

void printArticle(article message) {
    printf("Producer %d %s %d\n", message.producerID,
    getType(message.type), message.produced);
}

void printBounded(Bounded* bounded) {
    printf("----bounded----\n");
    int i = bounded->start;
    while (i != bounded->end) {
        printArticle(bounded->articles[i]);
        i = (i + 1) % bounded->size;
    }
    printf("----boundend----\n");
}
// producer

typedef struct {
    int id;
    int produced[3];
    int toProduce;
    Bounded* queue;
} Producer;

void* producerThread(void* producerArg) {
    Producer* producer = (Producer*)producerArg;
    for (int i = 0; i < producer->toProduce; i++) {
        article product;
        int type = rand() % 3;
        product.producerID = producer->id;
        product.type = type;
        product.produced = producer->produced[type];
        producer->produced[type] += 1;
        sem_wait(&(producer->queue->empty));
        pthread_mutex_lock(&(producer->queue->mutex));
        boundedInsert(producer->queue, product);
        sem_post(&(producer->queue->full));
        pthread_mutex_unlock(&(producer->queue->mutex));
    }
    article done = {0, Done, 0};
    sem_wait(&(producer->queue->empty));
    pthread_mutex_lock(&(producer->queue->mutex));
    boundedInsert(producer->queue, done);
    sem_post(&(producer->queue->full));
    pthread_mutex_unlock(&(producer->queue->mutex));
    pthread_exit(NULL);
}

// Dispatcher

typedef struct {
    int amountProducers;
    int producersEnded;
    Bounded** producer_queues;
    UnBounded** dispatcher_queues;
} Dispatcher;

void* dispatcherThread(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    int producer_idx = 0;
    article done = {0, Done, 0};
    while (1) {
        Bounded* producer_queue = dispatcher->producer_queues[producer_idx];
        // printBounded(producer_queue);
        sem_wait(&(producer_queue->full));
        pthread_mutex_lock(&(producer_queue->mutex));
        article message = boundedRemove(producer_queue);
        sem_post(&(producer_queue->empty));
        pthread_mutex_unlock(&(producer_queue->mutex));
        if (message.type == Done) {
            sem_destroy(&(dispatcher->producer_queues[producer_idx]->empty));
            sem_destroy(&(dispatcher->producer_queues[producer_idx]->full));
            pthread_mutex_destroy(&(dispatcher->producer_queues[producer_idx]->mutex));
            free(dispatcher->producer_queues[producer_idx]->articles);
            free(dispatcher->producer_queues[producer_idx]);
            dispatcher->producer_queues[producer_idx] = NULL;
            (dispatcher->producersEnded)++;
            if (dispatcher->producersEnded == dispatcher->amountProducers) {
                pthread_mutex_lock(&(dispatcher->dispatcher_queues[Sports]->mutex));
                unBoundedInsert(dispatcher->dispatcher_queues[Sports], done);
                sem_post(&(dispatcher->dispatcher_queues[Sports]->thereIs));
                pthread_mutex_unlock(&(dispatcher->dispatcher_queues[Sports]->mutex));

                pthread_mutex_lock(&(dispatcher->dispatcher_queues[News]->mutex));
                unBoundedInsert(dispatcher->dispatcher_queues[News], done);
                sem_post(&(dispatcher->dispatcher_queues[News]->thereIs));
                pthread_mutex_unlock(&(dispatcher->dispatcher_queues[News]->mutex));

                pthread_mutex_lock(&(dispatcher->dispatcher_queues[Weather]->mutex));
                unBoundedInsert(dispatcher->dispatcher_queues[Weather], done);
                sem_post(&(dispatcher->dispatcher_queues[Weather]->thereIs));
                pthread_mutex_unlock(&(dispatcher->dispatcher_queues[Weather]->mutex));
                break;
            }
        } else {
            pthread_mutex_lock(&(dispatcher->dispatcher_queues[message.type]->mutex));
            // printf("in\n");
            unBoundedInsert(dispatcher->dispatcher_queues[message.type], message);
            // printf("sent\n");
            sem_post(&(dispatcher->dispatcher_queues[message.type]->thereIs));
            pthread_mutex_unlock(&(dispatcher->dispatcher_queues[message.type]->mutex));
        }
        do {
            producer_idx = (producer_idx + 1) % dispatcher->amountProducers;
        } while(!dispatcher->producer_queues[producer_idx]);
    }
    pthread_exit(NULL);
}

// Co-Editor

typedef struct {
    productType type;
    UnBounded* dispatcherQueue;
    Bounded* screenQueue;
} Co_Editor;

void* coEditorThread(void* arg) {
    Co_Editor* coeditor = (Co_Editor*)arg;
    article done = {0, Done, 0};
    while (1) {
        // printf("coeditor %s\n", getType(coeditor->type));
        sem_wait(&(coeditor->dispatcherQueue->thereIs));
        pthread_mutex_lock(&(coeditor->dispatcherQueue->mutex));
        article message = unBoudedRemove(coeditor->dispatcherQueue);
        pthread_mutex_unlock(&(coeditor->dispatcherQueue->mutex));
        //printArticle(message);

        // Simulate editing process
        if (message.type != Done)
            usleep(100000);  // 0.1 seconds

        sem_wait(&(coeditor->screenQueue->empty));
        pthread_mutex_lock(&(coeditor->screenQueue->mutex));
        boundedInsert(coeditor->screenQueue, message);
        sem_post(&(coeditor->screenQueue->full));
        pthread_mutex_unlock(&(coeditor->screenQueue->mutex));
        if (message.type == Done)
            break;
    }
    pthread_exit(NULL);
}

// Screen Manager

typedef struct {
    Bounded* screenQueue;
    int done_count;
} ScreenManager;

void* screenManagerThread(void* arg) {
    ScreenManager* screenManager = (ScreenManager*)arg;
    while (1) {
        // printf("screenmanager\n");
        sem_wait(&(screenManager->screenQueue->full));
        pthread_mutex_lock(&(screenManager->screenQueue->mutex));
        article message = boundedRemove(screenManager->screenQueue);
        sem_post(&(screenManager->screenQueue->empty));
        pthread_mutex_unlock(&(screenManager->screenQueue->mutex));

        if (message.type == Done) {
            screenManager->done_count++;
            if (screenManager->done_count == 3) {
                printf("DONE\n");
                break;
            }
            continue;
        } else {
            printf("Producer %d %s %d\n", message.producerID,
            getType(message.type), message.produced);
        }
    }
    pthread_exit(NULL);
}

int main( int argc, char *argv[] )  {
    srand(time(NULL));
    FILE *file;
    int **numbers = NULL;
    int rows = 0;
    int cols = 0;
    int singleNumber;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("error opening file\n");
        return 1;
    }

    numbers = (int **)malloc(sizeof(int *));
    while (fscanf(file, "%d", &singleNumber) == 1) {
        if (cols == 0) {
            if (rows == 0) {
            } else {
                numbers = (int **)realloc(numbers, (rows + 1) * sizeof(int *));
            }
        }

        if (numbers == NULL) {
            printf("error allocating mem\n");
            fclose(file);
            return 1;
        }
        if (cols == 0)
            numbers[rows] = (int *)calloc(3,  sizeof(int));
        if (numbers[rows] == NULL) {
            printf("error allocating mem\n");
            fclose(file);
            return 1;
        }

        numbers[rows][cols] = singleNumber;
        cols++;
        if (cols == 3) {
            cols = 0;
            rows++;
        }
    }
    fclose(file);
    free(numbers[rows]);
    int lastSingleNumber = singleNumber;
    int numOfProducers = rows;

    Bounded** producersQ = (Bounded**)calloc(numOfProducers,
    sizeof(Bounded*));
    Producer* producers = (Producer*)calloc(numOfProducers,
    sizeof(Producer));
    for (int i = 0; i < numOfProducers; i++) {
        producersQ[i] = (Bounded*)malloc(sizeof(Bounded));
        boundedInit(producersQ[i], numbers[i][2]);
        producers[i].id = numbers[i][0];
        producers[i].produced[0] = 0;
        producers[i].produced[1] = 0;
        producers[i].produced[2] = 0;
        producers[i].queue = producersQ[i];
        producers[i].toProduce = numbers[i][1];
    }
    UnBounded** dispatcherQ = (UnBounded**)calloc(3, sizeof(UnBounded*));
    for (int i = 0; i < 3; i++) {
        dispatcherQ[i] = (UnBounded*)malloc(sizeof(UnBounded));
        unBoudedInit(dispatcherQ[i]);
    }
    Dispatcher disp = {numOfProducers, 0, producersQ, dispatcherQ};

    Bounded* screenQ = (Bounded*)malloc(sizeof(Bounded));;
    boundedInit(screenQ, lastSingleNumber);
    Co_Editor coeditors[3];
    for(int i = 0; i < 3; i++) {
        coeditors[i].type = i;
        coeditors[i].dispatcherQueue = dispatcherQ[i];
        coeditors[i].screenQueue = screenQ;
    }

    ScreenManager sm = {screenQ, 0};
    pthread_t* producersT = (pthread_t*)calloc(numOfProducers,
    sizeof(pthread_t));
    pthread_t dispatcherT, coeditorT[3], smT;
    for(int i = 0; i < numOfProducers; i++) {
        pthread_create(&(producersT[i]), NULL, producerThread,
        (void*)&(producers[i]));
    }
    pthread_create(&dispatcherT, NULL, dispatcherThread, (void*)&disp);
    for (int i = 0; i < 3; i++) {
        pthread_create(&coeditorT[i], NULL, coEditorThread, (void*)&coeditors[i]);
    }
    pthread_create(&smT, NULL, screenManagerThread, (void*)&sm);
    pthread_join(smT, NULL);
    for (int i = 0; i < numOfProducers; i++) {
        pthread_join(producersT[i], NULL);
    }
    pthread_join(dispatcherT, NULL);
    for (int i = 0; i < 3; i++) {
        pthread_join(coeditorT[i], NULL);
    }


    for (int i = 0; i < numOfProducers; i++) {
        free(producersQ[i]);
    }
    free(producersQ);
    free(producers);
    free(producersT);
    for (int i = 0; i < 3; i++) {
        sem_destroy(&(dispatcherQ[i]->thereIs));
        pthread_mutex_destroy(&(dispatcherQ[i]->mutex));
        free(dispatcherQ[i]->articles);
        free(dispatcherQ[i]);
    }
    free(dispatcherQ);
    sem_destroy(&(screenQ->empty));
    sem_destroy(&(screenQ->full));
    pthread_mutex_destroy(&(screenQ->mutex));
    free(screenQ->articles);
    free(screenQ);
    for (int i = 0; i < rows; i++) {
        free(numbers[i]);
    }
    free(numbers);

    return 0;
}