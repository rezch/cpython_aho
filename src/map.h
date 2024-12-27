#pragma once

#include <stdlib.h>
#include <stdbool.h>


struct map_node_t {
    int key;
    void *ptr;
} typedef map_node_t;

struct map_t {
    size_t size;
    size_t capacity;
    map_node_t **data;
} typedef map_t;


static map_node_t* map_node_init();

static int reserve_map(map_t *map, size_t capacity);

static int resize_map(map_t *map, size_t size);


// ------ USER FUNCTIONS ------
map_t* map_init();

void delete_map(map_t *map);

bool map_contains(map_t *map, int key);

map_node_t* map_at(map_t *map, int key);
