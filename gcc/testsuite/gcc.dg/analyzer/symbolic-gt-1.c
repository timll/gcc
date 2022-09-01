#include "analyzer-decls.h"

/* Test GT_EXPR comparison of symbolic values.  */

void test1 (unsigned i)
{
  unsigned a = i;
  unsigned b = i;
  __analyzer_eval (a > b); /* { dg-warning "FALSE" } */

  a = i + 1;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */

  /* Again i + 4294967295 in gimple, but i - 1 in dynamic execution...  */
  a = i - 1;
  __analyzer_eval (a > b); /* { dg-bogus "TRUE" "" { xfail *-*-* } } */

  a = i * 2;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */

  a = i * i;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */
}

void test2 (int i)
{
  int a = i * i;
  int b = i;
  /* Operand i is left but is an int and thus, possibly negative.  The
    elimination algorithm misses that the other eliminated operand also
    was i, but that is a limitation of the algorithm.  */
  __analyzer_eval (a > b); /* { dg-bogus "UNKNOWN" "" { xfail *-*-* } } */
}

void test3 (int i, int j)
{
  int a = i * j;
  int b = i * j;
  __analyzer_eval (a > b); /* { dg-warning "FALSE" } */
  a = j * i;
  __analyzer_eval (a > b); /* { dg-warning "FALSE" } */

  a = (i * j) + 1;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */
  a = (j * i) + 1;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */

  a = i * (j + 1);
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */
  a = (i + 1) * j;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */

  a = a + b;
  __analyzer_eval (a > b); /* { dg-warning "UNKNOWN" } */
}

void test4 (int i, int j, int k)
{
  int a = (i + j + k + 1) * (j + k) * k;
  int b = (i + j + k) * (j + k) * k;
  __analyzer_eval (a > b); /* { dg-warning "TRUE" } */
}

void test5 (int i, int j, int k)
{
  int a = (i + j + k - 1) * (j + k) * k;
  int b = (i + j + k) * (j + k) * k;
  __analyzer_eval (a > b); /* { dg-warning "UNKNOWN" } */
}
