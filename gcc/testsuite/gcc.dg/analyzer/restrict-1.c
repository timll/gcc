#define _GNU_SOURCE
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

static memcpy_t __attribute__((noinline))
get_mempcpy (void)
{
  return mempcpy;
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

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test2 } */
  /* { dg-message "copying 8 bytes from '&buf' to '&buf\\\[1\\\]' overlaps by 4 bytes" "note" { target *-*-* } test2 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test2 } */
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

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test4 } */
  /* { dg-message "copying 8 bytes from '&buf\\\[0\\\]' to '&buf\\\[1\\\]' overlaps by 4 bytes" "note" { target *-*-* } test4 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test4 } */
}

/* offset_region & decl_region.  */

void test5 (void)
{
  memcpy_t fn = get_mempcpy ();

  int32_t buf[4] = {0};
  fn (buf + 2, buf, 2 * sizeof(int32_t));
}

void test6 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4] = {0};
  fn (buf + 1, buf, 2 * sizeof(int32_t)); /* { dg-line test6 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test6 } */
  /* { dg-message "copying 8 bytes from '&buf' to '&buf \\\+ 4' overlaps by 4 bytes" "note" { target *-*-* } test6 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test6 } */
}

/* offset_region & offset_region.  */

void test7 (void)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[5] = {0};
  fn (buf + 3, buf + 1, 2 * sizeof(int32_t));
}

void test8 (void)
{
  memcpy_t fn = get_mempcpy ();

  int32_t buf[4] = {0};
  fn (buf + 3, buf + 2, 2 * sizeof(int32_t)); /* { dg-line test8 } */

  /* { dg-warning "calling 'mempcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test8 } */
  /* { dg-message "copying 8 bytes from '&buf \\\+ 8' to '&buf \\\+ 12' overlaps by 4 bytes" "note" { target *-*-* } test8 } */
  /* { dg-message "use 'memmove' instead of 'mempcpy' with overlapping buffers" "note" { target *-*-* } test8 } */
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

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test10 } */
  /* { dg-message "copying 8 bytes from 'buf' to 'buf \\\+ 4' overlaps by 4 bytes" "note" { target *-*-* } test10 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test10 } */
}

/* offset_region & heap_allocated_region.  */

void test11 (void)
{
  memcpy_t fn = get_mempcpy ();

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

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test12 } */
  /* { dg-message "copying 8 bytes from 'buf' to 'buf \\\+ 4' overlaps by 4 bytes" "note" { target *-*-* } test12 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test12 } */
}

/* aliased region.  */

void test13 (void)
{
  memcpy_t fn = get_memcpy ();
  
  int32_t buf[4] = {0};
  void *view = buf;
  fn (view, buf, 2 * sizeof(int32_t)); /* { dg-line test13 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test13 } */
  /* { dg-message "copying 8 bytes from '&buf' to 'view' overlaps by 8 bytes" "note" { target *-*-* } test13 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test13 } */
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

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test15 } */
  /* { dg-message "copying 8 bytes from '&buf' to 'view \\\+ 4' overlaps by 4 bytes" "note" { target *-*-* } test15 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test15 } */
}

void test16 (void)
{
  memcpy_t fn = get_mempcpy ();

  int16_t buf[2];
  int32_t *view = (int32_t *) buf;
  fn (view, buf, sizeof (int32_t)); /* { dg-line test16 } */

  /* { dg-warning "calling 'mempcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test16 } */
  /* { dg-message "copying 4 bytes from '&buf' to 'view' overlaps by 4 bytes" "note" { target *-*-* } test16 } */
  /* { dg-message "use 'memmove' instead of 'mempcpy' with overlapping buffers" "note" { target *-*-* } test16 } */
}

void test17 (void)
{
  memcpy_t fn = get_memcpy ();

  int16_t buf[4];
  int32_t *view = (int32_t *) buf;
  fn (&view[1], buf, sizeof (int32_t));
}

/* field_region.  */

struct my_struct {
  int32_t i[4];
};

void test18 (void)
{
  memcpy_t fn = get_memcpy ();

  struct my_struct s;
  for (int i = 0; i < 4; i++)
    s.i[i] = i;
  fn (s.i + 2, s.i, 2 * sizeof (int32_t));
}

void test19 (void)
{
  memcpy_t fn = get_memcpy ();

  struct my_struct s;
  for (int i = 0; i < 4; i++)
    s.i[i] = i;
  fn (s.i + 1, s.i, 2 * sizeof (int32_t)); /* { dg-line test19 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test19 } */
  /* { dg-message "copying 8 bytes from '&s.i' to '&s.i \\\+ 4' overlaps by 4 bytes" "note" { target *-*-* } test19 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test19 } */
}

