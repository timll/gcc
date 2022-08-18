#include <string.h>
#include "analyzer-decls.h"

char *test_1 (void)
{
  char str[] = "Hello";
  char buf[6];
  char *result = strcpy (buf, str);
  __analyzer_eval (result == buf); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[0] == 'H'); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[1] == 'e'); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[2] == 'l'); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[3] == 'l'); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[4] == 'o'); /* { dg-warning "TRUE" } */
  __analyzer_eval (buf[5] == 0); /* { dg-warning "TRUE" } */
  return result;
}
