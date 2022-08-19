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

char *test_2 (void)
{
  char str[] = "Hello";
  char buf[6];
  char *result = strcpy (buf, str);
  __analyzer_eval (result == buf); /* { dg-warning "TRUE" } */

  /* result[0] describes as
      svalue: ‘unaryop_svalue (nop_expr,
                sub_svalue (,
                  constant_svalue(‘char[6]’, "Hello"),
                    cast_region(
                      decl_region(
                        frame_region(‘test_2’, index: 0, depth: 1),
                        ‘char[6]’,
                        ‘buf’),
                      ‘char’)))’.  */
  __analyzer_eval (result[0] == 'H'); /* { dg-bogus "UNKNOWN" "warning" { xfail *-*-* } } */

  /* result[1] describes as svalue: ‘constant_svalue(‘int’, 101)’.  */
  __analyzer_eval (result[1] == 'e'); /* { dg-warning "TRUE" } */
  __analyzer_eval (result[2] == 'l'); /* { dg-warning "TRUE" } */
  __analyzer_eval (result[3] == 'l'); /* { dg-warning "TRUE" } */
  __analyzer_eval (result[4] == 'o'); /* { dg-warning "TRUE" } */
  __analyzer_eval (result[5] == 0); /* { dg-warning "TRUE" } */
  return result;
}
