#include "ip.h"

#include <assert.h>
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
static inline unsigned int iap_raw_fast(const iap_ip_t *a) {
  return (a->a[0] << 24) | (a->a[1] << 16) | (a->a[2] << 8) | a->a[3];
}

/**
 * check contain address in subnet
 */
static inline int iap_in_fast(const iap_ip_t *net, const iap_ip_t *a) {
  return net->cidr <= a->cidr &&
         (iap_raw_fast(net) & iap_mask_fast(net->cidr)) ==
             (iap_raw_fast(a) & iap_mask_fast(net->cidr));
}

static inline int max(int a, int b) { return a > b ? a : b; }

/**
 * Return AVL height of target node
 */
static inline int height(iap_ip_t *a) { return a ? a->h : 0; }

/**
 * Return AVL balance factor for target node
 */
static inline int bfactor(iap_ip_t *node) {
  return height(node->l) - height(node->r);
}

/**
 * AVL rotate left operation for target node
 * return new node
 */
static inline iap_ip_t *rotl(iap_ip_t *x) {
  iap_ip_t *y = x->r;
  iap_ip_t *T2 = y->l;

  y->l = x;
  x->r = T2;

  x->h = max(height(x->l), height(x->r)) + 1;
  y->h = max(height(y->l), height(y->r)) + 1;

  return y;
}

/**
 * AVL rotate right operation for target node
 * return new node
 */
static inline iap_ip_t *rotr(iap_ip_t *y) {
  iap_ip_t *x = y->l;
  iap_ip_t *T2 = x->r;

  x->r = y;
  y->l = T2;

  y->h = max(height(y->l), height(y->r)) + 1;
  x->h = max(height(x->l), height(x->r)) + 1;

  return x;
}

/**
 * AVL balance target node
 * return new node
 */
static iap_ip_t *balance(iap_ip_t *a) {
  int bfac;

  if (!a)
    return NULL;

  bfac = bfactor(a);

  if (bfac > 1) {
    if (bfactor(a->l) < 0)
      a->l = rotl(a->l);

    return rotr(a);
  }
  if (bfac < -1) {
    if (bfactor(a->r) > 0)
      a->r = rotr(a->r);

    return rotl(a);
  }

  return a;
}

/**
 * Compare ip nodes by address and subnets
 * 127.0.0.1 == 127.0.0.0/24
 * 127.0.0.0/24 == 127.0.0.1
 * 127.0.0.1 > 127.0.0.0
 * 0.0.0.0/0 == any address
 */
