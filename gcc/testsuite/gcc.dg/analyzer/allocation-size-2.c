#include <stdlib.h>
#include <stdio.h>

/* Tests with symbolic buffer sizes */

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

  /* { dg-warning "Allocated buffer size is not a multiple of the pointee's size" "" { target *-*-* } malloc } */
  /* { dg-message "\\(1\\) Allocation is incompatible with 'int \\*'" "" { target *-*-* } malloc } */
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

  /* { dg-warning "Allocated buffer size is not a multiple of the pointee's size" "" { target *-*-* } assign } */
  /* { dg-message "\\(2\\) 'ptr' is incompatible with 'int \\*'; either the allocated size at \\(1\\)" "" { target *-*-* } assign } */
}
