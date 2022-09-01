#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Tests requiring folding constraints.  */

void test1 (void)
{
  int buf[4];
  for (int i = 0; i <= 4; i++)
    buf[i] = i; /* { dg-warning "" } */
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

/* Avoid folding of memcpy.  */
typedef void * (*memcpy_t) (void *dst, const void *src, size_t n);

static memcpy_t __attribute__((noinline))
get_memcpy (void)
{
  return memcpy;
}

extern int is_valid (void);

int returnChunkSize (void *ptr)
{
  /* If chunk info is valid, return the size of usable memory,
     else, return -1 to indicate an error.  */
  return is_valid () ? sizeof (*ptr) : -1;
}

/* Taken from CWE-787.  */
void test7 (void)
{
  memcpy_t fn = get_memcpy ();

  int destBuf[4];
  int srcBuf[4];
  fn (destBuf, srcBuf, returnChunkSize (destBuf)); /* { dg-line test7 } */

  /* { dg-warning "overread" "warning" { target *-*-* } test7 } */
  /* { dg-warning "overflow" "warning" { target *-*-* } test7 } */
}

/* Test with OOB in an infeasible path.  */

void test8 (int cond)
{
  int buf[4];

  int n;
  if (cond)
    n = 1;
  else
    n = 2;

  int m;
  if (!cond)
    m = 1;
  else
    m = 2;
  
  buf[n * m] = 42;
}

void test9 (int cond)
{
  int buf[4];

  int n;
  if (cond)
    n = 1;
  else
    n = 2;

  int m;
  if (cond)
    m = 2;
  else
    m = 1;

  buf[n * m] = 42;
}

void test10 (int cond)
{
  int buf[4];

  int n;
  if (cond)
    n = 2;
  else
    n = 1;

  int m;
  if (cond)
    m = 2;
  else
    m = 1;

  /* Even though n * m == 4 here, the checker doesn't warn because
     it can't be sure that the combination is an infeasible path.  */
  buf[n * m] = 42;
}
