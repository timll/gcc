#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Wanalyzer-out-of-bounds tests for buffer underreads and writes.  */

/* Avoid folding of memcpy.  */
typedef void * (*memcpy_t) (void *dst, const void *src, size_t n);
static memcpy_t __attribute__((noinline))
get_memcpy (void)
{
  return memcpy;
}


void test1 (void)
{
  int buf[4];
  /* This actually compiles down to &buf + 18446744073709551612.  */ 
  int *e = buf - 1;
  *e = 42; /* { dg-line test1 } */

  /* { dg-warning "underflow" "warning" { target *-*-* } test1 } */
  /* { dg-message "" "note" { target *-*-* } test1 } */
}

void test2 (void)
{
  int buf[4];
  int *e = buf + 1;
  *e = 123;
  /* This actually compiles down to e_2 + 18446744073709551612.
     Doesn't seem to be defined behavior?  */
  *(e - 1) = 321;
}

void test3 (void)
{
  int buf[4];
  int *e = buf + 1;
  *e = 123;
  /* This compiles down to e_2 + 18446744073709551612. Doesn't seem
     to be defined behavior?  */
  *(e - 2) = 321; /* { dg-line test3 } */

  /* { dg-warning "underflow" "warning" { target *-*-* } test3 } */
  /* { dg-message "" "note" { target *-*-* } test3 } */
}

void test4 (void)
{
  memcpy_t fn = get_memcpy ();
  int buf[4];
  memset (buf, 1, 4 * sizeof (int));
  int n = -4;
  fn (&(buf[n]), buf, sizeof (int));  /* { dg-line test4 } */

  /* { dg-warning "underflow" "warning" { target *-*-* } test4 } */
  /* { dg-message "" "note" { target *-*-* } test4 } */
}

void test5 (void)
{
  int buf[4];
  memset (buf, 1, 4 * sizeof (int));

  int sum = 0;
  for (int i = 4; i >= 0; i++)
    sum += *(buf + i); /* { dg-line test5 } */

  /* { dg-warning "overread" "warning" { target *-*-* } test5 } */
  /* { dg-message "" "note" { target *-*-* } test5 } */
}

void test6 (void)
{
  int buf[4];
  memset (buf, 1, 4 * sizeof (int));

  int *view = buf + 1;
  int sum = 0;
  for (int i = 0; i < 4; i++)
    sum += *(view++);
}

void test8 (void)
{
  memcpy_t fn = get_memcpy ();
  int buf[4];
  memset (buf, 1, 4 * sizeof (int));
  int n = -4;
  fn (buf, &(buf[n]), sizeof (int));  /* { dg-line test8 } */

  /* { dg-warning "underread" "warning" { target *-*-* } test8 } */
  /* { dg-message "" "note" { target *-*-* } test8 } */
}

/* Further reduced container_of pattern from the Linux Kernel.  */

struct inner {
  /* Don't care */
};

struct outer {
  int i;
  struct inner inner_struct;
};

struct outer *container_of (struct inner *ptr_to_inner)
{
  struct outer *ptr_to_outer = ((struct outer *) (((void *) ptr_to_inner) - __builtin_offsetof(struct outer, inner_struct)));
  return ptr_to_outer;
}

int test9 (struct outer *outer_p, struct inner *inner_p)
{
  struct outer test;
  test.i = 42;
  int sum = 0;
  struct outer *o;

  /* Symbolic inner struct.  */
  o = container_of (inner_p);
  sum += o->i; // ok
  /* not ok, but we can't be sure that outer
     is actually the container of inner.  */
  sum += (o - 1)->i;
  /* Symbolic outer struct.  */
  o = container_of (&(outer_p->inner_struct));
  sum += o->i; // ok
  /* not ok, but indistinguishable from the case above.  */
  sum += (o - 1)->i;
  /* Concrete outer struct.  */
  o = container_of (&(test.inner_struct));
  sum += o->i;  // ok
  /* not ok, but this time we do not have a symbolic region.  */
  sum += (o - 1)->i; /* { dg-line test9 } */

  return sum;
  /* { dg-warning "underread" "warning" { target *-*-* } test9 } */
  /* { dg-message "" "note" { target *-*-* } test9 } */
}
