#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* Flow warnings */

void *create_buffer(int n)
{
  return malloc(n);
}

void test_1(void) 
{
  // FIXME
  int *buf = create_buffer(42); /* { dg-warning "" "" { xfail *-*-* } } */
  free (buf);
}

void test_2(void) 
{
  void *buf = create_buffer(42); /* { dg-message } */
  int *ibuf = buf; /* { dg-line assign2 } */
  free (ibuf);

  /* { dg-warning "" "" { target *-*-* } assign2 } */
  /* { dg-message "" "" { target *-*-* } assign2 } */
}

void test_3(void)
{
  void *buf = malloc(42); /* { dg-message } */
  if (buf != NULL) /* { dg-message } */
    {
      int *ibuf = buf; /* { dg-line assign3 } */
      free (ibuf);
    }

  /* { dg-warning "" "" { target *-*-* } assign3 } */
  /* { dg-message "" "" { target *-*-* } assign3 } */
}

void test_4(void)
{
  int n;
  scanf("%i", &n);

  int size;
  if (n == 0)
    size = 1;
  else if (n == 1)
    size = 10;
  else
    size = 20;

  int *buf = malloc(size); // Size should be 'unknown' at this point
  free (buf);
}

void test_5(void)
{
  int n;
  scanf("%i", &n);

  int size;
  if (n == 0)
    size = 2;
  else
    size = 10;

  short *buf = malloc(size); // Size should be widened to 2 and 10, both fit
  free (buf);
}


void test_6(void)
{
  int n;
  scanf("%i", &n);

  int size;
  if (n == 0)
    size = 1;
  else
    size = 10;

  short *buf = malloc(size); /* { dg-line malloc6 } */
  free (buf);
  

  /* { dg-warning "" "" { target *-*-* } malloc6 } */
  /* { dg-message "" "" { target *-*-* } malloc6 } */
}
