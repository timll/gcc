#include <stdio.h>
#include <stdlib.h>

/* Tests requiring folding constraints.  */

int *test1 ()
{
  int *buf = malloc (4 * sizeof (int));
  if (!buf)
    return NULL;
  for (int i = 0; i <= 4; i++)
    buf[i] = i; /* { dg-warning "" } */
  return buf;
}

void test2 (int i)
{
  char buf[4] = {0};

  if (i == 5)
    printf ("%c", buf[i]); /* { dg-warning "" } */
}

void test3 (int i, int j)
{
  char buf[7] = {0};
  if (i <= 4)
    if (j == 2)
      printf ("%c", buf[i * j]); /* { dg-warning "" } */
}