/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <stdio.h>
#include <math.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

extern int mem_access(int addr, int write_flag, int write_data);
extern int get_num_mem_accesses();

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int valid;
    int beginAt;
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int lruLabel;
    int set;
    int tag;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;
int hits = 0;
int misses = 0;
int writebacks = 0;

void printAction(int, int, enum actionType);
void printCache();

/*
 * Set up the cache with given command line parameters. This is 
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet) {
    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;

    for (int b = 0; b < MAX_CACHE_SIZE; b++) {
        cache.blocks->valid = 0;
        cache.blocks->dirty = 0;
        cache.blocks->set = b % cache.numSets;
    }
    return;
}

void updateLRU(int i) {
    // LRU = 0
    cache.blocks[i].lruLabel = 0;
    int startBlock = i - (i % cache.blocksPerSet);

    for (int block = 0; block < cache.blocksPerSet; block++) {
        if (cache.blocks[startBlock + block].valid == 1 && (block != i)) {
            cache.blocks[startBlock + block].lruLabel += 1;
        }
    }
}

int findVictim(int set) {
    int max_lru = -1;
    int victim = -1;
    // iterate through each block in the set
    for (int i = cache.blocksPerSet * set; i < cache.blocksPerSet * set + cache.blocksPerSet; i++) {
        if (cache.blocks[i].lruLabel > max_lru) {
            max_lru = cache.blocks[i].lruLabel;
            victim = i;
        }
    }
    return victim;
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data) {
    //printCache();
    //printf("%d Main memory words accessed: %d\n", addr, get_num_mem_accesses());
    // Break up address
    int numBlockOffsetBits = (int)log2((double)cache.blockSize);
    int numSetBits = (int)log2((double)cache.numSets);
    int numTagBits = 16 - numBlockOffsetBits - numSetBits;

    int blockOffset = (addr) & ((1 << numBlockOffsetBits) - 1); // isolate last 2^n-1 bit
    int set = (addr >> numBlockOffsetBits) & ((1 << numSetBits) - 1);
    int tag = (addr >> (numBlockOffsetBits + numSetBits)) & ((1 << numTagBits) - 1);
    
    /************************* HIT (check if it is a hit) *************************/
    for (int i = cache.blocksPerSet * set; i < cache.blocksPerSet * set + cache.blocksPerSet; i++) {
        if (cache.blocks[i].valid && tag == cache.blocks[i].tag) { // HIT (no need to access memory)
            // printf("hit! %d\n", addr);
            // printf("cache block: %d, valid: %d, tag: %d\n", i, cache.blocks[i].valid, cache.blocks[i].tag);
            // printf("my current tag: %d\n", tag);
            hits += 1;
            updateLRU(i); // update LRU
            // [ SW ]
            if (write_flag) {
                printAction(addr, 1, processorToCache);
                cache.blocks[i].data[blockOffset] = write_data; // update cache
                cache.blocks[i].dirty = 1;                      // make dirty
                return (cache.blocks[i].data[blockOffset]);
            }
            // [ not SW ]
            printAction(addr, 1, cacheToProcessor);
            return (cache.blocks[i].data[blockOffset]);
        }
    }

    /************************* MISS (bring block from mem to cache) *************************/
    blockStruct newBlock;
    newBlock.dirty = write_flag == 1 ? 1 : 0;
    newBlock.set = set;
    newBlock.tag = tag;
    newBlock.valid = 1;
    newBlock.beginAt = addr - (addr % cache.blockSize);
    newBlock.lruLabel = 0;

    misses += 1;
    int start = addr - (addr % cache.blockSize);

    for (int i = 0; i < cache.blockSize; i++) {
        newBlock.data[i] = mem_access(start + i, 0, 0);
        //printf("Data at %d: %d\n", i, newBlock.data[i]);
    }

    // Insert new block at first available spot in set
    for (int i = cache.blocksPerSet * set; i < cache.blocksPerSet * set + cache.blocksPerSet; i++) {
        if (cache.blocks[i].valid == 0) {   // SET HAS ROOM
            printAction(start, cache.blockSize, memoryToCache);
            cache.blocks[i] = newBlock;     // insert newBlock
            updateLRU(i);    // update LRU

            if (write_flag == 1) {   // sw
                printAction(start + blockOffset, 1, processorToCache);
                cache.blocks[i].data[blockOffset] = write_data; // update cache
                return (cache.blocks[i].data[blockOffset]);
            }
            else {
                printAction(start + blockOffset, 1, cacheToProcessor);
                return (cache.blocks[i].data[blockOffset]);
            }
            
        }
    }

    // SET IS FULL (evict)
    int victim = findVictim(set);
    // printf("victim: %d\n", victim);
    if (cache.blocks[victim].dirty == 1) {  // Write back to memory
        printAction(cache.blocks[victim].beginAt, cache.blockSize, cacheToMemory);
        writebacks += 1;
        for (int i = 0; i < cache.blockSize; i++) {
            mem_access(i + cache.blocks[victim].beginAt, 1, cache.blocks[victim].data[i]);
            // printf("data at old block [%d]: %d\n", i + cache.blocks[victim].beginAt, cache.blocks[victim].data[i]);
        }
    }
    else {
        printAction(cache.blocks[victim].beginAt, cache.blockSize, cacheToNowhere);
    }

    // deal with new block
    printAction(start, cache.blockSize, memoryToCache);
    cache.blocks[victim] = newBlock;

    if (write_flag == 1) {
        printAction(start + blockOffset, 1, processorToCache);
        cache.blocks[victim].data[blockOffset] = write_data; // update cache
    }
    else {
        printAction(start + blockOffset, 1, cacheToProcessor);
    }
    updateLRU(victim);

    return (cache.blocks[victim].data[blockOffset]);
}


/*
 * print end of run statistics like in the spec. This is not required,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(){
    printf("$$$ Main memory words accessed: %d\n", get_num_mem_accesses());
    printf("End of run statistics:\nhits %d, misses %d, writebacks %d\n", hits, misses, writebacks);
    return;
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache()
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}