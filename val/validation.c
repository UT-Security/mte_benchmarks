#include "util.h"
#include "sha256.h"
#include <stdio.h>
#include <string.h>

// testmte = 1: Test oob access does not work with MTE. Expect result: crush if MTE is enabled.
// testmte = 2: Test memory tag after madvice. Expected result: madvice clears memory tagging.
// testmte = 3: Access tagged memory with no-tag pointer. Expect result: crush if MTE is enabled.
int test_mte(int testmte){

    int page_size = sysconf(_SC_PAGESIZE);

    if(testmte == 1){

        unsigned char *temp1, *temp2;
        temp1 = mmap_option(1, page_size);
        temp2 = mmap_option(1, page_size);
        printf("temp1: %p\n", temp1);
        printf("temp2: %p\n", temp2);
        unsigned long long curr_key1 = (unsigned long long) (1);
        unsigned long long curr_key2 = (unsigned long long) (2);
        temp1 = mte_tag(temp1, curr_key1, 0);
        temp2 = mte_tag(temp2, curr_key2, 0);
        printf("temp1 after tag: %p\n", temp1);
        printf("temp2 after tag: %p\n", temp2);
        unsigned char *temp1_no_tag = (unsigned char *) ((uint64_t) temp1 & 0x00FFFFFFFFFFFFFF);
        unsigned char *temp2_no_tag = (unsigned char *) ((uint64_t) temp2 & 0x00FFFFFFFFFFFFFF);
        printf("temp1_no_tag: %p\n", temp1_no_tag);
        printf("temp2_no_tag: %p\n", temp2_no_tag);
        uint64_t diff = (uint64_t) temp2_no_tag - (uint64_t) temp1_no_tag;
        printf("diff: %" PRIu64 "\n", diff);
        printf("temp1[temp2-temp1]: %d\n", temp1[temp2 - temp1]); // this works
        printf("temp1[diff]: %d\n", temp1[diff]); // this does not work
        munmap(temp1, page_size);
        munmap(temp2, page_size);

    }else if((testmte == 2) || (testmte == 3)){

        unsigned char *temp1 = mmap_option(1, page_size);
        printf("temp1: %p\n", temp1);
        unsigned long long curr_key1 = (unsigned long long) (1);
        temp1 = mte_tag(temp1, curr_key1, 0);
        printf("temp1 after tag: %p\n", temp1);
        unsigned char *temp1_no_tag = (unsigned char *) ((uint64_t) temp1 & 0x00FFFFFFFFFFFFFF);
        printf("temp1_no_tag: %p\n", temp1_no_tag);
        if(testmte==2){
            madvise(temp1, page_size, MADV_DONTNEED);
        }
        temp1_no_tag[0] = 100;
        printf("temp1_no_tag[0]: %d\n", temp1_no_tag[0]);
        munmap(temp1, page_size);
    }
    return 0;
            
}

void bench_no_mte(int buffer_size, int workload, int setup_teardown, int iteration){
    double elapsed_time = 0;
    struct timeval start, end;

    unsigned char * buffer = NULL;

    MEASURE_TIME(
        buffer = mmap_option(-1, buffer_size);
        mprotect_option(0, buffer, buffer_size);
        , 
        "Setup"
    );

    // Init some variables
    uint64_t** buffer_8;
    int buffer_size_8 = buffer_size/8;

    if(setup_teardown==1){

        MEASURE_TIME(
            int ret = madvise(buffer, buffer_size, MADV_DONTNEED);
            if (ret != 0) {
                perror("madvise failed");
                exit(EXIT_FAILURE);
            }
            ,
            "Tear down"
        );

    }else{

        for (int i = 0; i < iteration; i++){
            if(workload==0){
                MEASURE_TIME(
                    // Fill buffer with pointers chasing
                    buffer_8 = (uint64_t**)buffer;
                    create_random_chain(buffer_size_8, buffer_8);
                    , 
                    "Fill buffer with pointer chasing"
                );
                    
                MEASURE_TIME(
                    chase_pointers(buffer_8, buffer_size_8);
                    , 
                    "Access"
                );
            }else if(workload==1){
                MEASURE_TIME(
                    memset(buffer, 1, buffer_size);
                    unsigned char digest[SHA256_BLOCK_SIZE];
                    sha256(buffer, buffer_size, digest);
                    , 
                    "Memset and Hash"
                );
            }
            

            MEASURE_TIME(
                int ret = madvise(buffer, buffer_size, MADV_DONTNEED);
                if (ret != 0) {
                    perror("madvise failed");
                    exit(EXIT_FAILURE);
                }
                ,
                "Tear down"
            );
        }

    }

    munmap(buffer, buffer_size);
}



