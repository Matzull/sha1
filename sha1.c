#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/sha.h>

/* Function prototypes */

const int ITERS = 10000000;
const int DIFFICULTY = 1;

#define THREAD_COUNT 1

struct thread_arg_t
{
    int t_id;
    int difficulty;
    unsigned char digest[20];
    uint32_t msg[5];
};

bool HASH_FOUND = false;

void toHex(char* input)
{
    char digest[41];
	for (int i = 0; i < 20; i += 1) {
		snprintf(&digest[2 * i], 3, "%02x", input[i]&0xff);
	}
    digest[40] = '\0';
    printf("%s\n", digest);
}

void* hasher(void* arg)
{
    struct thread_arg_t* t_arg = (struct thread_arg_t*)arg;

    printf("Starting thread: %d\n", t_arg->t_id);
    unsigned char output[20];
    char hex[20];
    long long int i = t_arg->t_id;

    unsigned char msg[20] = {0};

    while (!HASH_FOUND)
    {
        SHA1((unsigned char*)t_arg->msg, strlen((const char *)t_arg->msg), t_arg->digest);
        //SHA1(msg, strlen(msg), t_arg->digest);
        printf("Nonce is: %s\n", msg);
        toHex(t_arg->digest);
        printf("\n");

        if (!memcmp(t_arg->digest, &(int){0}, t_arg->difficulty))
        {
            HASH_FOUND = true;
            printf("Message is: %s; Hash is: %s", (char*)t_arg->msg, t_arg->digest);
        }
        else
        {
            //(int)t_arg->msg[4]++;
            msg += 0x01;
            i+=THREAD_COUNT;
        }
        
    }
    return NULL;
}


/* Main program */

int main(void) {
	
    pthread_t threads[THREAD_COUNT];
	
    clock_t start_time = clock();

    struct thread_arg_t arg;

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        arg.difficulty = DIFFICULTY;
        arg.t_id = i;
        memset(arg.msg, 0x0, 20);
        pthread_create(&threads[i], NULL, hasher, &arg);
    }

    for (size_t i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }

    clock_t ending_time = clock();
	printf("Speed: %.2f H/s\n", (ITERS * THREAD_COUNT * CLOCKS_PER_SEC) / ((float)(ending_time - start_time)));
	
	return EXIT_SUCCESS;
}