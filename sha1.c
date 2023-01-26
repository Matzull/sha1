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
const int DIFFICULTY = 4;

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

inline char* toHex(char* input, char* output)
{
	for (int i = 0; i < 20; i += 1) {
		snprintf(&output[2 * i], 3, "%02x", input[i]&0xff);
	}
    output[40] = '\0';
    return output;
}

inline int compare_DUCO_S1(
    const unsigned char hex_digest[2*20],
    const unsigned char byte_digest[20], int diff)
{
    for(int i = 0; i < diff; i++){
        // Converts from byte to hexadecimal
        char msb = "0123456789abcdef"[byte_digest[i]>>4],
             lsb = "0123456789abcdef"[byte_digest[i]&0x0f];
        //printf("Msb: %c Lsb: %c Target %c%c\n", msb, lsb, hex_digest[2*i], hex_digest[2*i + 1]);
        if(msb != hex_digest[2*i] || lsb != hex_digest[2*i+1])
        {
            return 0;
        } 
    }
    return 1;
}

void* hasher(void* arg)
{
    struct thread_arg_t* t_arg = (struct thread_arg_t*)arg;

    printf("Starting thread: %d\n", t_arg->t_id);
    int i = t_arg->t_id;
    (*(uint64_t*)&(t_arg->msg[12])) += i;
    char outputmsg[41];
    char outputhsh[41];
    unsigned char hex_target[40];
    long long int L_ITERS = 0;
    toHex((char[20]){0}, hex_target);
    SHA_CTX base_ctx;
    SHA_CTX temp_ctx;
    SHA1_Init(&base_ctx);
    printf("Thread with id: %d msg %s\n", i, toHex((char*)t_arg->msg, (char*)outputmsg));
    // SHA1_Update(&base_ctx, input_prefix, 40);
    while (!HASH_FOUND)
    {
        temp_ctx = base_ctx;
        SHA1_Update(&temp_ctx, t_arg->msg, 20);
        SHA1_Final(t_arg->digest, &temp_ctx);

        //printf("Message is: %s; Hash is: %s\n", toHex(t_arg->msg, outputmsg), toHex(t_arg->digest, outputhsh));
        if(compare_DUCO_S1(hex_target, t_arg->digest, DIFFICULTY))
        {
            HASH_FOUND = true;
            printf("Message is: %s; Hash is: %s\n", toHex((char*)t_arg->msg, (char*)outputmsg), toHex((char*)t_arg->digest, (char*)outputhsh));
        }
        else
        {           
            (*(uint64_t*)&(t_arg->msg[12]))+=THREAD_COUNT;
            L_ITERS++;
            if (L_ITERS % 10000000 == 0)
            {
                printf("%lld Iterations. Calculating ...\n", L_ITERS);
            }
        }
    }
    pthread_mutex_lock(&mt);
    ITERS += L_ITERS;
    pthread_mutex_unlock(&mt);
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