void bench_mte(int buffer_size, int workload, int setup_teardown, int iteration){
    double elapsed_time = 0;
    struct timeval start, end;

    unsigned char * buffer = NULL;

    MEASURE_TIME(
        buffer = mmap_option(-1, buffer_size);
        mprotect_option(1, buffer, buffer_size);
        , 
        "Setup with MTE"
    );

    // Apply tags
    unsigned long long curr_key = (unsigned long long) (1);
    unsigned char *buffer_tag;
    uint64_t** buffer_8;
    int buffer_size_8 = buffer_size/8;

    if(setup_teardown==1){
        
        MEASURE_TIME(
            buffer_tag = mte_tag(buffer, curr_key, 0);
            for (int j = 16; j < buffer_size; j += (size_t) 16){
                mte_tag(buffer+j, curr_key, 0);
            }
            , 
            "Apply MTE Tag"
        );

        MEASURE_TIME(
            int ret = madvise(buffer_tag, buffer_size, MADV_DONTNEED);
            if (ret != 0) {
                perror("madvise failed");
                exit(EXIT_FAILURE);
            }
            ,
            "Tear down with MTE"
        );

    }else{

        for (int i = 0; i < iteration; i++)
        {
            
            MEASURE_TIME(
                buffer_tag = mte_tag(buffer, curr_key, 0);
                for (int j = 16; j < buffer_size; j += (size_t) 16){
                    mte_tag(buffer+j, curr_key, 0);
                }
                , 
                "Apply MTE Tag"
            );

            if(workload==0){
                MEASURE_TIME(
                    // Fill buffer with pointers chasing
                    buffer_8 = (uint64_t**)buffer_tag;
                    create_random_chain(buffer_size_8, buffer_8);
                    , 
                    "Fill buffer with pointer chasing"
                );

                MEASURE_TIME(
                    chase_pointers((uint64_t**)buffer_tag, buffer_size_8);
                    , 
                    "Access with MTE"
                );
            }else if(workload==1){
                MEASURE_TIME(
                    memset(buffer_tag, 1, buffer_size);
                    unsigned char digest[SHA256_BLOCK_SIZE];
                    sha256(buffer_tag, buffer_size, digest);
                    , 
                    "Memset and Hash with MTE"
                );
            }

            MEASURE_TIME(
                int ret = madvise(buffer_tag, buffer_size, MADV_DONTNEED);
                if (ret != 0) {
                    perror("madvise failed");
                    exit(EXIT_FAILURE);
                }
                ,
                "Tear down with MTE"
            );
        }
    }

    munmap(buffer, buffer_size);
}

int main(int argc, char *argv[])
{
	
    int testmte = atoi(argv[1]);
    int buffer_size = atoi(argv[2]);
    int setup_teardown = atoi(argv[3]);
    int workload = atoi(argv[4]);
    int iteration = atoi(argv[5]);

    printf("testmte %d\n", testmte);
    printf("buffer_size %d\n", buffer_size);
    printf("setup_teardown %d\n", setup_teardown);
    printf("workload %d\n", workload);
    printf("iteration %d\n", iteration);

    if(buffer_size%16!=0){
        return 0;
    }

    // PIN to CPU core 0
    pin_cpu(0);

    // Initialize MTE
    init_mte();

    if(testmte > 0){ // we want to test MTE works without benchmark anything
        test_mte(testmte);
        return 0;
    }else if(testmte == 0){ // benchmark mte behavior
        if(setup_teardown==1){
            for (int i = 0; i < iteration; i++){
                bench_mte(buffer_size, workload, setup_teardown, iteration);
            }
        }else{
            bench_mte(buffer_size, workload, setup_teardown, iteration);
        }
        
    }else if(testmte == -1){ // benchmark no mte behavior
        if(setup_teardown==1){
            for (int i = 0; i < iteration; i++){
                bench_no_mte(buffer_size, workload, setup_teardown, iteration);
            }
        }else{
            bench_no_mte(buffer_size, workload, setup_teardown, iteration);
        }
    }

    return 0;
}