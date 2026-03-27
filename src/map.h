#pragma once

#include <stdlib.h>
#include <stdbool.h>


struct map_node_t {
    int key;
    void *value;
} typedef map_node_t;

struct map_t {
    size_t size;
    size_t capacity;
    map_node_t **data;
} typedef map_t;

map_t* map_init();

void delete_map(map_t *map);

bool map_contains(map_t *map, int key);

void **map_at(map_t *map, int key);
