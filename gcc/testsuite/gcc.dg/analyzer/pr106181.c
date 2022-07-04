#include <stdint.h>

int32_t *
foo (int32_t x)
{
  return __builtin_calloc (x * 1.1, 1); /* { dg-line calloc } */
  /* { dg-warning "allocated buffer size is not a multiple of the pointee's size \\\[CWE-131\\\]" "warning" { target *-*-* } calloc } */
  /* { dg-message "'int32_t \\*' (\\\{aka 'int \\*'\\\})? here; 'sizeof \\(int32_t (\\\{aka int\\\})?\\)' is '4'" "note" { target *-*-* } calloc } */
}
