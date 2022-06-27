#include <stdlib.h>
#include <stdio.h>

/* Tests with constant buffer sizes.  */

void test_1 (void)
{
  short *ptr = malloc (21 * sizeof (short));
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
  int *iptr = (int *)ptr; /* { dg-line assign4 } */
  free (iptr);

  /* { dg-warning "" "" { target *-*-* } assign4 } */
  /* { dg-message "" "" { target *-*-* } assign4 } */
}

void test_5 (void)
{
  int user_input;
  scanf("%i", &user_input);
  int n;
  if (user_input == 0)
    n = 21 * sizeof (short);
  else
    n = 42 * sizeof (short);
  void *ptr = malloc (n);
  short *sptr = (short *)ptr;
  free (sptr);
}

void test_6 (void)
{
  int user_input;
  scanf("%i", &user_input);
  int n;
  if (user_input == 0)
    n = 21 * sizeof (short);
  else
    n = 42 * sizeof (short);
  void *ptr = malloc (n); /* { dg-message } */
  int *iptr = (int *)ptr; /* { dg-line assign6 } */
  free (iptr);

  /* { dg-warning "" "" { target *-*-* } assign6 } */
  /* { dg-message "" "" { target *-*-* } assign6 } */
}

void test_7 (void)
{
  int user_input;
  scanf("%i", &user_input);
  int n;
  if (user_input == 0)
    n = 1;
  else if (user_input == 2)
    n = 5;
  else
    n = 7;
  /* n is an unknown_svalue at this point.  */
  void *ptr = malloc (n);
  int *iptr = (int *)ptr;
  free (iptr);
}

void *create_buffer(int n)
{
  return malloc(n);
}

void test_8(void) 
{
  int *buf = create_buffer(4 * sizeof (int));
  free (buf);
}

void test_9(void) 
{
  // FIXME
  int *buf = create_buffer(42); /* { dg-warning "" "" { xfail *-*-* } } */
  free (buf);
}
