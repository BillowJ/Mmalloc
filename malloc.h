
#ifndef _MALLOC_H
#define _MALLOC_H 

#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <mcheck.h>




/* struct to hold block metadata
 * size represents the free block's size in bytes.
 * next points to next free block.
 */
typedef struct block_info
{
   int size;
   struct block_info *next;
}block_info;

/*mutex for global heap.*/
pthread_mutex_t global_heap_mutex = PTHREAD_MUTEX_INITIALIZER;

/* mutex for updating the stats variables */
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

/* mutex for Bin_a*/
pthread_mutex_t global_Bin_a = PTHREAD_MUTEX_INITIALIZER;

// total arena size allocated in bytes.
unsigned long total_arena_size_allocated = 0;

// total size allocated through mmap system call.
unsigned long total_mmap_size_allocated = 0;

// total number of blocks in heap.~
unsigned long total_number_of_blocks = 0;

// total number of allocation done. It is count of number of times malloc called
unsigned long total_allocation_request = 0;

// total free requests made.
unsigned long total_free_request = 0;

// total number of free blocks available (of all threads.)
unsigned long total_free_blocks = 0;


/* Four bins of size 8 byte, 64 byte, 512 byte and every thing else greater
 *  than 512 bytes.
 *
 * Each thread will have its own bin and storage arena.
 * Iniially all the bins are empty. The list gets build up on successive free
 * calls after malloc.
 */
// bin_a仅用于小内存块的空余内存块存储
  static block_info *bin_a     = NULL; 
__thread block_info *bin_8     = NULL;
__thread block_info *bin_16    = NULL;
__thread block_info *bin_32    = NULL;
__thread block_info *bin_64    = NULL;
__thread block_info *bin_128   = NULL;
__thread block_info *bin_256   = NULL;
__thread block_info *bin_512   = NULL;
__thread block_info *bin_large = NULL;


/*
 * A pointer to heap memory upto which the heap addresses are assigned to
 * the threads of process. Addresses beyond this upto heap end are available
 * for future thread of more memory expansion to threads.
 */
void *heap_used_memory_end = NULL;

/*
 *  pointer to a location from which hepa memory allocated to thread has not
 *  been
 *  used yet.
 */
__thread void *thread_unused_heap_start = NULL;


/*
 * End point for the thread heap memory area.
 */
__thread void *thread_heap_end = NULL;



/*
  Aligns pointer to 8 byte address.
  params : pointer to align.
  returns: a pointer (aligned to 8 bytes)
 */
void * align8(void *x);

/*
 * Allocate memory from heap area. For memory request of sizes < 512, chunks are
 * allocated from heap.
 * params : size.
 * returns: pointer to allocated area.
 */
void * heap_allocate(size_t size);

/*
 * Finds best fit block from bin_large. On memory request > 512, first
 * it is checked if any of large free memory chunks fits to request.
 *
 * params: size of block to allocate.
 * returns: pointer to best fitting block, NULL on failure.
 */
void * find_best_fit_from_bin_large(size_t size);

/*
 * maps new memory address using mmap system call for size request > 512 bytes.
 * params: requested size in bytes.
 * returns: pointer to block allocated., NULL on failure.
 */
void * mmap_new_memory(size_t size);

/*
 * Allocates the memory.
 */
void* Mmalloc(size_t);


/*
 * Free up the memory allocated at pointer p. It appends the block into free
 * list.
 * params: address to pointer to be freed.
 * returns: NONE.
 */
void Mfree(void *p);


/*
 * Prints malloc stats like number of free blocks, total number of memory
 * allocated.
 */
void malloc_stats();

void* allocLarge(size_t);
void* allocLargeByMmap(size_t);

void* findBestFitFromLarge(size_t);

// Operation About Bin_a 
void* findBestBlockFromBinA(size_t size);
bool addBlockToBinA(void*);
bool deleteBlockFromBinA(void*);


block_info** InitListHelper(size_t size){
   switch (size)
   {
      case 8   : return &bin_8;
      case 16  : return &bin_16;
      case 32  : return &bin_32;
      case 64  : return &bin_64;
      case 128 : return &bin_128;
      case 256 : return &bin_256;
      case 512 : return &bin_512;
      default  : return &bin_large;
   }
}

// FIXME : 初始化有问题
size_t InitGlobalList(void* ptr){
   size_t allSize = 0;
   size_t curSize = 8;
   // 初始化除了bin_large 和 bin_a 的其余链表
   for(int idx = 0; idx < 7; idx++){
      // 获取对应的链表
      block_info** curBinPtr = InitListHelper(curSize);
      block_info* cur = *curBinPtr; 
      size_t offset = sizeof(block_info) + curSize;
      // 预先分配三块内存
      for(int num = 0; num < 3; num++){
         if (cur == nullptr)
         {
            cur = (block_info*)(ptr + allSize);
            *curBinPtr = cur;
         }

         cur -> size = curSize;
         // size_t offset = sizeof(block_info) + curSize;
         cur -> next = num == 2 ? NULL : (block_info*)(ptr + offset);
         allSize += offset;
         cur = (block_info*)(ptr + allSize);
      }
      // 8 -> 16 -> 32 -> 64 -> 128 -> ..
      curSize <<= 1;
   }
   return allSize;
}

namespace tcmalloc{

// typedef struct threadCacheInfo
// {
//    struct threadCacheInfo* next;
//    size_t size;
// }threadCacheInfo;

#define ListSize 7

class threadCache{
public:
   threadCache();
   static pthread_key_t heapKey;

   static threadCache* getCache();
   static threadCache* createCache();
   static void destroyThreadCache(void* ptr);
   static void deleteCache(threadCache*);
   static bool hasInit();
   void cleanUp();

   void Init(pthread_t tid);
   static void InitTSD();

   int getTheListIndex(size_t size);
   void* allocate(size_t size);
   void deallocate(void* ptr);
   block_info* freeList[ListSize];

private:
   block_info* FromInitList(size_t);
   block_info* FromGlobalList(size_t);
   void* FromHeap(size_t);

   
   static threadCache* mThreadCache;
   static bool isInit;
   size_t size_;

public:
   threadCache* prev;
   threadCache* next;
   pthread_t tid_;

};

bool threadCache::isInit = false;
pthread_key_t threadCache::heapKey;
threadCache* threadCache::mThreadCache = nullptr;
void* Mmallocc(size_t);
void Mfreee(size_t);
bool threadCache::hasInit(){
   return isInit;
}

} // namespace tcmalloc




#endif