void test20 (int cond)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  int32_t n;
  if (cond)
    n = 2 * sizeof (int32_t);
  else
    n = 3 * sizeof (int32_t);
  fn (buf + 2, buf, n); /* { dg-line test20 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test20 } */
  /* { dg-message "copying 12 bytes from '&buf' to '&buf \\\+ 8' overlaps by 4 bytes" "note" { target *-*-* } test20 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test20 } */
}

struct nested_struct {
  struct my_struct *ptr;
};

void test21 (void)
{
  memcpy_t fn = get_memcpy ();

  struct my_struct first;
  memset (first.i, 0, sizeof (first.i));
  struct my_struct second;
  memset (second.i, 0, sizeof (second.i));

  struct my_struct arr[2];
  arr[0] = first;
  arr[1] = second;
  struct nested_struct ns;
  ns.ptr = arr;

  fn (&ns.ptr[0], &ns.ptr[1], 3 * sizeof (int32_t));
  fn (ns.ptr, ((char *) ns.ptr) + 2 * sizeof (int32_t), 2 * sizeof (int32_t));
  fn (ns.ptr, ((char *) ns.ptr) + 2 * sizeof (int32_t), 3 * sizeof (int32_t)); /* { dg-line test21a } */
  fn (ns.ptr[0].i, ns.ptr[1].i, 3 * sizeof (int32_t));
  fn (ns.ptr[0].i, ns.ptr[0].i + 2, 2 * sizeof (int32_t));
  fn (ns.ptr[0].i, ns.ptr[0].i + 2, 3 * sizeof (int32_t)); /* { dg-line test21b } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test21a } */
  /* { dg-message "copying 12 bytes from" "note" { target *-*-* } test21a } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test21a } */
  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test21b } */
  /* { dg-message "copying 12 bytes from" "note" { target *-*-* } test21b } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test21b } */
}

/* 35 bits aka more than 4 bytes.  */
struct bit_struct {
  unsigned int a : 7;
  unsigned int b : 7;
  unsigned int c : 7;
  unsigned int d : 7;
  unsigned int e : 7;
};

void test22 (void)
{
  memcpy_t fn = get_memcpy ();
  struct bit_struct bs;
  memset (&bs, 0, sizeof (bs));

  char *ptr = (char *) &bs;
  fn (ptr, ptr + 2, 2);
  fn (ptr + 2, ptr, 3); /* { dg-line test22 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test22 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test22 } */
  /* { dg-message "copying 3 bytes from 'ptr' to 'ptr \\\+ 2' overlaps by 1 byte\\n" "note" { target *-*-* } test22 } */
}

/* Union.  */

union my_union {
  int16_t s;
  int32_t m;
};

void test23 (void)
{
  memcpy_t fn = get_memcpy ();
  
  union my_union u;
  void *ptr = (void *) &u;
  fn (ptr + 2, &u, 2);
}

void test24 (void)
{
  memcpy_t fn = get_memcpy ();
  
  union my_union u;
  void *ptr = (void *) &u;
  fn (ptr + 1, &u, 2); /* { dg-line test24 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test24 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test24 } */
  /* { dg-message "copying 2 bytes from '&u' to 'ptr \\\+ 1' overlaps by 1 byte\\n" "note" { target *-*-* } test24 } */
}

/* Test fallback.  */

void test25 (int n)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  fn (buf, buf, 2 * n); /* { dg-line test25 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test25 } */
  /* { dg-message "copying n \\\* 2 bytes from '&buf' to '&buf' overlaps by n \\\* 2 bytes" "note" { target *-*-* } test25 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test25 } */
}

void test26 (int n)
{
  memcpy_t fn = get_memcpy ();

  int32_t buf[4];
  void *alias = buf;
  fn (alias, buf, 2 * n); /* { dg-line test26 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test26 } */
  /* { dg-message "copying n \\\* 2 bytes from '&buf' to 'alias' overlaps by n \\\* 2 bytes" "note" { target *-*-* } test26 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test26 } */
}

void test27 (int n)
{
  memcpy_t fn = get_memcpy ();

  int buf[4];
  int *alias = buf;
  fn (alias + 1, buf, 2 * n);
}

/* Interprocedural.  */

void __attribute__((noinline)) memcpy_wrapper (void *dst, void *src, size_t count)
{
  memcpy_t fn = get_memcpy ();

  fn (dst, src, count); /* { dg-line test28 } */

  /* { dg-warning "calling 'memcpy' with overlapping buffers results in undefined behavior" "warning" { target *-*-* } test28 } */
  /* { dg-message "copying 12 bytes from 'src' to 'dst' overlaps by 4 bytes" "note" { target *-*-* } test28 } */
  /* { dg-message "use 'memmove' instead of 'memcpy' with overlapping buffers" "note" { target *-*-* } test28 } */
}

void test28 (void)
{
  int32_t buf[5];
  memcpy_wrapper (buf + 2, buf, 3 * sizeof (int32_t));
}
