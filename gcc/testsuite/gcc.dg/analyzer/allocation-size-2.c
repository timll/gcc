#include <stdlib.h>
#include <stdio.h>

/* Tests with symbolic buffer sizes.  */

void test_1 (void)
{  
  int n;
  scanf("%i", &n);
  short *ptr = malloc (n * sizeof (short));
  free (ptr);
}

void test_2 (void)
{
  int n;
  scanf("%i", &n);
  int *ptr = malloc (n * sizeof (short)); /* { dg-line malloc2 } */
  free (ptr);

  /* { dg-warning "" "" { target *-*-* } malloc2 } */
  /* { dg-message "" "" { target *-*-* } malloc2 } */
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
    n = 3 * user_input * sizeof (short);
  else
    n = 5 * user_input * sizeof (short);
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
    n = user_input;
  else if (user_input == 2)
    n = user_input * 3;
  else
    n = user_input * 5;
  /* n is an unknown_svalue at this point.  */
  void *ptr = malloc (n);
  int *iptr = (int *)ptr;
  free (iptr);
}

void *create_buffer(int n)
{
  return malloc(n);
}

void test_7(void) 
{
  int n;
  scanf("%i", &n);
  int *buf = create_buffer(n * sizeof (int));
  free (buf);
}

void test_8(void) 
{
  int n;
  scanf("%i", &n);
  // FIXME
  int *buf = create_buffer(n * sizeof(short)); /* { dg-warning "" "" { xfail *-*-* } } */
  free (buf);
}

void test_9 (void)
{
  int n;
  scanf("%i", &n);
  /* n is a conjured_svalue.  */
  void *ptr = malloc (n); /* { dg-message } */
  int *iptr = (int *)ptr; /* { dg-line assign9 } */
  free (iptr);

  /* { dg-warning "" "" { target *-*-* } assign9 } */
  /* { dg-message "" "" { target *-*-* } assign9 } */
}

void test_11 (void)
{
  int n;
  scanf("%i", &n);
  void *ptr = malloc (n);
  if (n == 4)
    {
      /* n is a conjured_svalue but guarded.  */
      int *iptr = (int *)ptr;
      free (iptr);
    }
  else
    free (ptr);
}
