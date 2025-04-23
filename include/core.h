#ifndef core_h
#define core_h

#define IAP_BEST_LEN 19

typedef struct iap {
  unsigned char a[4], cidr;
  struct iap *l, *r; // right
  int avl_height;
} iap_t;

enum { IAP_WALK_PREORDER = 0, IAP_WALK_INORDER = 1, IAP_WALK_POSTORDER = 2 };

typedef void (*iap_walk_proc_p)(const iap_t *a, int depth, int mode,
                                void *data);

/**
 * @brief Return raw subnet mask
 *
 * Return subnet mask for given cidr. For example, if cidr is 24, the function
 * returns 0xFFFFFF00.
 *
 * @param[in] cidr
 * @return raw mask
 */
unsigned int iap_mask(int cidr);
/**
 * @brief Return raw representation of an address
 *
 * Return raw representation of an address. Example: if address is 192.168.0.1,
 * the function returns 0xC0A80001.
 *
 * @param[in] net address struct
 * @return raw address
 */
unsigned int iap_raw(const iap_t *net);
/**
 * @brief Return first address in range
 *
 * Return first address in range. Example: if address is 192.168.0.1/24,
 * the "from" variable is set to 192.168.0.0.
 *
 * @param[in] net address struct
 * @param[out] from first address in range
 * @return void
 */
void iap_from(const iap_t *net, iap_t *from);
/**
 * @brief Return last address in range
 *
 * Return last address in range. Example: if address is 192.168.0.1/24,
 * the "to" variable is set to 192.168.0.255.
 *
 * @param[in] net address struct
 * @param[out] to last address in range
 * @return void
 */
void iap_to(const iap_t *net, iap_t *to);
/**
 * @brief Increment address
 *
 * Increment address. Example: if address is 192.168.0.1, this address becomes
 * 192.168.0.2.
 *
 * @param[in,out] net address struct
 * @return void
 */
void iap_inc(iap_t *net);
/**
 * @brief Check address in range
 *
 * Check address in range. if net0 contains net1 return true
 *
 * @param[in] net0 first address
 * @param[in] net1 second address
 * @return int
 */
int iap_in(const iap_t *net0, const iap_t *net1);
/**
 * @brief Compare addresses
 *
 * Compare addresses. Return 1 if net0 address == net1 address and net0 mask ==
 * net1 mask
 *
 * @param[in] net0 first address
 * @param[in] net1 second address
 * @return int
 */
int iap_eq(const iap_t *net0, const iap_t *net1);
/**
 * @brief Insert address into tree
 *
 * Insert address into tree. If address already exists, return existing address.
 * If address is part of an existing address, return existing address If address
 * contain one or many addresses in tree, this address will be remove from tree.
 *
 * @param[in,out] root root of tree
 * @param[in] _new address to insert
 * @return iap_t*
 */
iap_t *iap_insert(iap_t **root, const iap_t *);
/**
 * @brief Remove addresses from tree by subnet
 *
 * Remove from tree all addresses that are part of net.
 *
 * @param[in,out] root root of tree
 * @param[in] net address to prune
 * @return void
 */
void iap_prune(iap_t **root, const iap_t *net);
/**
 * @brief Remove address from tree by concrete address and subnet
 *
 * Remove one address from tree by concrete address and subnet.
 *
 * @param[in,out] root root of tree
 * @param[in] net address to remove
 * @return void
 */
void iap_remove(iap_t **root, const iap_t *net);
/**
 * @brief Free tree
 *
 * Free all memory allocated for tree. "root" will be set to NULL.
 *
 * @param[in,out] root root of tree
 * @return void
 */
void iap_free(iap_t **root);
/**
 * @brief Walk tree
 *
 * Walk tree and call callback function for each address. for each address
 * function will be called 3 times with different modes:
 * 1. iap_walk_mode_t::PREORDER
 * 2. iap_walk_mode_t::INORDER
 * 3. iap_walk_mode_t::POSTORDER
 *
 * @param[in] root root of tree
 * @param[in] proc callback function
 * @param[in] data user data
 * @return void
 */
void iap_walk(const iap_t *root, iap_walk_proc_p proc, void *data);
/**
 * @brief Convert address to string
 *
 * Convert address into string with cidr subnet.
 *
 * @param[in] net address struct
 * @param[out] out output buffer
 * @return return length of string
 */
int iap_ntoa(const iap_t *net, char *out);
/**
 * @brief Convert string to address
 *
 * Convert string into address with cidr subnet.
 * 127.0.0.0/30 Ok
 * 0.0.0.0/0 Ok
 * 127.0.0.1/31 Failed
 * 127.0.0.1/32 Ok
 *
 * @param[in] str input string
 * @param[in] size size of input string
 * @param[out] out output buffer
 * @return return length of string or 0 if address is invalid
 */
int iap_aton(const char *str, int size, iap_t *out);
/**
 * @brief Append into tree all cidr subnets containing all addresses in range.
 *
 * Append all cidr subnets containing all addresses in range.
 *
 * @param[in] from first address
 * @param[in] to last address
 * @param[in,out] root root of tree
 * @return return count of inserted addresses or 0 if memory allocation failed
 */
int iap_range_insert(const iap_t *from, const iap_t *to, iap_t **root);
/**
 * @brief Parse string into range of addressses.
 *
 * Parse string into range of addresses.
 *
 * @param[in] str input string
 * @param[in] size size of input string
 * @param[out] from first address
 * @param[out] to last address
 * @return return 1 if success, 0 if failed
 */
int iap_range_aton(const char *str, int size, iap_t *from, iap_t *to);

#endif
