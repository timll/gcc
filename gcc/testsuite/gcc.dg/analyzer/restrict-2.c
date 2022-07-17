/* { dg-additional-options -Wno-restrict } */

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
