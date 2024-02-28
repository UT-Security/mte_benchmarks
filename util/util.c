#include "util.h"

volatile uint64_t* chase_pointers_global; // to defeat optimizations for pointer chasing algorithm
SHA256_CTX ctx; // for sha256

void pin_cpu(size_t core_ID)
{
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(core_ID, &set);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &set) < 0) {
		printf("Unable to Set Affinity\n");
		exit(EXIT_FAILURE);
	}
}


int init_mte(){
    /*
     * Use the architecture dependent information about the processor
     * from getauxval() to check if MTE is available.
     */
    if (!((getauxval(AT_HWCAP2)) & HWCAP2_MTE)){
        printf("MTE is not supported\n");
        return EXIT_FAILURE;
    }else{
        printf("MTE is supported\n");
    }

    /*
     * Enable MTE with synchronous checking
     */
    if (prctl(PR_SET_TAGGED_ADDR_CTRL,
              PR_TAGGED_ADDR_ENABLE | PR_MTE_TCF_SYNC | (0xfffe << PR_MTE_TAG_SHIFT),
              0, 0, 0)){
            perror("prctl() failed");
            return EXIT_FAILURE;
    }
    return 0;
}


unsigned char * mmap_option(int mte, int size){
    unsigned char *ptr;
    if(mte==1){
        ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_MTE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED){
            perror("mmap() failed");
            exit(EXIT_FAILURE);
        }
    }else if(mte==0){
        ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED){
            perror("mmap() failed");
            exit(EXIT_FAILURE);
        }
    }else if(mte==-1){
        ptr = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED){
            perror("mmap() failed");
            exit(EXIT_FAILURE);
        }
    }
    return ptr;
}


void mprotect_option(int mte, unsigned char * ptr, int size){
    int ret = 0;
    if(mte==1){
        ret = mprotect(ptr, size, PROT_READ | PROT_WRITE | PROT_MTE);
        if (ret != 0) {
            perror("mprotect failed");
            exit(EXIT_FAILURE);
        }
    }else if(mte==0){
        ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
        if (ret != 0) {
            perror("mprotect failed");
            exit(EXIT_FAILURE);
        }
    }
}

unsigned char * mte_tag(unsigned char *ptr, unsigned long long tag, int random){
    unsigned char * ptr_mte = NULL;
    if(random==1){
        ptr_mte = (unsigned char *) insert_random_tag(ptr);
    }else{
        ptr_mte = insert_my_tag(ptr, tag);
    }

    set_tag(ptr_mte);
    return ptr_mte;
}


/* create a cyclic pointer chain that covers all words
   in a memory section of the given size in a randomized order */
// Inspired by: https://github.com/afborchert/pointer-chasing/blob/master/random-chase.cpp
void create_random_chain(int len, uint64_t** ptr) {

    // shuffle indices
    int* indices = (int*) malloc(len*sizeof(int));
    for (int i = 0; i < len; ++i) {
        indices[i] = i;
    }
    for (int i = 0; i < len-1; ++i) {
        int j = i + rand()%(len - i);
        if (i != j) {
            int temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
        }
    }
    // fill memory with pointer references
    for (int i = 1; i < len; ++i) {
        ptr[indices[i-1]] = (uint64_t*) &ptr[indices[i]];
    }
    ptr[indices[len-1]] = (uint64_t*) &ptr[indices[0]];
    free(indices);
}

void chase_pointers(uint64_t** ptr, int count) {
   // chase the pointers count times
   uint64_t** p = (uint64_t**) ptr;
   while (count-- > 0) {
      p = (uint64_t**) *p;
   }
   chase_pointers_global = *p;
}


void sha256(const unsigned char * str, int size, unsigned char hash[]){
    sha256_init(&ctx);
	sha256_update(&ctx, str, size);
	sha256_final(&ctx, hash);
}

