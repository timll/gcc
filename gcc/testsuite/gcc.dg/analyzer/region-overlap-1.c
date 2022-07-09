#include <string.h>
#include <stdlib.h>
#include <stdint.h>

typedef void * (*memcpy_t) (void *dst, const void *src, size_t n);
  
static memcpy_t __attribute__((noinline))
get_memcpy (void)
{
  return memcpy;
}

void test1 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  fn (&buf[2], &buf[0], 2 * sizeof(int32_t));
}

void test2 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  fn (buf + 2, buf, 2 * sizeof(int32_t));
}

void test3 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  fn (&buf[1], &buf[0], 2 * sizeof(int32_t)); /* { dg-line test3 } */

  /* { dg-warning "" "warning" { target *-*-* } test3 } */
  /* { dg-message "" "note" { target *-*-* } test3 } */
}

void test4 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  fn (buf + 1, buf, 2 * sizeof(int32_t)); /* { dg-line test4 } */

  /* { dg-warning "" "warning" { target *-*-* } test4 } */
  /* { dg-message "" "note" { target *-*-* } test4 } */
}
