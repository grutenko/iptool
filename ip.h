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

/**
 * @brief Return raw subnet mask
 *
 * @param cidr
 * @return raw mask
 */
unsigned int iap_mask(int cidr);
/**
 * @brief return raw repr for address
 *
 * @param net address struct
 * @return raw address
 */
unsigned int iap_raw(const iap_t *net);
/**
 * @brief Return first address in range
 *
 * @param net address struct
 * @param from first address in range
 * @return void
 */
void iap_from(const iap_t *net, iap_t *from);
/**
 * @brief Return last address in range
 *
 * @param net address struct
 * @param to last address in range
 * @return void
 */
void iap_to(const iap_t *net, iap_t *to);
/**
 * @brief Increment address
 *
 * @param net address struct
 * @return void
 */
void iap_inc(iap_t *net);
/**
 * @brief Compare two addresses
 *
 * @param net0 first address
 * @param net1 second address
 * @return int
 */
int iap_in(const iap_t *net0, const iap_t *net1);
/**
 * @brief Compare two addresses
 *
 * @param net0 first address
 * @param net1 second address
 * @return int
 */
int iap_cmp(const iap_t *net0, const iap_t *net1);
/**
 * @brief Insert address into tree
 *
 * @param root root of tree
 * @param new address to insert
 * @return iap_t*
 */
iap_t *iap_insert(iap_t **root, const iap_t *new);
/**
 * @brief Remove addresses from tree by subnet
 *
 * @param root root of tree
 * @param net address to prune
 * @return void
 */
void iap_prune(iap_t **root, const iap_t *net);
/**
 * @brief Remove address from tree by concrete address and subnet
 *
 * @param root root of tree
 * @param net address to remove
 * @return void
 */
void iap_remove(iap_t **root, const iap_t *net);
/**
 * @brief Free tree
 *
 * @param root root of tree
 * @return void
 */
void iap_free(iap_t **root);
/**
 * @brief Walk tree
 *
 * @param root root of tree
 * @param proc callback function
 * @return void
 */
void iap_walk(const iap_t *root, iap_walk_proc_p proc);
/**
 * @brief Convert address to string
 *
 * @param net address struct
 * @param out output buffer
 * @return return length of string
 */
int iap_ntoa(const iap_t *net, char *out);
/**
 * @brief Convert string to address
 *
 * @param str input string
 * @param size size of input string
 * @param out output buffer
 * @return return length of string or 0 if address is invalid
 */
int iap_aton(const char *str, int size, iap_t *out);
/**
 * @brief Create tree with subnets with all addresses in range
 *
 * @param from first address
 * @param to last address
 * @param root root of tree
 * @return return count of inserted addresses or 0 if memory allocation failed
 */
int iap_range(const iap_t *from, const iap_t *to, iap_t **root);

#endif
