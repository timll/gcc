#include <stdlib.h>
#include <stdio.h>

/* Tests related to structs.  */

typedef struct a {
  short s;
} a;

int *test_1 (void)
{
  a A;
  A.s = 1;
  int *ptr = (int *) &A; /* { dg-line assign1 } */
  return ptr;

  /* { dg-warning "" "" { target *-*-* } assign1 } */
  /* { dg-message "" "" { target *-*-* } assign1 } */
}

int *test2 (void)
{
  char arr[4];
  int *ptr = (int *)arr;
  return ptr;
}

int *test3 (void)
{
  char arr[2];
  int *ptr = (int *)arr; /* { dg-line assign3 } */
  return ptr;

  /* { dg-warning "" "" { target *-*-* } assign3 } */
  /* { dg-message "" "" { target *-*-* } assign3 } */
}

int main() {
  return 0;
}