#include "core.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * return bit mask for target cidr.
 */
static inline unsigned int iap_mask_fast(int cidr) {
  return cidr > 0 ? ~0U << (32 - cidr) : 0U;
}

/**
 * return ip address as unsigned integer
 */
static inline unsigned int iap_raw_fast(const iap_t *addr) {
  return (addr->a[0] << 24) | (addr->a[1] << 16) | (addr->a[2] << 8) |
         addr->a[3];
}

/**
 * check contain address in subnet
 */
static inline int iap_in_fast(const iap_t *net, const iap_t *addr) {
  return net->cidr <= addr->cidr &&
         (iap_raw_fast(net) & iap_mask_fast(net->cidr)) ==
             (iap_raw_fast(addr) & iap_mask_fast(net->cidr));
}

static inline int max(int a, int b) { return a > b ? a : b; }

/**
 * Return AVL height of target node
 */
static inline int height(iap_t *node) { return node ? node->avl_height : 0; }

/**
 * Return AVL balance factor for target node
 */
static inline int bfactor(iap_t *node) {
  return height(node->l) - height(node->r);
}

/**
 * AVL rotate left operation for target node
 * return new node
 */
static inline iap_t *rotl(iap_t *x) {
  iap_t *y = x->r;
  iap_t *T2 = y->l;

  y->l = x;
  x->r = T2;

  x->avl_height = max(height(x->l), height(x->r)) + 1;
  y->avl_height = max(height(y->l), height(y->r)) + 1;

  return y;
}

/**
 * AVL rotate right operation for target node
 * return new node
 */
static inline iap_t *rotr(iap_t *y) {
  iap_t *x = y->l;
  iap_t *T2 = x->r;

  x->r = y;
  y->l = T2;

  y->avl_height = max(height(y->l), height(y->r)) + 1;
  x->avl_height = max(height(x->l), height(x->r)) + 1;

  return x;
}

/**
 * AVL balance target node
 * return new node
 */
static iap_t *balance(iap_t *node) {
  int bfac;

  if (!node)
    return NULL;

  bfac = bfactor(node);

  if (bfac > 1) {
    if (bfactor(node->l) < 0)
      node->l = rotl(node->l);

    return rotr(node);
  }
  if (bfac < -1) {
    if (bfactor(node->r) > 0)
      node->r = rotr(node->r);

    return rotl(node);
  }

  return node;
}

/**
 * Compare ip nodes by address and subnets
 * 127.0.0.1 == 127.0.0.0/24
 * 127.0.0.0/24 == 127.0.0.1
 * 127.0.0.1 > 127.0.0.0
 * 0.0.0.0/0 == any address
 */
static inline int iap_key_cmp_fast(const iap_t *a, const iap_t *b) {
  unsigned int raw_a, raw_b, min_cidr;
  min_cidr = a->cidr > b->cidr ? b->cidr : a->cidr;
  raw_a = iap_raw_fast(a) & iap_mask_fast(min_cidr);
  raw_b = iap_raw_fast(b) & iap_mask_fast(min_cidr);
  if (raw_a > raw_b)
    return 1;
  else if (raw_a < raw_b)
    return -1;
  return 0;
}

/**
 * Strict compare two addresses:
 * 127.0.0.1 == 127.0.0.1
 * 127.0.0.0/31 < 127.0.0.0/30
 * 127.0.0.0/31 == 127.0.0.0/31
 */
static inline int iap_key_cmp_strict_fast(const iap_t *a, const iap_t *b) {
  unsigned int raw_a, raw_b;
  raw_a = iap_raw_fast(a);
  raw_b = iap_raw_fast(b);
  if (raw_a > raw_b)
    return 1;
  else if (raw_a < raw_b)
    return -1;
  else if (a->cidr > b->cidr)
    return -1;
  else if (a->cidr < b->cidr)
    return 1;
  return 0;
}

/**
 * remove node from tree by address and cidr.
 */
static iap_t *iap_remove_fast(iap_t *root, const iap_t *addr) {
  int cmp;

  if (!root)
    return (void *)0;

  cmp = iap_key_cmp_strict_fast(root, addr);
  if (cmp > 0)
    root->r = iap_remove_fast(root->r, addr);
  else if (cmp < 0)
    root->l = iap_remove_fast(root->l, addr);
  else {
    iap_t *t = root;

    if (!root->l)
      root = root->r;
    else if (!root->r)
      root = root->l;
    else {
      iap_t *p = root->r;

      while (p->l)
        p = p->l;

      memmove(root, p, sizeof(iap_t));

      root->r = iap_remove_fast(root->r, p);
      t = (void *)0;
    }

    free(t);
  }

  return balance(root);
}

