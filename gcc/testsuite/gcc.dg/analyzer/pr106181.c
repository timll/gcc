#include <stdint.h>

void *
foo (int x)
{
  return __builtin_calloc (x * 1.1, 1); /* { dg-line calloc } */

  /* { dg-warning "use of floating point arithmetic inside the size argument might yield unexpected results" "warning" { target *-*-* } calloc } */
  /* { dg-message "operand '\(\\d|e|f|\\.|\\+|\)+' is of type 'double'" "note" { target *-*-* } calloc } */
  /* { dg-message "only use operands of a type that represents whole numbers inside the size argument" "note" { target *-*-* } calloc } */
}
