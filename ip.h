#ifndef ip_h
#define ip_h

#include <stdio.h>

#define IAP_BEST_LEN 19

typedef struct iap_ip {
  unsigned char a[4];
  int cidr;
  struct iap_ip *l; // left
  struct iap_ip *r; // right
  int h;
} iap_ip_t;

#define IAP_IP(a, b, c, d)                                                     \
  {.a = {a, b, c, d}, .cidr = 32, .l = 0, .r = 0, .h = 0}

unsigned int iap_raw(const iap_ip_t *a);
unsigned int iap_mask(int cidr);
int iap_in(const iap_ip_t *net, const iap_ip_t *a);
iap_ip_t *iap_insert(iap_ip_t **r, const iap_ip_t *a);
void iap_prune(iap_ip_t **r, const iap_ip_t *net);
void iap_remove(iap_ip_t **r, const iap_ip_t *a);
void iap_free(iap_ip_t **r);
int iap_ip_parse(const char *str, int size, iap_ip_t *a);
void iap_net_from(iap_ip_t *a, iap_ip_t *from);
void iap_net_to(iap_ip_t *a, iap_ip_t *to);
void iap_increment(iap_ip_t *a);
int iap_ip_to_a(iap_ip_t *a, char *out);

typedef struct iap_ctx {
  iap_ip_t *stack[256];
  int sp;
} iap_ctx_t;

iap_ip_t *iap_begin(iap_ip_t *r, iap_ctx_t *ctx);
iap_ip_t *iap_next(iap_ctx_t *ctx);

#endif
