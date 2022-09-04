/* { dg-additional-options "-Wno-stringop-overflow -Wno-stringop-truncation" } */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Wanalyzer-out-of-bounds tests for strpy-related overflows.
  
   The intra-procedural tests are all caught by Wstringop-overflow.
   The inter-procedural out-of-bounds are only found by the analyzer.  */

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
  char *src = "Hello";
  char dst[5];
  strcpy (dst, src); /* { dg-line test5 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test5 } */
  /* { dg-message "dst" "note" { target *-*-* } test5 } */
}

void test4 (void)
{
  char *src = "Hello";
  char dst[6];
  strcpy (dst, src);
}

char *return_hello (void)
{
  return "hello";
}

void test5 (void)
{
  char *str = return_hello ();
  if (!str)
    return;
  char dst[5];
  strcpy (dst, str); /* { dg-line test9 } */

  /* { dg-warning "overflow" "warning" { target *-*-* } test9 } */
  /* { dg-message "dst" "note" { target *-*-* } test9 } */
}

void test6 (void)
{
  char *str = return_hello ();
  if (!str)
    return;
  char dst[6];
  strcpy (dst, str);
}