/**
 * remove all nodes contain in target subnet
 */
static iap_t *iap_prune_fast(iap_t *root, const iap_t *net) {
  iap_t *t;

  if (!root)
    return (void *)0;

  if (root->l)
    root->l = iap_prune_fast(root->l, net);
  if (root->r)
    root->r = iap_prune_fast(root->r, net);

  if (iap_in_fast(net, root)) {
    t = root;

    if (!root->l)
      root = root->r;
    else if (!root->r)
      root = root->l;
    else {
      iap_t *p = root->r;

      while (p->l)
        p = p->l;

      memmove(root, p, sizeof(iap_t));

      root->r = iap_remove_fast(root->r, p);
      t = (void *)0;
    }

    free(t);
  }

  return balance(root);
}

/**
 * Insert node in tree
 */
iap_t *iap_insert(iap_t **root, const iap_t *new) {
  iap_t **p;
  iap_t **stack[256] = {0};
  int infloop_guard = 0;
  int sp;

_again:

  sp = 0;
  p = root;
  stack[sp++] = p;

  // find box for new node, or equal node if exist
  while (*p) {
    int cmp;
    cmp = iap_key_cmp_fast(*p, new);

    if (cmp < 0)
      p = &((*p)->r);
    else if (cmp > 0)
      p = &((*p)->l);
    else
      break;

    stack[sp++] = p;
  }

  // if target node already exists - return this
  if (*p) {
    if ((*p)->cidr <= new->cidr)
      return *p;

    assert(infloop_guard == 0);
    infloop_guard = 1;

    *root = iap_prune_fast(*root, new);
    goto _again;
  }

  // p now is &(parent_node->[left|right])

  iap_t *t;
  t = malloc(sizeof(iap_t));

  if (!t)
    goto _emem;

  memmove(t, new, sizeof(iap_t));
  t->avl_height = 1;
  t->l = 0;
  t->r = 0;

  *p = t;

  // balance tree postorder
  int i;
  for (i = sp - 1; i >= 0; i--)
    *stack[i] = balance(*stack[i]);

  return t;
_emem:
  fprintf(stderr, "Out of memory\n");
  exit(EXIT_FAILURE);
}

static inline void iap_from_fast(const iap_t *net, iap_t *out) {
  unsigned int raw_a = iap_raw_fast(net) & iap_mask_fast(net->cidr);
  out->a[0] = (raw_a >> 24) & 0xff;
  out->a[1] = (raw_a >> 16) & 0xff;
  out->a[2] = (raw_a >> 8) & 0xff;
  out->a[3] = raw_a & 0xff;
  out->cidr = 32;
}

static inline void iap_to_fast(const iap_t *net, iap_t *out) {
  unsigned int raw_a = iap_raw_fast(net) | ~iap_mask_fast(net->cidr);
  out->a[0] = (raw_a >> 24) & 0xff;
  out->a[1] = (raw_a >> 16) & 0xff;
  out->a[2] = (raw_a >> 8) & 0xff;
  out->a[3] = raw_a & 0xff;
  out->cidr = 32;
}

/**
 * free tree
 */
static void iap_free_fast(iap_t *root) {
  if (!root)
    return;

  if (root->l)
    iap_free_fast(root->l);
  if (root->r)
    iap_free_fast(root->r);

  free(root);
}

unsigned int iap_raw(const iap_t *addr) { return iap_raw_fast(addr); }

unsigned int iap_mask(int cidr) { return iap_mask_fast(cidr); }

int iap_in(const iap_t *net, const iap_t *a) { return iap_in_fast(net, a); }

void iap_remove(iap_t **root, const iap_t *a) {
  *root = iap_remove_fast(*root, a);
}

void iap_prune(iap_t **root, const iap_t *net) {
  *root = iap_prune_fast(*root, net);
}

void iap_free(iap_t **root) {
  iap_free_fast(*root);
  *root = (void *)0;
}

void iap_inc(iap_t *net) {
  unsigned int raw_a = iap_raw_fast(net) + 1;
  net->a[0] = (raw_a >> 24) & 0xff;
  net->a[1] = (raw_a >> 16) & 0xff;
  net->a[2] = (raw_a >> 8) & 0xff;
  net->a[3] = raw_a & 0xff;
}

void iap_from(const iap_t *net, iap_t *from) { iap_from_fast(net, from); }

void iap_to(const iap_t *net, iap_t *to) { iap_to_fast(net, to); }

