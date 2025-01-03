#include "map.h"


static map_node_t* map_node_init() {
    map_node_t *node = (map_node_t*) malloc(sizeof(map_node_t));
    if (node == NULL)
        return NULL;
    node->value = NULL;
    return node;
}

static int reserve_map(map_t *map, size_t capacity) {
    if (capacity < map->size)
        return -1;

    if (capacity <= map->capacity) {
        for (size_t i = capacity; i < map->capacity; ++i) {
            if (map->data[i]->value != NULL)
                free(map->data[i]->value);
            free(map->data[i]);
        }

        map->capacity = capacity;
        return 0;
    }

    map->data = (map_node_t**) realloc(map->data, capacity * sizeof(map_node_t**));
    if (map->data == NULL)
        return -1;

    for (size_t i = map->capacity; i < capacity; ++i) {
        map->data[i] = map_node_init();
        if (map->data[i] == NULL)
            return -1;
    }

    map->capacity = capacity;
    return 0;
}

static int resize_map(map_t *map, size_t size) {
    if (size > map->capacity
        && reserve_map(map, size << 1) < 0) {
        return -1;
    }

    map->size = size;
    return 0;
}

map_t* map_init() {
    map_t *map = (map_t*) malloc(sizeof(map_t));
    if (map == NULL)
        return NULL;

    map->size = 0;
    map->capacity = 2;
    map->data = (map_node_t**) malloc(sizeof(map_node_t*) * map->capacity);
    if (map->data == NULL)
        return NULL;

    for (size_t i = 0; i < map->capacity; ++i) {
        map->data[i] = map_node_init();
        if (map->data[i] == NULL)
            return NULL;
    }

    return map;
}

void delete_map(map_t *map) {
    for (size_t i = 0; i < map->capacity; ++i)
        free(map->data[i]);

    free(map->data);
    free(map);
}

bool map_contains(map_t *map, int key) {
    for (size_t i = 0; i < map->size; ++i)
        if (map->data[i]->key == key)
            return true;
    return false;
}

map_node_t* map_at(map_t *map, int key) {
    for (size_t i = 0; i < map->size; ++i)
        if (map->data[i]->key == key)
            return map->data[i];

    if (resize_map(map, map->size + 1) < 0)
        return NULL;

    if (map->data[map->size - 1] == NULL)
        map->data[map->size - 1] = map_node_init();

    map->data[map->size - 1]->key = key;
    return map->data[map->size - 1];
}
