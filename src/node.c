#include "node.h"
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create new node
Node* node_create(const char *name, uint32_t table_size) {
    if (!name || strlen(name) >= MAX_NODE_NAME_LEN) {
        return NULL;
    }

    Node *node = malloc(sizeof(Node));
    if (!node) {
        return NULL;
    }

    // Initialize basic node information
    strncpy(node->name, name, MAX_NODE_NAME_LEN - 1);
    node->name[MAX_NODE_NAME_LEN - 1] = '\0';
    node->is_active = true;
    node->next_index = 0;
    node->color_index = assign_unique_color_index();

    // Allocate preference list memory
    node->preference_list = malloc(table_size * sizeof(uint32_t));
    if (!node->preference_list) {
        free(node);
        return NULL;
    }

    // Generate preference list
    node_generate_preference_list(node, table_size);

    return node;
}

// Destroy node
void node_destroy(Node *node) {
    if (node) {
        if (node->preference_list) {
            free(node->preference_list);
        }
        free(node);
    }
}

// Generate node's preference list
void node_generate_preference_list(Node *node, uint32_t table_size) {
    if (!node || !node->preference_list) {
        return;
    }

    uint32_t offset = hash_offset(node->name, table_size);
    uint32_t skip = hash_skip(node->name, table_size);

    // Generate preference list: traverse entire table starting from offset with skip step
    for (uint32_t i = 0; i < table_size; i++) {
        node->preference_list[i] = (offset + i * skip) % table_size;
    }
}

// Reset node's index pointer
void node_reset_index(Node *node) {
    if (node) {
        node->next_index = 0;
    }
}