#ifndef ip_parse_h
#define ip_parse_h

#include "ip.h"
#include <stdio.h>

int iap_ip_parse_from_file(FILE *a, iap_ip_t **r);
int iap_ip_parse_from_str_list(int argc, const char **argv, iap_ip_t **r);
const char *iap_ip_parse_last_error_str();

#endif