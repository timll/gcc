#include <stdlib.h>

/* Tests with constant buffer sizes */

void test_1 (void)
{
  short *ptr = malloc (21 * sizeof(short));
  free (ptr);
}

void test_2 (void)
{
  int *ptr = malloc (21 * sizeof (short)); /* { dg-line malloc } */
  free (ptr);

  /* { dg-warning "Allocated buffer size is not a multiple of the pointee's size" "" { target *-*-* } malloc } */
  /* { dg-message "\\(1\\) Casting a 42 byte buffer to 'int \\*' leaves 2 trailing bytes" "" { target *-*-* } malloc } */
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

  /* { dg-warning "Allocated buffer size is not a multiple of the pointee's size" "" { target *-*-* } assign } */
  /* { dg-message "\\(2\\) Casting 'ptr' to 'int \\*' leaves 2 trailing bytes" "" { target *-*-* } assign } */
}
