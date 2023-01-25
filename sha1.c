#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <alloca.h>

/* Function prototypes */

long long int ITERS = 0;
const int DIFFICULTY = 3;

#define THREAD_COUNT 16

struct thread_arg_t
{
    int t_id;
    int difficulty;
    unsigned char digest[20];
    unsigned char msg[20];
};

bool HASH_FOUND = false;

pthread_mutex_t mt;

char* toHex(char* input, char* output)
{
	for (int i = 0; i < 20; i += 1) {
		snprintf(&output[2 * i], 3, "%02x", input[i]&0xff);
	}
    output[40] = '\0';
    return output;
}

void* hasher(void* arg)
{
    struct thread_arg_t* t_arg = (struct thread_arg_t*)arg;

    printf("Starting thread: %d\n", t_arg->t_id);
    long long int i = t_arg->t_id;

    char outputmsg[41];
    char outputhsh[41];
    const unsigned char input_prefix[40] = {0}; 
    SHA_CTX base_ctx;
    SHA_CTX temp_ctx;
    SHA1_Init(&base_ctx);
    SHA1_Update(&base_ctx, input_prefix, 40);
    while (!HASH_FOUND)
    {
        //SHA1(t_arg->msg, 20, t_arg->digest);
        temp_ctx = base_ctx;
        SHA1_Update(&temp_ctx, t_arg->msg, 20);
        SHA1_Final(t_arg->digest, &temp_ctx);

        printf("Message is: %s; Hash is: %s\n", toHex(t_arg->msg, outputmsg), toHex(t_arg->digest, outputhsh));
        if (!memcmp(t_arg->digest, &(int){0}, t_arg->difficulty))
        {
            HASH_FOUND = true;
            printf("Message is: %s; Hash is: %s\n", toHex((char*)t_arg->msg, (char*)outputmsg), toHex((char*)t_arg->digest, (char*)outputhsh));
        }
        else
        {           
            (*(uint64_t*)&(t_arg->msg[12]))++;
            pthread_mutex_lock(&mt);
            ITERS++;
            if (ITERS %1000000 == 0)
            {
                printf("%lld Iterations. Calculating ...\n", ITERS);
            }
            pthread_mutex_unlock(&mt);
            i+=THREAD_COUNT;
            usleep(10000);
        }
    }
    return NULL;
}


/* Main program */

int main(void) {
	
    pthread_t threads[THREAD_COUNT];
	
    clock_t start_time = clock();

    pthread_mutex_init(&mt, NULL);

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        struct thread_arg_t* arg = alloca(sizeof(struct thread_arg_t));
        arg->difficulty = DIFFICULTY;
        arg->t_id = i;
        memset(arg->msg, 0, 20);
        pthread_create(&threads[i], NULL, hasher, arg);
    }

    for (size_t i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    clock_t ending_time = clock();
	printf("Speed: %.2f H/s\n", (ITERS * THREAD_COUNT * CLOCKS_PER_SEC) / ((float)(ending_time - start_time)));
	
	return EXIT_SUCCESS;
}