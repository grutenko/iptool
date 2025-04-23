#include "cmd.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>

struct list_item {
  unsigned int ip;
  unsigned int cidr;
  struct list_item *prev, *next;
};

struct list {
  struct list_item *head, *tail;
};

static struct list_item *list_alloc() {
  struct list_item *item = malloc(sizeof(struct list_item));
  if (item) {
    item->prev = item->next = NULL;
  }
  return item;
}

static void list_push(struct list *l, struct list_item *i) {
  if (!l->head)
    l->head = l->tail = i;
  else {
    i->prev = l->tail;
    l->tail->next = i->next;
    l->tail = i;
  }
}

static void list_remove(struct list *l, struct list_item *i) {
  if (i->prev)
    i->prev->next = i->next;
  if (i->next)
    i->next->prev = i->prev;
  if (i == l->tail)
    l->tail = i->prev;
  if (i == l->head)
    l->head = i->next;
  free(i);
}

static void list_free(struct list *l) {
  struct list_item *p = l->head, *t;
  while (p) {
    t = p->next;
    free(p);
    p = t;
  }
  l->head = NULL;
  l->tail = NULL;
}

static inline unsigned int network(unsigned int a, int cidr) {
  return cidr > 0 ? a & (~0U << (32 - cidr)) : 0;
}

static void walk(const iap_t *a, int depth, int mode, void *data) {
  if(mode != IAP_WALK_INORDER)
    return;

  struct list *l = (struct list *)data;
  struct list_item *i = list_alloc();
  if (!i) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  i->ip = (a->a[0] << 24) | (a->a[1] << 16) | (a->a[2] << 8) | a->a[3];
  i->cidr = a->cidr;
  list_push(l, i);
}

int cmd_deflate(int argc, char **argv) {
  // Implementation of the deflate command
  iap_t *root = (void *)0;
  struct list list = {0};
  int i;
  struct list_item *p, *q, *t;
  int ok = 0;

  // TODO: maybe parse options

  parse_ips(argc, argv, &root);

  iap_walk(root, walk, (void *)&list);
  iap_free(&root);

  for (i = 32; i >= 0; i--) {
    p = list.head;
    while (p) {
      if (p->cidr < i)
        continue;

      ok = 0;
      q = p->next;

      while (q && network(p->ip, i) == network(q->ip, i)) {
        if (q->cidr < i)
          continue;

        ok = 1;
        t = q->next;
        list_remove(&list, q);
        q = t;
      }

      if (ok) {
        p->ip = network(p->ip, i);
        p->cidr = i;
      }

      p = p->next;
    }
  }

  p = list.head;
  while (p) {
    // print
    p = p->next;
  }

  return 0;
}

void cmd_deflate_help() {}
