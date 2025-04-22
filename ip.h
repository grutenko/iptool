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

enum {
  IAP_WALK_PREORDER = 0,
  IAP_WALK_INORDER = 1,
  IAP_WALK_POSTORDER = 2
};

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
int iap_ip_parse_from_file(FILE *o, iap_ip_t **r);
int iap_ip_parse_from_str_list(int argc, const char **argv, iap_ip_t **r);
const char *iap_ip_parse_last_error_str();
void iap_walk(iap_ip_t *r, void(*proc)(iap_ip_t *a, int mode));
void iap_print_tree(iap_ip_t *r, int offset);

#endif
