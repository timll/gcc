#include <stddef.h>
#include <stdlib.h>

void *create_buffer(int n)
{
  return malloc(n);
}

void test_1(void) 
{
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
