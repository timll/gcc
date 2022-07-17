#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Wanalyzer-restrict tests for memcpy.  */

/* Avoid folding of memcpy.  */
typedef void * (*memcpy_t) (void *dst, const void *src, size_t n);
  
static memcpy_t __attribute__((noinline))
get_memcpy (void)
{
  return memcpy;
}

/* element_region & decl_region.  */

void test1 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (&buf[2], buf, 2 * sizeof(int32_t));
}

void test2 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (&buf[1], buf, 2 * sizeof(int32_t)); /* { dg-line test2 } */

  /* { dg-warning "" "warning" { target *-*-* } test2 } */
  /* { dg-message "" "note" { target *-*-* } test2 } */
}

/* element_region & element_region.  */

void test3 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (&buf[2], &buf[0], 2 * sizeof(int32_t));
}

void test4 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (&buf[1], &buf[0], 2 * sizeof(int32_t)); /* { dg-line test4 } */

  /* { dg-warning "" "warning" { target *-*-* } test4 } */
  /* { dg-message "" "note" { target *-*-* } test4 } */
}

/* offset_region & decl_region.  */

void test5 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (buf + 2, buf, 2 * sizeof(int32_t));
}

void test6 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (buf + 1, buf, 2 * sizeof(int32_t)); /* { dg-line test6 } */

  /* { dg-warning "" "warning" { target *-*-* } test6 } */
  /* { dg-message "" "note" { target *-*-* } test6 } */
}

/* offset_region & offset_region.  */

void test7 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (buf + 3, buf + 1, 2 * sizeof(int32_t));
}

void test8 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (buf + 3, buf + 2, 2 * sizeof(int32_t)); /* { dg-line test8 } */

  /* { dg-warning "" "warning" { target *-*-* } test8 } */
  /* { dg-message "" "note" { target *-*-* } test8 } */
}

/* element_region & heap_allocated_region.  */

void test9 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t *buf = malloc (4 * sizeof (int32_t));
  if (!buf)
    return;
  fn (&buf[2], buf, 2 * sizeof(int32_t));
  free (buf);
}

void test10 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t *buf = malloc (4 * sizeof (int32_t));
  if (!buf)
    return;
  fn (&buf[1], buf, 2 * sizeof(int32_t)); /* { dg-line test10 } */
  free (buf);

  /* { dg-warning "" "warning" { target *-*-* } test10 } */
  /* { dg-message "" "note" { target *-*-* } test10 } */
}

/* offset_region & heap_allocated_region.  */

void test11 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t *buf = malloc (4 * sizeof (int32_t));
  if (!buf)
    return;
  fn (buf + 2, buf, 2 * sizeof(int32_t));
  free (buf);
}

void test12 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t *buf = malloc (4 * sizeof (int32_t));
  if (!buf)
    return;
  fn (buf + 1, buf, 2 * sizeof(int32_t)); /* { dg-line test12 } */
  free (buf);

  /* { dg-warning "" "warning" { target *-*-* } test12 } */
  /* { dg-message "" "note" { target *-*-* } test12 } */
}

/* aliased region.  */

void test13 (void)
{
  memcpy_t fn = get_memcpy ();
  
  int32_t buf[4] = {0};
  void *view = buf;
  fn (view, buf, 2 * sizeof(int32_t)); /* { dg-line test13 } */

  /* { dg-warning "" "warning" { target *-*-* } test13 } */
  /* { dg-message "" "note" { target *-*-* } test13 } */
}

void test14 (void)
{
  memcpy_t fn = get_memcpy ();
  
  int32_t buf[4] = {0};
  void *view = buf;
  fn (view + 2 * sizeof(int32_t), buf, 2 * sizeof(int32_t));
}

void test15 (void)
{
  memcpy_t fn = get_memcpy ();
  
  int32_t buf[4] = {0};
  void *view = buf;
  fn (view + sizeof(int32_t), buf, 2 * sizeof(int32_t)); /* { dg-line test15 } */

  /* { dg-warning "" "warning" { target *-*-* } test15 } */
  /* { dg-message "" "note" { target *-*-* } test15 } */
}


/* field_region.  */

struct my_struct {
  int32_t i[4];
};

void test16 (void)
{
  memcpy_t fn = get_memcpy ();

  struct my_struct s;
  for (int i = 0; i < 4; i++)
    s.i[i] = i;
  memcpy (s.i + 2, s.i, 2 * sizeof (int32_t));
}

void test17 (void)
{
  memcpy_t fn = get_memcpy ();

  struct my_struct s;
  for (int i = 0; i < 4; i++)
    s.i[i] = i;
  memcpy (s.i + 1, s.i, 2 * sizeof (int32_t)); /* { dg-line test17 } */

  /* { dg-warning "" "warning" { target *-*-* } test17 } */
  /* { dg-message "" "note" { target *-*-* } test17 } */
}
