#ifndef NODE_H
#define NODE_H

#include "maglev.h"

// Node management functions
Node* node_create(const char *name, uint32_t table_size);
void node_destroy(Node *node);
void node_generate_preference_list(Node *node, uint32_t table_size);
void node_reset_index(Node *node);

#endif // NODE_H