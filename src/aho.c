#include "aho.h"
#include "map.h"


static node_t* node_init() {
    node_t *node = (node_t*) malloc(sizeof(*node));
    if (node == NULL)
        return NULL;

    node->p = NULL;
    node->link = NULL;

    if ((node->to = map_init()) == NULL)
        return NULL;

    if ((node->go = map_init()) == NULL)
        return NULL;

    node->pch = 0;
    node->terminated = 0;
    node->cnt = -1;

    return node;
}

static node_t* add_node(node_t *p, char pch) {
    node_t *node = node_init();
    if (node == NULL)
        return NULL;

    node->p = p;
    node->pch = pch;

    return node;
}

static int clear_node(node_t *node) {
    node->link = NULL;
    delete_map(node->go);

    if ((node->go = map_init()) == NULL)
        return -1;

    return 0;
}

static node_t* get_link(aho_t *aho, node_t *node) {
    if (node->link != NULL)
        return node->link;

    if (aho->root == node || aho->root == node->p)
        node->link = aho->root;
    else
        node->link = go(aho, get_link(aho, node->p), node->pch);

    return node->link;
}

static node_t* go(aho_t *aho, node_t *node, char by) {
    if (map_contains(node->go, by))
        return map_at(node->go, by)->value;

    map_node_t *go_node = map_at(node->go, by);
    if (go_node == NULL)
        return NULL;

    if (map_contains(node->to, by)) {
        return go_node->value = map_at(node->to, by)->value;
    }

    if (node == aho->root)
        return go_node->value = aho->root;
    
    return go_node->value = go(aho, get_link(aho, node), by);
}

static int get_cnt(aho_t *aho, node_t *node) {
    if (node->cnt != -1)
        return 0;

    node_t *link = get_link(aho, node);
    if (link == NULL)
        return -1;

    if (get_cnt(aho, link) == -1)
        return -1;

    node->cnt = node->link->cnt + node->terminated;

    return 0;
}

static void aho_delete_nodes(node_t *root) {
    if (root == NULL)
        return;

    for (size_t i = 0; i < root->to->size; ++i)
        aho_delete_nodes(root->to->data[i]->value);

    delete_map(root->to);
    delete_map(root->go);
    free(root);
}

static int clear_links(node_t *node) {
    if (node == NULL)
        return - 1;

    if (clear_node(node) == -1)
        return -1;

    map_node_t **ptr = node->to->data;
    for (size_t i = 0; i < node->to->size; ++i, ++ptr)
        if (clear_links((*ptr)->value) == -1)
            return -1;

    return 0;
}

static int build_from(node_t *curr, node_t *other) {
    curr->terminated += other->terminated;

    for (size_t i = 0; i < other->to->size; ++i) {
        int key = other->to->data[i]->key;
        bool contains = map_contains(curr->to, key);
        map_node_t *curr_node = map_at(curr->to, key);
        if (curr_node == NULL)
            return -1;

        if (!contains) {
            curr_node->value = add_node(curr, key);
        }

        map_node_t *other_node = map_at(other->to, key);
        if (other_node == NULL)
            return -1;

        if (build_from(curr_node->value, other_node->value) == -1)
            return -1;
    }

    return 0;
}

static int rebuild(aho_t *aho, aho_t **other) {
    aho_t *other_ptr = *other;

    if (clear_links(aho->root) == -1)
        return -1;

    aho->words_count += other_ptr->words_count;

    if (build_from(aho->root, other_ptr->root) == -1)
        return -1;

    aho_delete(other_ptr);

    if ((*other = aho_init()) == NULL)
        return -1;

    return 0;
}

aho_t* aho_init() {
    aho_t *aho = (aho_t*) malloc(sizeof(*aho));
    if (aho == NULL)
        return NULL;

    if ((aho->root = node_init()) == NULL)
        return NULL;

    aho->root->cnt = 0;
    aho->words_count = 0;

    return aho;
}

void aho_delete(aho_t *aho) {
    aho_delete_nodes(aho->root);
    free(aho);
}

int add(aho_t *aho, const char *str, int32_t count) {
    node_t *curr = aho->root;

    for (const char *c_ptr = str; *c_ptr != '\0'; ++c_ptr) {
        char c = *c_ptr;
        bool contains = map_contains(curr->to, c);
        map_node_t *next = map_at(curr->to, c);
        if (next == NULL)
            return -1;

        if (!contains)
            next->value = add_node(curr, c);

        curr = next->value;
        curr->cnt = -1;
    }

    curr->terminated += count;
    ++aho->words_count;

    return 0;
}

int32_t count_entry(aho_t *aho, const char *str) {
    int32_t result = 0;
    node_t *curr = aho->root;

    for (const char *c = str; *c != '\0'; ++c) {
        curr = go(aho, curr, *c);
        if (curr == NULL)
            return -1;

        if (get_cnt(aho, curr) == -1)
            return -1;

        result += curr->cnt;
    }

    return result;
}

dynamic_aho_t* dynamic_aho_init(uint32_t size) {
    dynamic_aho_t *aho = (dynamic_aho_t*) malloc(sizeof(*aho));
    if (aho == NULL)
        return NULL;

    aho->buckets = (aho_t**) malloc(sizeof(aho_t*) * size);
    if (aho->buckets == NULL)
        return NULL;

    aho->size = size;
    for (size_t i = 0; i < size; ++i)
        if ((aho->buckets[i] = aho_init()) == NULL)
            return NULL;

    return aho;
}

void dynamic_aho_delete(dynamic_aho_t *aho) {
    for (size_t i = 0; i < aho->size; ++i)
        aho_delete(aho->buckets[i]);
}

int resize(dynamic_aho_t *aho, uint32_t size) {
    if (size < aho->size)
        for (size_t i = size; i < aho->size; ++i)
            aho_delete(aho->buckets[i]);

    aho->buckets = realloc(aho->buckets, size * sizeof(aho_t*));
    if (aho->buckets == NULL)
        return -1;

    if (aho->size < size)
        for (size_t i = aho->size; i < size; ++i)
            if ((aho->buckets[i] = aho_init()) == NULL)
                return -1;

    aho->size = size;

    return 0;
}

int insert(dynamic_aho_t *aho, const char *str, int32_t count) {
    if (add(aho->buckets[0], str, count) == -1)
        return -1;

    size_t empty_bucket = 0;
    for (; empty_bucket < aho->size; ++empty_bucket)
        if (aho->buckets[empty_bucket]->words_count == 0)
            break;

    if (empty_bucket == aho->size)
        if (resize(aho, aho->size << 1) == -1)
            return -1;

    for (size_t i = 0; i < empty_bucket; ++i)
        if (rebuild(aho->buckets[empty_bucket], &aho->buckets[i]) == -1)
            return -1;

    return 0;
}

int32_t request(dynamic_aho_t *aho, const char *str) {
    int32_t result = 0;

    for (size_t i = 0; i < aho->size; ++i) {
        int32_t count = count_entry(aho->buckets[i], str);
        if (count == -1)
            return -1;

        result += count;
    }

    return result;
}
