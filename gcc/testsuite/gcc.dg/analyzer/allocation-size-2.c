#include <stdlib.h>
#include <stdio.h>

/* Tests with symbolic buffer sizes.  */

void test_1 (void)
{
  int n;
  scanf("%i", &n);
  short *ptr = malloc (n * sizeof(short));
  free (ptr);
}

void test_2 (void)
{
  int n;
  scanf("%i", &n);
  int *ptr = malloc (n * sizeof (short)); /* { dg-line malloc } */
  free (ptr);

  /* { dg-warning "" "" { target *-*-* } malloc } */
  /* { dg-message "" "" { target *-*-* } malloc } */
}

void test_3 (void)
{
  int n;
  scanf("%i", &n);
  void *ptr = malloc (n * sizeof (short));
  short *sptr = (short *)ptr;
  free (sptr);
}

void test_4 (void)
{
  int n;
  scanf("%i", &n);
  void *ptr = malloc (n * sizeof (short)); /* { dg-message } */
  int *iptr = (int *)ptr; /* { dg-line assign } */
  free (iptr);

  /* { dg-warning "" "" { target *-*-* } assign } */
  /* { dg-message "" "" { target *-*-* } assign } */
}
