#include "aho.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>


bool alpha(char c) {
    return MIN_ALPHA <= c && c < MIN_ALPHA + ALPHA_SIZE;
}

node_t* node_init() {
    node_t* node = (node_t*) malloc(sizeof(*node));
    node->p = NULL;
    node->link = NULL;
    for (size_t i = 0; i < ALPHA_SIZE; ++i) {
        node->to[i] = NULL;
        node->go[i] = NULL;
    }
    node->pch = '#';
    node->terminated = 0;
    node->cnt = -1;
    return node;
}

node_t* add_node(node_t* p, char pch) {
    node_t* node = node_init();
    node->p = p;
    node->pch = pch;
    return node;
}

void clear_node(node_t* node) {
    node->link = NULL;
    for (size_t i = 0; i < ALPHA_SIZE; ++i)
        node->go[i] = NULL;
}

node_t* get_link(aho_t* aho, node_t* node) {
    if (node->link != NULL)
        return node->link;

    if (aho->root == node || aho->root == node->p)
        node->link = aho->root;
    else
        node->link = go(aho, get_link(aho, node->p), node->pch);

    return node->link;
}

node_t* go(aho_t* aho, node_t* node, char by) {
    size_t ind = (size_t)(by - MIN_ALPHA);

    if (node->go[ind] != NULL)
        return node->go[ind];

    if (node->to[ind] != NULL)
        return node->go[ind] = node->to[ind];
    
    if (node == aho->root)
        return node->go[ind] = aho->root;
    
    return node->go[ind] = go(aho, get_link(aho, node), by);
}

void get_cnt(aho_t* aho, node_t* node) {
    if (node->cnt != -1)
        return;

    get_cnt(aho, get_link(aho, node));

    node->cnt = node->link->cnt + node->terminated;
}

void clear_links(node_t* node) {
    if (node == NULL)
        return;

    clear_node(node);

    for (size_t i = 0; i < ALPHA_SIZE; ++i) {
        clear_links(node->to[i]);
    }
}

void build_from(node_t* curr, node_t* other) {
    curr->terminated += other->terminated;

    for (size_t i = 0; i < ALPHA_SIZE; ++i) {
        if (other->to[i] == NULL)
            continue;

        if (curr->to[i] == NULL)
            curr->to[i] = add_node(curr, i + MIN_ALPHA);

        build_from(curr->to[i], other->to[i]);
    }
}

void rebuild(aho_t* aho, aho_t** other) {
    aho_t* other_ptr = *other;
    clear_links(aho->root);
    aho->words_count += other_ptr->words_count;
    build_from(aho->root, other_ptr->root);
    aho_delete(other_ptr);
    *other = aho_init();
}

aho_t* aho_init() {
    aho_t* aho = (aho_t*) malloc(sizeof(*aho));
    aho->root = node_init();
    aho->root->cnt = 0;
    aho->words_count = 0;
    return aho;
}

void aho_delete_nodes(node_t* root) {
    if (root == NULL)
        return;

    for (size_t i = 0; i < ALPHA_SIZE; ++i)
        aho_delete_nodes(root->to[i]);

    free(root);
}

void aho_delete(aho_t* aho) {
    aho_delete_nodes(aho->root);
    free(aho);
}

void add(aho_t* aho, const char* str, int32_t count) {
    node_t* curr = aho->root;

    for (const char* c = str; *c != '\0'; ++c) {
        if (!alpha(*c))
            continue;

        size_t ind = (size_t)(*c - MIN_ALPHA);

        if (curr->to[ind] == NULL)
            curr->to[ind] = add_node(curr, *c);

        curr = curr->to[ind];
        curr->cnt = -1;
    }

    curr->terminated += count;
    ++aho->words_count;
}

uint32_t count_entry(aho_t* aho, const char* str) {
    uint32_t result = 0;
    node_t* curr = aho->root;

    for (const char* c = str; *c != '\0'; ++c) {
        if (!alpha(*c))
            continue;

        curr = go(aho, curr, *c);
        get_cnt(aho, curr);

        result += curr->cnt;
    }

    return result;
}

dynamic_aho_t* dynamic_aho_init(uint32_t size) {
    dynamic_aho_t* aho = (dynamic_aho_t*) malloc(sizeof(*aho));
    aho->buckets = (aho_t**) malloc(sizeof(aho_t*) * size);

    aho->size = size;
    for (size_t i = 0; i < size; ++i)
        aho->buckets[i] = aho_init();

    return aho;
}

void dynamic_aho_delete(dynamic_aho_t* aho) {
    for (size_t i = 0; i < aho->size; ++i)
        aho_delete(aho->buckets[i]);
}

void insert(dynamic_aho_t* aho, const char* str, int32_t count) {
    add(aho->buckets[0], str, count);

    size_t empty_bucket = 0;
    for (; empty_bucket < aho->size; ++empty_bucket)
        if (aho->buckets[empty_bucket]->words_count == 0)
            break;

    for (size_t i = 0; i < empty_bucket; ++i)
        rebuild(aho->buckets[empty_bucket], &aho->buckets[i]);
}

int32_t request(dynamic_aho_t* aho, const char* str) {
    int32_t result = 0;

    for (size_t i = 0; i < aho->size; ++i)
        result += count_entry(aho->buckets[i], str);

    return result;
}
