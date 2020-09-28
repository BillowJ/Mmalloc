#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>

#include "malloc.h"

/*
void* threadFunc(void* arg){
  void* p1 = Mmalloc(18);
  void* p2 = Mmalloc(513);
  printf("thread id: %lu", pthread_self());
  printf("thread malloc end \n");
  Mfree(p1);
  Mfree(p2);
  return nullptr;
}
*/

int main(int argc, char **argv)
{
  pthread_t tid;
  size_t size = 10;
  size_t size0 = 100;
  size_t size1 = 1000;
  void *mem = Mmalloc(size);
  void *mem0 = Mmalloc(size0);
  void *mem1 = Mmalloc(size1);
  printf("before free the memory:\n");
  printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  printf("Successfully malloc'd %zu bytes at addr %p\n", size0, mem0);
  printf("Successfully malloc'd %zu bytes at addr %p\n", size1, mem1);
  sleep(1);
  Mfree(mem);
  Mfree(mem0);
  Mfree(mem1);
  return 0;
}
