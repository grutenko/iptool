#include "ip-parse.h"
#include "ip.h"

#include <stdio.h>
#include <string.h>

/**
 * static buffer for string error of iap_ip_parse_from_file(),
 * iap_ip_parse_from_ip_list()
 */
static char iap_parse_buffer[256] = {0};

/**
 * parse string impl of ip address
 */
int iap_ip_parse(const char *str, int size, iap_ip_t *out) {
  const char *p = str;
  unsigned int a[4] = {0, 0, 0, 0}, cidr = 32;
  int digit = 0, part = 0;

  while (p - str < size) {
    if (*p >= '0' && *p <= '9') {
      a[part] = a[part] * 10 + (*p - '0');

      if (digit >= 3 || a[part] > 255)
        return 0;

      digit++;
    } else if (*p == '.') {
      if (!digit || part >= 3)
        return 0;

      digit = 0;
      part++;
    } else if (*p == '/')
      break;
    else
      return 0;

    p++;
  }

  if (!digit || part != 3)
    return 0;

  if (*p == '/') {
    p++;
    digit = 0;
    cidr = 0;
    while (*p >= '0' && *p <= '9') {
      cidr = cidr * 10 + (*p - '0');

      if (digit >= 2 || cidr > 32)
        return 0;

      digit++;
      p++;
    }

    if (!digit)
      return 0;
  }

  if (out) {
    out->a[0] = a[0];
    out->a[1] = a[1];
    out->a[2] = a[2];
    out->a[3] = a[3];
    out->cidr = cidr;
  }

  return p - str;
}

/**
 * parse list of ip addresses from file into tree
 */
int iap_ip_parse_from_file(FILE *o, iap_ip_t **r) {
  char buffer[IAP_BEST_LEN + 1], *buffer_p = buffer;
  int c = getc(o);
  iap_ip_t a = {0};
  int rc;

  while (c != EOF) {
    if ((c >= '0' && c <= '9') || c == '.' || c == '/') {
      if (buffer_p - buffer >= IAP_BEST_LEN)
        goto _failure;

      *buffer_p++ = c;
      *buffer_p = '\0';
    } else if (c == ' ' || c == '\n' || c == ',') {
      if (buffer_p - buffer > 0) {
        rc = iap_ip_parse(buffer, buffer_p - buffer, &a);

        if (!rc)
          goto _failure;

        if (!iap_insert(r, &a))
          goto _mem_failure;

        buffer_p = buffer;
      }
    } else {
      goto _failure_inv_char;
    }
    c = getc(o);
  }

  return 0;
_mem_failure:
  iap_free(r);
  return 2;
_failure:
  snprintf(iap_parse_buffer, 255, "invalid input: %s\n", buffer);
  iap_free(r);
  return 1;
_failure_inv_char:
  snprintf(iap_parse_buffer, 255, "invalid character %c.\n", c);
  iap_free(r);
  return 1;
}

/**
 * parse list of ip addresses from list of strings into tree
 */
int iap_ip_parse_from_str_list(int argc, const char **argv, iap_ip_t **r) {
  int i;
  int rc;
  iap_ip_t a = {0};

  for (i = 0; i < argc; i++) {
    rc = iap_ip_parse(argv[i], strlen(argv[i]), &a);

    if (!rc)
      goto _failure;

    if (!iap_insert(r, &a))
      goto _mem_failure;
  }

  return 0;
_mem_failure:
  iap_free(r);
  return 2;
_failure:
  snprintf(iap_parse_buffer, 255, "invalid address %s.\n", argv[i]);
  iap_free(r);
  return 1;
}

/**
 * return string error for last list parsing
 */
const char *iap_ip_parse_last_error_str() { return iap_parse_buffer; }