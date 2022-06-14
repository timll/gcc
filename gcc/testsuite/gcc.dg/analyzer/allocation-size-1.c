#include <stdlib.h>

void test_1 (void)
{
  short *ptr = malloc (42);
}

void test_2 (void)
{
  int *ptr = malloc (42);
}

void test_3 (void)
{
  void *ptr = malloc (42);
  short *sptr = (short *)ptr;
}

void test_4 (void)
{
  void *ptr = malloc (42);
  int *sptr = (int *)ptr;
}
