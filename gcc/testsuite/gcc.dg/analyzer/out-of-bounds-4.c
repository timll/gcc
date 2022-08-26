#include <stdio.h>

void test1 (int i)
{
  char buf[4];

  if (i == 5)
    printf ("%c", buf[i]);
}

void test2 (int i, int j)
{
  char buf[7];
  if (i <= 4)
    if (j == 2)
      printf ("%c", buf[i * j]);
}