static inline int iap_key_cmp_fast(const iap_ip_t *a, const iap_ip_t *b) {
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
static inline int iap_key_cmp_strict_fast(const iap_ip_t *a,
                                          const iap_ip_t *b) {
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
static iap_ip_t *iap_remove_fast(iap_ip_t *r, const iap_ip_t *b) {
  int cmp;

  if (!r)
    return (void *)0;

  cmp = iap_key_cmp_strict_fast(r, b);
  if (cmp > 0)
    r->r = iap_remove_fast(r->r, b);
  else if (cmp < 0)
    r->l = iap_remove_fast(r->l, b);
  else {
    iap_ip_t *t = r;

    if (!r->l)
      r = r->r;
    else if (!r->r)
      r = r->l;
    else {
      iap_ip_t *p = r->r;

      while (p->l)
        p = p->l;

      memmove(r, p, sizeof(iap_ip_t));

      r->r = iap_remove_fast(r->r, p);
      t = (void *)0;
    }

    free(t);
  }

  return balance(r);
}

/**
 * remove all nodes contain in target subnet
 */
static iap_ip_t *iap_prune_fast(iap_ip_t *a, const iap_ip_t *net) {
  iap_ip_t *t;

  if (!a)
    return (void *)0;

  if (a->l)
    a->l = iap_prune_fast(a->l, net);
  if (a->r)
    a->r = iap_prune_fast(a->r, net);

  if (iap_in_fast(net, a)) {
    t = a;

    if (!a->l)
      a = a->r;
    else if (!a->r)
      a = a->l;
    else {
      iap_ip_t *p = a->r;

      while (p->l)
        p = p->l;

      memmove(a, p, sizeof(iap_ip_t));

      a->r = iap_remove_fast(a->r, p);
      t = (void *)0;
    }

    free(t);
  }

  return balance(a);
}

/**
 * Insert node in tree
 */
iap_ip_t *iap_insert(iap_ip_t **r, const iap_ip_t *a) {
  iap_ip_t **p;
  iap_ip_t **stack[256] = {0};
  int infloop_guard = 0;
  int sp;

_again:

  sp = 0;
  p = r;
  stack[sp++] = p;

  // find box for new node, or equal node if exist
  while (*p) {
    int cmp;
    cmp = iap_key_cmp_fast(*p, a);

    if (cmp > 0)
      p = &((*p)->r);
    else if (cmp < 0)
      p = &((*p)->l);
    else
      break;

    stack[sp++] = p;
  }

  // if target node already exists - return this
  if (*p) {
    if ((*p)->cidr <= a->cidr)
      return *p;

    assert(infloop_guard == 0);
    infloop_guard = 1;

    *r = iap_prune_fast(*r, a);
    goto _again;
  }

  // p now is &(parent_node->[left|right])

  iap_ip_t *t;
  t = malloc(sizeof(iap_ip_t));

  if (!t)
    return 0;

  memmove(t, a, sizeof(iap_ip_t));
  t->h = 1;
  t->l = 0;
  t->r = 0;

  *p = t;

  // balance tree postorder
  int i;
  for (i = sp - 1; i >= 0; i--)
    *stack[i] = balance(*stack[i]);

  return t;
}

static inline void iap_net_from_fast(iap_ip_t *a, iap_ip_t *from) {
  unsigned int raw_a = iap_raw_fast(a) & iap_mask_fast(a->cidr);
  from->a[0] = (raw_a >> 24) & 0xff;
  from->a[1] = (raw_a >> 16) & 0xff;
  from->a[2] = (raw_a >> 8) & 0xff;
  from->a[3] = raw_a & 0xff;
  from->cidr = 32;
}

static inline void iap_net_to_fast(iap_ip_t *a, iap_ip_t *to) {
  unsigned int raw_a = iap_raw_fast(a) | ~iap_mask_fast(a->cidr);
  to->a[0] = (raw_a >> 24) & 0xff;
  to->a[1] = (raw_a >> 16) & 0xff;
  to->a[2] = (raw_a >> 8) & 0xff;
  to->a[3] = raw_a & 0xff;
  to->cidr = 32;
}

/**
 * free tree
 */
static void iap_free_fast(iap_ip_t *a) {
  if (!a)
    return;

  if (a->l)
    iap_free_fast(a->l);
  if (a->r)
    iap_free_fast(a->r);
  free(a);
}

unsigned int iap_raw(const iap_ip_t *a) { return iap_raw_fast(a); }

unsigned int iap_mask(int cidr) { return iap_mask_fast(cidr); }

int iap_in(const iap_ip_t *net, const iap_ip_t *a) {
  return iap_in_fast(net, a);
}

void iap_remove(iap_ip_t **r, const iap_ip_t *a) {
  *r = iap_remove_fast(*r, a);
}

void iap_prune(iap_ip_t **r, const iap_ip_t *net) {
  *r = iap_prune_fast(*r, net);
}

void iap_free(iap_ip_t **r) {
  iap_free_fast(*r);
  *r = (void *)0;
}

iap_ip_t *iap_begin(iap_ip_t *r, iap_ctx_t *ctx) {
  if (!r)
    return NULL;

  iap_ip_t *p = r;

  while (p) {
    ctx->stack[ctx->sp] = p;
    ctx->sp++;
    p = p->l;
  }

  ctx->sp--;

  // p now leftmost node

  return ctx->stack[ctx->sp--];
}

iap_ip_t *iap_next(iap_ctx_t *ctx) {
  if (ctx->sp < 0)
    return NULL;

  iap_ip_t *node = ctx->stack[ctx->sp];
  iap_ip_t *p = node->r;

  while (p) {
    ctx->sp++;
    ctx->stack[ctx->sp] = p;
    p = p->l;
  }

  ctx->sp--;

  return node;
}

void iap_increment(iap_ip_t *a) {
  unsigned int raw_a = iap_raw_fast(a) + 1;
  a->a[0] = (raw_a >> 24) & 0xff;
  a->a[1] = (raw_a >> 16) & 0xff;
  a->a[2] = (raw_a >> 8) & 0xff;
  a->a[3] = raw_a & 0xff;
}

void iap_net_from(iap_ip_t *a, iap_ip_t *from) { iap_net_from_fast(a, from); }

void iap_net_to(iap_ip_t *a, iap_ip_t *to) { iap_net_to_fast(a, to); }

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

static inline int iap_ip_to_a_fast(iap_ip_t *a, char *out) {
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

int iap_ip_to_a(iap_ip_t *a, char *out) { return iap_ip_to_a_fast(a, out); }
