#include <stdint.h>

void test_1 (int i)
{
  int32_t *ptr = malloc (3.14); /* { dg-line test_1 } */
  free (ptr);

  /* { dg-warning "use of floating point arithmetic inside the size argument is dangerous" "note" { target *-*-* } test_1 } */
  /* { dg-message "one or more operands" "note" { target *-*-* } test_1 } */
}

void test_2 (int i)
{
  float f;
  if (i == 0)
    f = 4.0;
  else
    f = 4.5;
  int32_t *ptr = malloc (f); /* { dg-line test_2 } */
  free (ptr);

  /* { dg-warning "use of floating point arithmetic inside the size argument is dangerous" "note" { target *-*-* } test_2 } */
  /* { dg-message "one or more operands" "note" { target *-*-* } test_2 } */
}
