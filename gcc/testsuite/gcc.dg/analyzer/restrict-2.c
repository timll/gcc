/* { dg-additional-options -Wno-restrict } */
#include <stdint.h>
#include <string.h>

/* Wanalyzer-restrict tests for arbitrary functions with restrict-qualifed
   parameters.  Wrestrict is disabled because the checker only warns if
   Wrestrict missed the warning.  */

void __attribute__ ((noinline)) fun (void * restrict a, void * b)
{
  return;
}

void __attribute__ ((noinline)) fun2 (void * restrict a, int b)
{
  return;
}

void test1 (void)
{
  int buf[4];
  fun (buf, buf); /* { dg-line test1 } */

  /* { dg-warning "" "warning" { target *-*-* } test1 } */
  /* { dg-message "" "note" { target *-*-* } test1 } */
}

void test2 (void)
{
  int buf[4];
  int *alias = buf;
  fun (alias, buf); /* { dg-line test2 } */

  /* { dg-warning "" "warning" { target *-*-* } test2 } */
  /* { dg-message "" "note" { target *-*-* } test2 } */
}

void test3 (void)
{
  int buf[4];
  int *alias = buf;
  fun (alias + 1, buf);
}

void test4 (void)
{
  int buf[1] = {0};
  fun2 (buf, *buf);
}

void test5 (void)
{
  int16_t buf[2];
  int32_t *view = (int32_t *) buf;
  fun (buf, view); /* { dg-line test5 } */

  /* { dg-warning "" "warning" { target *-*-* } test5 } */
  /* { dg-message "" "note" { target *-*-* } test5 } */
}

struct my_struct {
  void *a;
  void *b;
};

void test6 (void) {
  int a[4];
  memset (a, 0, sizeof (a));
  int b[4];
  memset (b, 0, sizeof (b));

  struct my_struct s;
  s.a = a;
  s.b = b;
  fun (s.a, s.b);
}

void test7 (void) {
  int a[4];
  memset (a, 0, sizeof (a));

  struct my_struct s;
  s.a = a;
  s.b = a;
  fun (s.a, s.b); /* { dg-line test7 } */

  /* { dg-warning "" "warning" { target *-*-* } test7 } */
  /* { dg-message "" "note" { target *-*-* } test7 } */
}
