#ifndef ip_range_h
#define ip_range_h

#include "ip.h"

int iap_range_to_cidr_list(iap_ip_t *from, iap_ip_t *to, iap_ip_t **r);

#endif