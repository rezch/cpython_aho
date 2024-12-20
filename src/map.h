#pragma once

#include <stdlib.h>
#include <stdbool.h>

#define map_at(map, key) \
        _map_at(map, key)->ptr


struct map_node_t {
    int key;
    void *ptr;
} typedef map_node_t;

struct map_t {
    size_t size;
    size_t capacity;
    map_node_t **data;
} typedef map_t;


map_t* map_init();

map_node_t* map_node_init();

int reserve_map(map_t *map, size_t capacity);

int resize_map(map_t *map, size_t size);

void delete_map(map_t *map);

map_node_t* _map_at(map_t *map, int key);

bool map_contains(map_t *map, int key);
