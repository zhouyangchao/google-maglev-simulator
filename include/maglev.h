#ifndef MAGLEV_H
#define MAGLEV_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_NODE_NAME_LEN 256
#define MAX_NODES 1000
#define DEFAULT_TABLE_SIZE 65537

typedef struct {
    char name[MAX_NODE_NAME_LEN];
    bool is_active;
    uint32_t *preference_list;  // Preference list
    uint32_t next_index;        // Next index position to try
    int color_index;            // Index in color array for display
} Node;

typedef struct {
    Node *nodes[MAX_NODES];     // Node array
    uint32_t node_count;        // Current node count
    uint32_t *lookup_table;     // Lookup table
    uint32_t table_size;        // Lookup table size
    bool is_initialized;        // Whether initialized
} MaglevTable;

// Global Maglev table instance
extern MaglevTable g_maglev;

// Core functions
bool maglev_init(uint32_t table_size);
void maglev_cleanup(void);
bool maglev_add_node(const char *node_name);
bool maglev_remove_node(const char *node_name);
void maglev_rebuild_table(void);
void maglev_show_nodes(void);
void maglev_show_table(void);
void maglev_show_table_colored(void);

// Helper functions
int find_node_index(const char *node_name);
bool is_prime(uint32_t n);
uint32_t next_prime(uint32_t n);
int get_max_node_name_length(void);

// Color functions
int assign_unique_color_index(void);
void print_colored_text(const char *text, int color_index);

#endif // MAGLEV_H