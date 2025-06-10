#include "maglev.h"
#include "node.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Global Maglev table instance
MaglevTable g_maglev = {0};

// Check if a number is prime
bool is_prime(uint32_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;

    for (uint32_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Find the next prime number
uint32_t next_prime(uint32_t n) {
    while (!is_prime(n)) {
        n++;
    }
    return n;
}

// Initialize Maglev table
bool maglev_init(uint32_t table_size) {
    // Clean up existing resources
    maglev_cleanup();

    // Ensure table size is prime
    if (table_size < 2) {
        table_size = DEFAULT_TABLE_SIZE;
    } else {
        table_size = next_prime(table_size);
    }

    // Allocate lookup table memory
    g_maglev.lookup_table = calloc(table_size, sizeof(uint32_t));
    if (!g_maglev.lookup_table) {
        return false;
    }

    // Initialize lookup table to invalid values
    for (uint32_t i = 0; i < table_size; i++) {
        g_maglev.lookup_table[i] = UINT32_MAX;
    }

    g_maglev.table_size = table_size;
    g_maglev.node_count = 0;
    g_maglev.is_initialized = true;

    printf("Maglev table initialized with size: %u\n", table_size);
    return true;
}

// Clean up Maglev table
void maglev_cleanup(void) {
    if (!g_maglev.is_initialized) {
        return;
    }

    // Free all nodes
    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        if (g_maglev.nodes[i]) {
            node_destroy(g_maglev.nodes[i]);
            g_maglev.nodes[i] = NULL;
        }
    }

    // Free lookup table
    if (g_maglev.lookup_table) {
        free(g_maglev.lookup_table);
        g_maglev.lookup_table = NULL;
    }

    g_maglev.node_count = 0;
    g_maglev.table_size = 0;
    g_maglev.is_initialized = false;
}

// Find node index
int find_node_index(const char *node_name) {
    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        if (g_maglev.nodes[i] && strcmp(g_maglev.nodes[i]->name, node_name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

// Get maximum node name length
int get_max_node_name_length(void) {
    int max_len = 1; // At least 1, used to display "-"

    if (g_maglev.is_initialized) {
        for (uint32_t i = 0; i < g_maglev.node_count; i++) {
            if (g_maglev.nodes[i]) {
                int len = strlen(g_maglev.nodes[i]->name);
                if (len > max_len) {
                    max_len = len;
                }
            }
        }
    }

    // Minimum width is 8, maximum width is 20
    if (max_len < 8) max_len = 8;
    if (max_len > 20) max_len = 20;

    return max_len;
}

// Add node
bool maglev_add_node(const char *node_name) {
    if (!g_maglev.is_initialized) {
        printf("Error: Maglev table not initialized\n");
        return false;
    }

    if (!node_name || strlen(node_name) == 0) {
        printf("Error: Invalid node name\n");
        return false;
    }

    // Check if node already exists
    if (find_node_index(node_name) >= 0) {
        printf("Error: Node '%s' already exists\n", node_name);
        return false;
    }

    // Check if maximum number of nodes is exceeded
    if (g_maglev.node_count >= MAX_NODES) {
        printf("Error: Maximum number of nodes reached\n");
        return false;
    }

    // Create new node
    Node *new_node = node_create(node_name, g_maglev.table_size);
    if (!new_node) {
        printf("Error: Failed to create node '%s'\n", node_name);
        return false;
    }

    // Add to node array
    g_maglev.nodes[g_maglev.node_count] = new_node;
    g_maglev.node_count++;

    // Rebuild lookup table
    maglev_rebuild_table();

    printf("Node '%s' added successfully\n", node_name);
    return true;
}

// Remove node
bool maglev_remove_node(const char *node_name) {
    if (!g_maglev.is_initialized) {
        printf("Error: Maglev table not initialized\n");
        return false;
    }

    int index = find_node_index(node_name);
    if (index < 0) {
        // Ignore non-existent nodes (as required)
        printf("Node '%s' does not exist (ignored)\n", node_name);
        return true;
    }

    // Destroy node
    node_destroy(g_maglev.nodes[index]);

    // Move array elements
    for (uint32_t i = index; i < g_maglev.node_count - 1; i++) {
        g_maglev.nodes[i] = g_maglev.nodes[i + 1];
    }
    g_maglev.nodes[g_maglev.node_count - 1] = NULL;
    g_maglev.node_count--;

    // Rebuild lookup table
    maglev_rebuild_table();

    printf("Node '%s' removed successfully\n", node_name);
    return true;
}

// Rebuild lookup table (Core Maglev algorithm)
void maglev_rebuild_table(void) {
    if (!g_maglev.is_initialized || g_maglev.node_count == 0) {
        // Clear table
        for (uint32_t i = 0; i < g_maglev.table_size; i++) {
            g_maglev.lookup_table[i] = UINT32_MAX;
        }
        return;
    }

    // Reset all nodes' index pointers
    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        node_reset_index(g_maglev.nodes[i]);
    }

    // Clear lookup table
    for (uint32_t i = 0; i < g_maglev.table_size; i++) {
        g_maglev.lookup_table[i] = UINT32_MAX;
    }

    // Standard Maglev algorithm: round-robin assignment
    uint32_t filled = 0;

    // Keep polling until all positions are filled
    while (filled < g_maglev.table_size) {
        // In each round, every node tries to get the next position from its preference list
        for (uint32_t i = 0; i < g_maglev.node_count; i++) {
            Node *node = g_maglev.nodes[i];
            if (!node || !node->is_active) continue;

            // If this node still has untried preference positions
            while (node->next_index < g_maglev.table_size) {
                uint32_t preferred_slot = node->preference_list[node->next_index];
                node->next_index++;

                // If this position is free, assign it to the current node
                if (g_maglev.lookup_table[preferred_slot] == UINT32_MAX) {
                    g_maglev.lookup_table[preferred_slot] = i;
                    filled++;
                    break; // This node got a position in this round, move to next node
                }
            }

            // If all positions are filled, exit early
            if (filled >= g_maglev.table_size) {
                break;
            }
        }
    }
}

// Show current node status
void maglev_show_nodes(void) {
    if (!g_maglev.is_initialized) {
        printf("Maglev table not initialized\n");
        return;
    }

    printf("Current nodes (%u total):\n", g_maglev.node_count);
    if (g_maglev.node_count == 0) {
        printf("  (no nodes)\n");
        return;
    }

    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        if (g_maglev.nodes[i]) {
            printf("  %u: %s\n", i, g_maglev.nodes[i]->name);
        }
    }
}

// Show complete Maglev table status
void maglev_show_table(void) {
    if (!g_maglev.is_initialized) {
        printf("Maglev table not initialized\n");
        return;
    }

    printf("Maglev lookup table (size: %u):\n", g_maglev.table_size);

    if (g_maglev.node_count == 0) {
        printf("  (empty - no nodes)\n");
        return;
    }

    // Count assignment for each node
    uint32_t *node_counts = calloc(g_maglev.node_count, sizeof(uint32_t));
    if (!node_counts) {
        printf("Error: Memory allocation failed\n");
        return;
    }

    uint32_t unassigned = 0;

    for (uint32_t i = 0; i < g_maglev.table_size; i++) {
        if (g_maglev.lookup_table[i] == UINT32_MAX) {
            unassigned++;
        } else if (g_maglev.lookup_table[i] < g_maglev.node_count) {
            node_counts[g_maglev.lookup_table[i]]++;
        }
    }

    // Show statistics
    printf("Distribution summary:\n");
    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        if (g_maglev.nodes[i]) {
            printf("  %s: %u slots (%.2f%%)\n",
                   g_maglev.nodes[i]->name,
                   node_counts[i],
                   100.0 * node_counts[i] / g_maglev.table_size);
        }
    }

    if (unassigned > 0) {
        printf("  Unassigned: %u slots (%.2f%%)\n",
               unassigned, 100.0 * unassigned / g_maglev.table_size);
    }

    // Show detailed assignment for first 100 slots (or all if table is smaller)
    uint32_t show_count = (g_maglev.table_size < 100) ? g_maglev.table_size : 100;
    int field_width = get_max_node_name_length();
    int items_per_line = (field_width <= 10) ? 10 : 8;  // Adjust items per line based on name length

    printf("\nFirst %u slots:\n", show_count);

    for (uint32_t i = 0; i < show_count; i++) {
        if (i % items_per_line == 0) {
            printf("\n%4u: ", i);
        }

        if (g_maglev.lookup_table[i] == UINT32_MAX) {
            printf("%*s ", field_width, "-");
        } else if (g_maglev.lookup_table[i] < g_maglev.node_count && g_maglev.nodes[g_maglev.lookup_table[i]]) {
            printf("%*s ", field_width, g_maglev.nodes[g_maglev.lookup_table[i]]->name);
        } else {
            printf("%*s ", field_width, "?");
        }
    }
    printf("\n");

    if (g_maglev.table_size > 100) {
        printf("... (showing first 100 out of %u total slots)\n", g_maglev.table_size);
    }

    free(node_counts);
}

// Predefined color array - includes more 256-color mode colors
static int color_palette[] = {
    // Standard 16 colors (avoid black and dark colors)
    31, 32, 33, 34, 35, 36, 37,         // red, green, yellow, blue, magenta, cyan, white
    91, 92, 93, 94, 95, 96, 97,         // bright red, bright green, bright yellow, bright blue, bright magenta, bright cyan, bright white

    // Selected 256-color mode colors (using 38;5;n format)
    // Red series
    196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
    // Green series
    46, 47, 48, 49, 50, 82, 83, 84, 85, 86,
    // Yellow series
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
    // Blue series
    21, 26, 27, 32, 33, 38, 39, 44, 45, 75,
    // Magenta/Pink series
    207, 213, 219, 225, 165, 171, 177, 183, 189, 195,
    // Cyan series
    51, 87, 123, 159, 14, 80, 116, 152, 188, 194,
    // Purple series
    129, 135, 141, 147, 153, 93, 99, 105, 111, 117,
    // Orange series
    166, 172, 178, 184, 190, 208, 214, 215, 216, 217,
    // Gray series
    244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
    // Special colors
    11, 12, 13, 14, 15, 76, 77, 78, 79, 118, 119, 120, 121, 122
};

// Assign unique color index
int assign_unique_color_index(void) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    int color_count = sizeof(color_palette) / sizeof(color_palette[0]);

    // Create array to mark used colors
    bool used_colors[color_count];
    for (int i = 0; i < color_count; i++) {
        used_colors[i] = false;
    }

    // Check colors used by existing nodes
    if (g_maglev.is_initialized) {
        for (uint32_t i = 0; i < g_maglev.node_count; i++) {
            if (g_maglev.nodes[i] && g_maglev.nodes[i]->color_index >= 0 && g_maglev.nodes[i]->color_index < color_count) {
                used_colors[g_maglev.nodes[i]->color_index] = true;
            }
        }
    }

    // First try to assign unused colors
    int available_colors[color_count];
    int available_count = 0;

    for (int i = 0; i < color_count; i++) {
        if (!used_colors[i]) {
            available_colors[available_count++] = i;
        }
    }

    // If there are unused colors, randomly select one
    if (available_count > 0) {
        int chosen_index = available_colors[rand() % available_count];
        return chosen_index;
    }

    // If all colors are used, randomly select one (when node count exceeds color count)
    return rand() % color_count;
}

