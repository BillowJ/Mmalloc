#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <mcheck.h>

#include "malloc.h"

using namespace global;

int main(int argc, char **argv)
{
  pthread_t tid;
  size_t size = 10;
  size_t size0 = 100;
  size_t size1 = 1000;
  void *mem = tcmalloc::Mmalloc(size);
  void *mem0 = tcmalloc::Mmalloc(size0);
  void *mem1 = tcmalloc::Mmalloc(size1);
  printf("before free the memory:\n");
  printf("Successfully malloc'd %zu bytes at addr %p\n", size, mem);
  printf("Successfully malloc'd %zu bytes at addr %p\n", size0, mem0);
  printf("Successfully malloc'd %zu bytes at addr %p\n", size1, mem1);
  sleep(1);
  tcmalloc::Mfree(mem);
  tcmalloc::Mfree(mem0);
  tcmalloc::Mfree(mem1);
  return 0;
}
