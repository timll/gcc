/* { dg-additional-options "-Wno-stringop-overflow -Wno-stringop-truncation" } */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Wanalyzer-out-of-bounds tests for str(n)py-related overflows.   */

void test1 (void)
{
  char dst[5];
  strcpy (dst, "Hello"); /* { dg-line test1 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test1 } */
  /* { dg-message "dst" "note" { target *-*-* } test1 } */
}

void test2 (void)
{
  char dst[6];
  strcpy (dst, "Hello");
}

void test3 (void)
{
  char dst[5];
  strncpy (dst, "Hello", 6); /* { dg-line test3 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test3 } */
  /* { dg-message "dst" "note" { target *-*-* } test3 } */
}

void test4 (void)
{
  char dst[5];
  strncpy (dst, "Hello", 5);
}

void test5 (void)
{
  char *src = "Hello";
  char dst[5];
  strcpy (dst, src); /* { dg-line test5 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test5 } */
  /* { dg-message "dst" "note" { target *-*-* } test5 } */
}

void test6 (void)
{
  char *src = "Hello";
  char dst[6];
  strcpy (dst, src);
}

void test7 (void)
{
  int n = 6;
  char *src = "Hello";
  char dst[5];
  strncpy (dst, src, n); /* { dg-line test7 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test7 } */
  /* { dg-message "dst" "note" { target *-*-* } test7 } */
}

void test8 (void)
{
  int n = 5;
  char *src = "Hello";
  char dst[n];
  strncpy (dst, src, n);
}