// Print colored text
void print_colored_text(const char *text, int color_index) {
    int color_count = sizeof(color_palette) / sizeof(color_palette[0]);

    if (color_index < 0 || color_index >= color_count) {
        printf("%s", text);  // If index is invalid, print text without color
        return;
    }

    int color_code = color_palette[color_index];

    // Determine if it's traditional 16-color or 256-color
    if (color_code <= 97) {
        // Traditional 16-color format
        printf("\033[%dm%s\033[0m", color_code, text);
    } else {
        // 256-color format
        printf("\033[38;5;%dm%s\033[0m", color_code, text);
    }
}

// Show colored Maglev table
void maglev_show_table_colored(void) {
    if (!g_maglev.is_initialized) {
        printf("Maglev table not initialized\n");
        return;
    }

    printf("Maglev lookup table (size: %u) - Colored:\n", g_maglev.table_size);

    if (g_maglev.node_count == 0) {
        printf("  (empty - no nodes)\n");
        return;
    }

    // Count assignment for each node
    uint32_t *node_counts = calloc(g_maglev.node_count, sizeof(uint32_t));
    if (!node_counts) {
        printf("Error: Memory allocation failed\n");
        return;
    }

    uint32_t unassigned = 0;

    for (uint32_t i = 0; i < g_maglev.table_size; i++) {
        if (g_maglev.lookup_table[i] == UINT32_MAX) {
            unassigned++;
        } else if (g_maglev.lookup_table[i] < g_maglev.node_count) {
            node_counts[g_maglev.lookup_table[i]]++;
        }
    }

    // Show statistics (with colors)
    printf("Distribution summary:\n");
    for (uint32_t i = 0; i < g_maglev.node_count; i++) {
        if (g_maglev.nodes[i]) {
            printf("  ");
            print_colored_text(g_maglev.nodes[i]->name, g_maglev.nodes[i]->color_index);
            printf(": %u slots (%.2f%%)\n",
                   node_counts[i],
                   100.0 * node_counts[i] / g_maglev.table_size);
        }
    }

    if (unassigned > 0) {
        printf("  Unassigned: %u slots (%.2f%%)\n",
               unassigned, 100.0 * unassigned / g_maglev.table_size);
    }

    // Show detailed assignment for first 100 slots (or all if table is smaller)
    uint32_t show_count = (g_maglev.table_size < 100) ? g_maglev.table_size : 100;
    int field_width = get_max_node_name_length();
    int items_per_line = (field_width <= 10) ? 10 : 8;  // Adjust items per line based on name length

    printf("\nFirst %u slots:\n", show_count);

    for (uint32_t i = 0; i < show_count; i++) {
        if (i % items_per_line == 0) {
            printf("\n%4u: ", i);
        }

        if (g_maglev.lookup_table[i] == UINT32_MAX) {
            printf("%*s ", field_width, "-");
        } else if (g_maglev.lookup_table[i] < g_maglev.node_count && g_maglev.nodes[g_maglev.lookup_table[i]]) {
            const char *node_name = g_maglev.nodes[g_maglev.lookup_table[i]]->name;
            int name_len = strlen(node_name);
            int left_padding = (field_width - name_len) / 2;
            int right_padding = field_width - name_len - left_padding;

            printf("%*s", left_padding, "");  // Left padding
            print_colored_text(node_name, g_maglev.nodes[g_maglev.lookup_table[i]]->color_index);
            printf("%*s ", right_padding, "");  // Right padding
        } else {
            printf("%*s ", field_width, "?");
        }
    }
    printf("\n");

    if (g_maglev.table_size > 100) {
        printf("... (showing first 100 out of %u total slots)\n", g_maglev.table_size);
    }

    free(node_counts);
}