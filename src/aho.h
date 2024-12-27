#pragma once

#include "map.h"

#include <stdint.h>


struct node_t {
    struct node_t *p;
    struct node_t *link;
    map_t* to;
    map_t* go;
    char pch;
    int32_t terminated;
    int32_t cnt;
} typedef node_t;


struct aho_t {
    node_t *root;
    uint32_t words_count;
} typedef aho_t;


struct dynamic_aho_t {
    uint32_t size;
    aho_t **buckets;
} typedef dynamic_aho_t;


static node_t* node_init();

static node_t* add_node(node_t *p, char pch);

static int clear_node(node_t *node);

static node_t* get_link(aho_t *aho, node_t *node);

static node_t* go(aho_t *aho, node_t *node, char by);

static int get_cnt(aho_t *aho, node_t *node);

static void aho_delete_nodes(node_t *root);

static int clear_links(node_t *node);

static int build_from(node_t *curr, node_t *other);

static int rebuild(aho_t *aho, aho_t **other);


// ------ USER FUNCTIONS ------
aho_t* aho_init();

void aho_delete(aho_t *aho);

int add(aho_t *aho, const char *str, int32_t count);

int32_t count_entry(aho_t *aho, const char *str);

dynamic_aho_t* dynamic_aho_init(uint32_t size);

void dynamic_aho_delete(dynamic_aho_t *aho);

int resize(dynamic_aho_t *aho, uint32_t size);

int insert(dynamic_aho_t *aho, const char *str, int32_t count);

int32_t request(dynamic_aho_t *aho, const char *str);
