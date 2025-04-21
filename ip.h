#ifndef ip_h
#define ip_h

typedef struct iap_ip {
  unsigned char a[4];
  int cidr;
  struct iap_ip *l; // left
  struct iap_ip *r; // right
  int h;
} iap_ip_t;

#define IAP_IP(a, b, c, d) \
  {.a = {a, b, c, d}, .cidr = 32, .l = 0, .r = 0, .h = 0}

unsigned int iap_raw(const iap_ip_t *a);
unsigned int iap_mask(int cidr);
int iap_in(const iap_ip_t *net, const iap_ip_t *a);
iap_ip_t *iap_set_insert(iap_ip_t **r, const iap_ip_t *a);
void iap_prune(iap_ip_t **r, const iap_ip_t *net);
void iap_remove(iap_ip_t **r, const iap_ip_t *a);
void iap_free(iap_ip_t **r);

#endif
