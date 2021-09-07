#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#define NUM_BUCKETS 5     // Buckets in hash table
#define NUM_KEYS 100000   // Number of keys inserted per thread
int num_threads = 1;      // Number of threads (configurable)
int keys[NUM_KEYS];

pthread_mutex_t lock;		//Declare a lock

/*
Multiple writes can happen simulataneously, multiple reads can happen simulataneously.

	Time before mutex locking (8 threads): 
					Insert:  0.119231 seconds
					Retrieve:  4.238782 seconds

					Insert:  0.040694 seconds
					Retrieve:  4.020896 seconds

					Insert:	0.147313 seconds
					Retrieve:  4.528983 seconds

	Time with mutex locking (8 threads):
					Insert:  0.006027 seconds
					Retrieve:  7.576814 seconds

					Insert:  0.007059 seconds
					Retrieve:  5.668296 seconds

					Insert:  0.022609 seconds
					Retrieve:  6.202725 seconds
					
As seen above, Mutex locking adds an overhead time of atleast 1 second in the retrieval time while the key insertion time remains relatively untouched.
	Mutex locking is choosing consistency over efficiency because of the necessary overhead of forcing other pthreads to wait their turn before taking the keys.

Part 3:  

If a write-based mutex lock design is used, then retrieving items should not require a lock.  
In this scenario, the write-system occurs when we insert our keys.

There is no simultaneous reads and writes occuring due to the addition of the mutex lock in the insert operations.
There is no race condition when a thread enters the retrieve operations.
The code already has the retrieval operations run in parallel.

Part 4:

	After allowing for parallelization (8 threads):

					Insert:  0.050999 seconds
					Retrieve:  2.749626 seconds

					Insert:  0.034869 seconds
					Retrieve:  2.637185 seconds

					Insert:  0.034869 seconds
					Retrieve:  2.989683 seconds
					
					(16 threads):
					
					Insert:  0.038141 seconds
					Retrieve:  3.357188 seconds
					
					Insert:  0.044549 seconds
					Retrieve:  3.250386 seconds
					
					Insert:  0.058516 seconds
					Retrieve:  3.445093 seconds
					
					(32 threads):
					
					Insert:  0.043325 seconds
					Retrieve:  3.785610 seconds

					Insert:  0.054126 seconds
					Retrieve:  3.716878 seconds

					Insert:  0.061497 seconds
					Retrieve:  3.634742 seconds


The parallelization of the insert operation allows for multiple threads to run in parallel.
This reduces the system retrieval time by half compared to the previous iteration of the mutex lock.
An issue the insert operations were having was that multiple threads were entering into the same hash table buckets.
The addition removes the race condition while allowing for parallelization by adding a lock when the thread enters an unoccupied hash bucket.
This allows multiple threads to run in parallel as long as they are in different buckets.

*/

typedef struct _bucket_entry {
    int key;
    int val;
    struct _bucket_entry *next;
} bucket_entry;

bucket_entry *table[NUM_BUCKETS];

void panic(char *msg) {
    printf("%s\n", msg);
    exit(1);
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Inserts a key-value pair into the table
void insert(int key, int val) {
    int i = key % NUM_BUCKETS;
    bucket_entry *e = (bucket_entry *) malloc(sizeof(bucket_entry));
    if (!e) panic("No memory to allocate bucket!");

	/*
	i ranges from 0 to 4.
	*/
    //printf("i is %d\n", i);

	//Critical section:

/*****Insert operations run in parallel******/

	if(i==0)
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	else if(i==1)
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	else if(i==2)
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	else if(i==3)
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	else if(i==4)
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	else
	{
pthread_mutex_lock(&lock);	//Acquire lock

    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;

pthread_mutex_unlock(&lock);	//Release lock
	}
	
/*****Insert operations run in parallel******/

}

// Retrieves an entry from the hash table by key
// Returns NULL if the key isn't found in the table
bucket_entry * retrieve(int key) {

    bucket_entry *b;
    for (b = table[key % NUM_BUCKETS]; b != NULL; b = b->next) {
        if (b->key == key) return b;

    }
    return NULL;
}

void * put_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;

    // If there are k threads, thread i inserts
    //      (i, i), (i+k, i), (i+k*2)

    /*
	This is where the keys are being taken by the threads.  
	A mutex lock is necessary to force the threads to wait and take the keys to prevent overlap and lost keys.
	This is having a thread enter, take a key, then leaving before the other threads come in.
    */
	//The insert is where the critical section begins
    //pthread_mutex_lock(&lock);		//Acquire lock
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }
    //pthread_mutex_unlock(&lock);	//Release lock
    pthread_exit(NULL);
}

void * get_phase(void *arg) {
    long tid = (long) arg;
    int key = 0;
    long lost = 0;

    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        if (retrieve(keys[key]) == NULL) lost++;
    }
    printf("[thread %ld] %ld keys lost!\n", tid, lost);

    pthread_exit((void *)lost);
}

int main(int argc, char **argv) {
    long i;
    pthread_t *threads;
    double start, end;

    if (argc != 2) {
        panic("usage: ./parallel_hashtable <num_threads>");
    }
    if ((num_threads = atoi(argv[1])) <= 0) {
        panic("must enter a valid number of threads to run");
    }

pthread_mutex_init(&lock, NULL); // initialize the lock

    srandom(time(NULL));
    for (i = 0; i < NUM_KEYS; i++)
        keys[i] = random();

    threads = (pthread_t *) malloc(sizeof(pthread_t)*num_threads);
    if (!threads) {
        panic("out of memory allocating thread handles");
    }

    // Insert keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, put_phase, (void *)i);
    }    

    // Barrier
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end = now();
    
    printf("[main] Inserted %d keys in %f seconds\n", NUM_KEYS, end - start);
    
    // Reset the thread array
    memset(threads, 0, sizeof(pthread_t)*num_threads);

    // Retrieve keys in parallel
    start = now();
    for (i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, get_phase, (void *)i);
    }

    // Collect count of lost keys
    long total_lost = 0;
    long *lost_keys = (long *) malloc(sizeof(long) * num_threads);
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], (void **)&lost_keys[i]);
        total_lost += lost_keys[i];
    }
    end = now();

    printf("[main] Retrieved %ld/%d keys in %f seconds\n", NUM_KEYS - total_lost, NUM_KEYS, end - start);

    return 0;
}