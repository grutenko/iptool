#include "ip.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static inline unsigned int iap_mask_fast(int cidr) {
  return ~0U << (32 - cidr);
}

static inline unsigned int iap_raw_fast(const iap_ip_t *a) {
  return (a->a[0] << 24) | (a->a[1] << 16) | (a->a[2] << 8) | a->a[3];
}

static inline int iap_in_fast(const iap_ip_t *net, const iap_ip_t *a) {
  return net->cidr <= a->cidr &&
         (iap_raw_fast(net) & iap_mask_fast(net->cidr)) ==
             (iap_raw_fast(a) & iap_mask_fast(net->cidr));
}

static inline int max(int a, int b) { return a > b ? a : b; }

static inline int height(iap_ip_t *a) { return a ? a->h : 0; }

static inline int bfactor(iap_ip_t *node) {
  return height(node->l) - height(node->r);
}

static inline iap_ip_t *rotl(iap_ip_t *x) {
  iap_ip_t *y = x->r;
  iap_ip_t *T2 = y->l;

  y->l = x;
  x->r = T2;

  x->h = max(height(x->l), height(x->r)) + 1;
  y->h = max(height(y->l), height(y->r)) + 1;

  return y;
}

static inline iap_ip_t *rotr(iap_ip_t *y) {
  iap_ip_t *x = y->l;
  iap_ip_t *T2 = x->r;

  x->r = y;
  y->l = T2;

  y->h = max(height(y->l), height(y->r)) + 1;
  x->h = max(height(x->l), height(x->r)) + 1;

  return x;
}

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

static inline int iap_key_cmp_fast(const iap_ip_t *a, const iap_ip_t *b) {
  unsigned int raw_a, raw_b, min_cidr;
  min_cidr = a->cidr > b->cidr ? b->cidr : a->cidr;
  raw_a = iap_raw_fast(a) & iap_mask_fast(min_cidr);
  raw_b = iap_raw_fast(b) & iap_mask_fast(min_cidr);
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

static inline int iap_key_cmp_strict_fast(const iap_ip_t *a,
                                          const iap_ip_t *b) {
  unsigned int raw_a, raw_b;
  raw_a = iap_raw_fast(a) & a->cidr;
  raw_b = iap_raw_fast(b) & b->cidr;
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

iap_ip_t *iap_set_insert(iap_ip_t **r, const iap_ip_t *a) {
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

    iap_prune(r, a);
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

static void iap_free_fast(iap_ip_t *a) {
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
