#ifndef proc_h
#define proc_h

#include "ip.h"
#include <stdio.h>

/**
 * Invert the addresses in the tree.
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_invert_proc(iap_t **r, FILE *out);
/**
 * Filter the addresses in the tree.
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_filter_proc(iap_t **r, FILE *out);
/**
 * Deflate the addresses in the tree.
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_deflate_proc(iap_t **r, FILE *out);
/**
 * Inflate the addresses in the tree.
 * @param r root of tree
 * @param out output stream
 * @return 0 on success, -1 on error
 */
int cmd_inflate_proc(iap_t **r, FILE *out);

#endif
