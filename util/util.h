#ifndef _MY_UTIL_H
#define _MY_UTIL_H

#define _GNU_SOURCE

#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/time.h>
#include "sha256.h"

// For MTE
#define PROT_MTE                 0x20
#define total_key                16

// set tag on the memory region
#define set_tag(tagged_addr) do {                                      \
        asm volatile("stg %0, [%0]" : : "r" (tagged_addr) : "memory"); \
} while (0)

// Insert with a user-defined tag
#define insert_my_tag(ptr, input_tag) \
    ((unsigned char*)((uint64_t)(ptr) | ((input_tag) << 56)))

// Insert a random tag
#define insert_random_tag(ptr) ({                       \
        uint64_t __val;                                 \
        asm("irg %0, %1" : "=r" (__val) : "r" (ptr));   \
        __val;                                          \
})

#define MEASURE_TIME(code, header_str) \
do { \
        struct timeval start, end; \
        double elapsed_time; \
        gettimeofday(&start, NULL); \
        { \
            code; \
        } \
        gettimeofday(&end, NULL); \
        elapsed_time = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec); \
        printf("%s time is %f us\n", header_str, elapsed_time); \
} while (0)


void pin_cpu(size_t core_ID);

int init_mte();

unsigned char * mmap_option(int mte, int size);

void mprotect_option(int mte, unsigned char * ptr, int size);

unsigned char * mte_tag(unsigned char *ptr, unsigned long long tag, int random);

void create_random_chain(int len, uint64_t** ptr);

void chase_pointers(uint64_t** ptr, int count);

void sha256(const unsigned char * str, int size, unsigned char hash[]);


#endif