#ifndef ip_h
#define ip_h

#define IAP_BEST_LEN 19

typedef struct iap {
  unsigned char a[4];
  int cidr;
  struct iap *l, *r; // right
  int h;
} iap_t;

enum {
  IAP_WALK_PREORDER = 0,
  IAP_WALK_INORDER = 1,
  IAP_WALK_POSTORDER = 2
};

typedef void(*iap_walk_proc_p)(const iap_t *a, int depth, int mode);

unsigned int iap_mask(int cidr);
unsigned int iap_raw(const iap_t *net);
void iap_from(const iap_t *net, iap_t *from);
void iap_to(const iap_t *net, iap_t *to);
void iap_inc(iap_t *net);
int iap_in(const iap_t *net0, const iap_t *net1);
iap_t *iap_insert(iap_t **root, const iap_t *new);
void iap_prune(iap_t **root, const iap_t *net);
void iap_remove(iap_t **root, const iap_t *net);
void iap_free(iap_t **root);
void iap_walk(const iap_t *root, iap_walk_proc_p proc);
int iap_ntoa(const iap_t *net, char *out);
int iap_aton(const char *str, int size, iap_t *out);

#endif
