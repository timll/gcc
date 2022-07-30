#include <stdint.h>

int32_t *
foo (int32_t x)
{
  return __builtin_calloc (x * 1.1, 1); /* { dg-message "operand '\[0-9efd\\.\\+\]*?' is of type 'double'" } */
}
