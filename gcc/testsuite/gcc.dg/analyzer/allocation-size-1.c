#include <stdlib.h>

/* Tests with constant buffer sizes.  */

void test_1 (void)
{
  short *ptr = malloc (21 * sizeof(short));
  free (ptr);
}

void test_2 (void)
{
  int *ptr = malloc (21 * sizeof (short)); /* { dg-line malloc2 } */
  free (ptr);

  /* { dg-warning "" "" { target *-*-* } malloc2 } */
  /* { dg-message "" "" { target *-*-* } malloc2 } */
}

void test_3 (void)
{
  void *ptr = malloc (21 * sizeof (short));
  short *sptr = (short *)ptr;
  free (sptr);
}

void test_4 (void)
{
  void *ptr = malloc (21 * sizeof (short)); /* { dg-message } */
  int *iptr = (int *)ptr; /* { dg-line assign } */
  free (iptr);

  /* { dg-warning "" "" { target *-*-* } assign } */
  /* { dg-message "" "" { target *-*-* } assign } */
}

struct s {
  int i;
};

void test_5 (void)
{
  struct s *ptr = malloc (5 * sizeof (struct s));
  free (ptr);
}

void test_6 (void)
{
  long *ptr = malloc (5 * sizeof (struct s));  /* { dg-line malloc6 } */
  free (ptr);

  /* { dg-warning "" "" { target *-*-* } malloc6 } */
  /* { dg-message "" "" { target *-*-* } malloc6 } */
}

void test_7 (void)
{
  char buf[2];
  int *ptr = (int *)buf; /* { dg-line malloc7 } */

  /* { dg-warning "" "" { target *-*-* } malloc7 } */
  /* { dg-message "" "" { target *-*-* } malloc7 } */
}