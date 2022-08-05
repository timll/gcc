/* Reduced from coreutils/ls.c attach.  */

void add_zero_terminator (char *buf)
{
  char *end = buf;
  while (end++);
  if (buf < end)
    /* This line triggered a false positive when the BUF+1 and BUF paths
       got merged into a single WIDENING_SVALUE. */
    end[-1] = '\0';
}
