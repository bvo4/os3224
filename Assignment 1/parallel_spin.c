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

/*
	Time using Mutex locks (8 threads): 
					Insert:  0.119231 seconds
					Retrieve:  4.238782 seconds

					Insert:  0.040694 seconds
					Retrieve:  4.020896 seconds

					Insert:	0.147313 seconds
					Retrieve:  4.528983 seconds

					(16 threads):

					Insert:  0.082118 seconds
					Retrieve:  4.811899 seconds

					Insert:  0.047970 seconds
					Retrieve:  5.670941 seconds
					
					(32 threads)
					
					Insert:  0.076449 seconds
					Retrieve:  5.998625 seconds					

					Insert:  0.059687 seconds
					Retrieve:  5.459100 seconds

	Time with spin locking (8 threads):
					Insert:  0.048081 seconds
					Retrieve:  2.277415 seconds

					Insert:  0.074558 seconds
					Retrieve:  2.593146 seconds

					Insert:	0.016828 seconds
					Retrieve:  2.571600 seconds

					(16 threads):

					Insert:  0.057044 seconds
					Retrieve:  2.714178 seconds

					Insert:  0.053510 seconds
					Retrieve:  3.264339 seconds
					
					(32 threads)
					
					Insert:  0.042117 seconds
					Retrieve:  2.927911 seconds					

					Insert:  0.048093 seconds
					Retrieve:  3.224393 seconds

	Spinlocking shows that it is more efficient than mutex lock for every thread case, cutting the time in half.  

	The time difference between the mutex lock and the spin lock decreased the more threads were added to the table.
	16 threads showed a smaller time difference between the mutex lock and the spin lock with mutex lock saving approximately half a second compared to the spinlock.
	Using 32 threads showed barely any time difference between the two locking mechanisms with the final case having a nearly identical time to retrieve for both spinlock and mutex lock.

	The spin-lock's overhead is because spinlocks have threads constantly checking the lock's state until it is released.  
	Mutex locks put the incoming threads to sleep while the threads wait for their turn, allowing the CPU to handle other things while the threads wait.
	Spin-lock burns more CPU cycles checking the thread states compared to the Mutex lock which uses CPU cycles to put threads to sleep and wake threads up.
	
	However, mutex locks suffer overhead from having to put threads to sleep and waking them up, making mutex locks inefficient for operations where threads do not run for very long.
	Spinlocks don't suffer as much because spinlocks have low overhead costs for locking and unlocking threads.
	In this case, the CPU burns more time putting the threads in a wait-queue than CPU burns when having threads enter a spinlock.

	
*/

pthread_spinlock_t lock;		//Declare a lock


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
	
	pthread_spin_lock(&lock);		//Acquire lock
    e->next = table[i];
    e->key = key;
    e->val = val;
    table[i] = e;
	    pthread_spin_unlock(&lock);	//Release lock
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

    
    for (key = tid ; key < NUM_KEYS; key += num_threads) {
        insert(keys[key], tid);
    }
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

pthread_spin_init(&lock, 0); // initialize the lock

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