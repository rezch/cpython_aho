#include "aho.h"


node_t* node_init() {
    node_t *node = (node_t*) malloc(sizeof(*node));
    if (node == NULL)
        return NULL;

    node->p = NULL;
    node->link = NULL;
    node->to = map_init();
    node->go = map_init();
    node->pch = 0;
    node->terminated = 0;
    node->cnt = -1;
    return node;
}

node_t* add_node(node_t *p, char pch) {
    node_t *node = node_init();
    node->p = p;
    node->pch = pch;
    return node;
}

void clear_node(node_t *node) {
    node->link = NULL;
    delete_map(node->go);
    node->go = map_init();
}

node_t* get_link(aho_t *aho, node_t *node) {
    if (node->link != NULL)
        return node->link;

    if (aho->root == node || aho->root == node->p)
        node->link = aho->root;
    else
        node->link = go(aho, get_link(aho, node->p), node->pch);

    return node->link;
}

node_t* go(aho_t *aho, node_t *node, char by) {
    if (map_contains(node->go, by))
        return map_at(node->go, by);

    if (map_contains(node->to, by))
        return map_at(node->go, by) = map_at(node->to, by);

    if (node == aho->root)
        return map_at(node->go, by) = aho->root;
    
    return map_at(node->go, by) = go(aho, get_link(aho, node), by);
}

void get_cnt(aho_t *aho, node_t *node) {
    if (node->cnt != -1)
        return;

    get_cnt(aho, get_link(aho, node));

    node->cnt = node->link->cnt + node->terminated;
}

void clear_links(node_t *node) {
    if (node == NULL)
        return;

    clear_node(node);

    map_node_t **ptr = node->to->data;
    for (size_t i = 0; i < node->to->size; ++i, ++ptr) {
        clear_links((*ptr)->ptr);
    }
}

void build_from(node_t *curr, node_t *other) {
    curr->terminated += other->terminated;

    for (size_t i = 0; i < other->to->size; ++i) {
        int key = other->to->data[i]->key;
        if (!map_contains(curr->to, key))
            map_at(curr->to, key) =
                add_node(curr, key);

        build_from(
            map_at(curr->to, key),
            map_at(other->to, key)
        );
    }
}

void rebuild(aho_t *aho, aho_t **other) {
    aho_t *other_ptr = *other;
    clear_links(aho->root);
    aho->words_count += other_ptr->words_count;
    build_from(aho->root, other_ptr->root);
    aho_delete(other_ptr);
    *other = aho_init();
}

aho_t* aho_init() {
    aho_t *aho = (aho_t*) malloc(sizeof(*aho));
    if (aho == NULL)
        return NULL;

    aho->root = node_init();
    aho->root->cnt = 0;
    aho->words_count = 0;
    return aho;
}

void aho_delete_nodes(node_t *root) {
    if (root == NULL)
        return;

    for (size_t i = 0; i < root->to->size; ++i)
        aho_delete_nodes(root->to->data[i]->ptr);

    delete_map(root->to);
    delete_map(root->go);
    free(root);
}

void aho_delete(aho_t *aho) {
    aho_delete_nodes(aho->root);
    free(aho);
}

void add(aho_t *aho, const char *str, int32_t count) {
    node_t *curr = aho->root;

    for (const char *c_ptr = str; *c_ptr != '\0'; ++c_ptr) {
        char c = *c_ptr;

        if (!map_contains(curr->to, c)) {
            map_at(curr->to, c) = add_node(curr, c);
        }

        curr = map_at(curr->to, c);
        curr->cnt = -1;
    }

    curr->terminated += count;
    ++aho->words_count;
}

int32_t count_entry(aho_t *aho, const char *str) {
    int32_t result = 0;
    node_t *curr = aho->root;

    for (const char *c = str; *c != '\0'; ++c) {
        curr = go(aho, curr, *c);
        get_cnt(aho, curr);

        result += curr->cnt;
    }

    return result;
}

dynamic_aho_t* dynamic_aho_init(uint32_t size) {
    dynamic_aho_t *aho = (dynamic_aho_t*) malloc(sizeof(*aho));
    if (aho == NULL)
        return NULL;

    aho->buckets = (aho_t**) malloc(sizeof(aho_t*)  *size);
    if (aho->buckets == NULL)
        return NULL;

    aho->size = size;
    for (size_t i = 0; i < size; ++i)
        aho->buckets[i] = aho_init();

    return aho;
}

void resize(dynamic_aho_t *aho, uint32_t size) {
    if (size < aho->size)
        for (size_t i = size; i < aho->size; ++i)
            aho_delete(aho->buckets[i]);

    aho->buckets = realloc(aho->buckets, size  *sizeof(aho_t*));

    if (aho->size < size)
        for (size_t i = aho->size; i < size; ++i)
            aho->buckets[i] = aho_init();

    aho->size = size;
}

void dynamic_aho_delete(dynamic_aho_t *aho) {
    for (size_t i = 0; i < aho->size; ++i)
        aho_delete(aho->buckets[i]);
}

void insert(dynamic_aho_t *aho, const char *str, int32_t count) {
    add(aho->buckets[0], str, count);

    size_t empty_bucket = 0;
    for (; empty_bucket < aho->size; ++empty_bucket)
        if (aho->buckets[empty_bucket]->words_count == 0)
            break;

    if (empty_bucket == aho->size)
        resize(aho, aho->size << 1);

    for (size_t i = 0; i < empty_bucket; ++i)
        rebuild(aho->buckets[empty_bucket], &aho->buckets[i]);
}

int32_t request(dynamic_aho_t *aho, const char *str) {
    int32_t result = 0;

    for (size_t i = 0; i < aho->size; ++i)
        result += count_entry(aho->buckets[i], str);

    return result;
}