static char *u8tab[] = {
    "0",   "1",   "2",   "3",   "4",   "5",   "6",   "7",   "8",   "9",   "10",
    "11",  "12",  "13",  "14",  "15",  "16",  "17",  "18",  "19",  "20",  "21",
    "22",  "23",  "24",  "25",  "26",  "27",  "28",  "29",  "30",  "31",  "32",
    "33",  "34",  "35",  "36",  "37",  "38",  "39",  "40",  "41",  "42",  "43",
    "44",  "45",  "46",  "47",  "48",  "49",  "50",  "51",  "52",  "53",  "54",
    "55",  "56",  "57",  "58",  "59",  "60",  "61",  "62",  "63",  "64",  "65",
    "66",  "67",  "68",  "69",  "70",  "71",  "72",  "73",  "74",  "75",  "76",
    "77",  "78",  "79",  "80",  "81",  "82",  "83",  "84",  "85",  "86",  "87",
    "88",  "89",  "90",  "91",  "92",  "93",  "94",  "95",  "96",  "97",  "98",
    "99",  "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    "110", "111", "112", "113", "114", "115", "116", "117", "118", "119", "120",
    "121", "122", "123", "124", "125", "126", "127", "128", "129", "130", "131",
    "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142",
    "143", "144", "145", "146", "147", "148", "149", "150", "151", "152", "153",
    "154", "155", "156", "157", "158", "159", "160", "161", "162", "163", "164",
    "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
    "176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186",
    "187", "188", "189", "190", "191", "192", "193", "194", "195", "196", "197",
    "198", "199", "200", "201", "202", "203", "204", "205", "206", "207", "208",
    "209", "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
    "220", "221", "222", "223", "224", "225", "226", "227", "228", "229", "230",
    "231", "232", "233", "234", "235", "236", "237", "238", "239", "240", "241",
    "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252",
    "253", "254", "255"};

static inline int u8toa(unsigned char v, char *out) {
  const char *s = u8tab[v];
  int i = 0;

  while ((out[i] = s[i]))
    i++;

  return i;
}

static inline int iap_ntoa_fast(const iap_t *a, char *out) {
  char *p = out;

  // clang-format off
  p += u8toa(a->a[0], p); *p = '.'; p++;
  p += u8toa(a->a[1], p); *p = '.'; p++;
  p += u8toa(a->a[2], p); *p = '.'; p++;
  p += u8toa(a->a[3], p);

  if(a->cidr != 32) {
    *p = '/'; p++;
    p += u8toa(a->cidr, p);
  }
  // clang-format on

  *p = '\0';

  return p - out;
}

int iap_ntoa(const iap_t *addr, char *out) { return iap_ntoa_fast(addr, out); }

/**
 * parse string impl of ip address
 */
int iap_aton(const char *str, int size, iap_t *out) {
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

  if ((((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]) &
       iap_mask_fast(cidr)) !=
      ((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]))
    return 0;

  if (out) {
    out->a[0] = a[0];
    out->a[1] = a[1];
    out->a[2] = a[2];
    out->a[3] = a[3];
    out->cidr = cidr;
  }

  return p - str;
}

static void iap_walk_fast(const iap_t *root, int level, void *data,
                          iap_walk_proc_p proc) {
  if (!root)
    return;

  proc(root, level, IAP_WALK_PREORDER, data);

  if (root->l)
    iap_walk_fast(root->l, level + 1, data, proc);

  proc(root, level, IAP_WALK_INORDER, data);

  if (root->r)
    iap_walk_fast(root->r, level + 1, data, proc);

  proc(root, level, IAP_WALK_POSTORDER, data);
}

void iap_walk(const iap_t *root, iap_walk_proc_p proc, void *data) {
  iap_walk_fast(root, 0, data, proc);
}

int iap_eq(const iap_t *net0, const iap_t *net1) {
  if (!net0 || !net1)
    return 0;
  if (net0->cidr != net1->cidr)
    return 0;
  return ((iap_raw_fast(net0) & iap_mask(net0->cidr)) ==
          (iap_raw_fast(net1) & iap_mask(net1->cidr)));
}

int iap_range_insert(const iap_t *from, const iap_t *to, iap_t **root) {
  return 0;
}

int iap_range_aton(const char *str, int size, iap_t *from, iap_t *to) {
  char *p = strchr(str, '-');
  if (!p || p - str >= size)
    return 0;

  if (!iap_aton(str, p - str, from) || from->cidr != 32)
    return 0;

  if (!iap_aton(p + 1, size - (p - str + 1), to) || to->cidr != 32)
    return 0;

  if (iap_raw_fast(from) <= iap_raw_fast(to))
    return 0;

  return 1;
}
