#include "maglev.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT_LEN 1024
#define MAX_ARGS 10

// Command type enumeration
typedef enum {
    CMD_INIT,
    CMD_ADD_NODE,
    CMD_DEL_NODE,
    CMD_SHOW_NODES,
    CMD_SHOW_MAGLEV,
    CMD_HELP,
    CMD_QUIT,
    CMD_UNKNOWN
} CommandType;

// Candidate list for command completion
static char *commands[] = {
    "init",
    "add",
    "del",
    "show",
    "help",
    "quit",
    "exit",
    "nodes",
    "maglev",
    "maglev-color",
    NULL
};

// Command generator function
char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = commands[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

// Completion function
char **command_completion(const char *text, int start, int end) {
    // Disable filename completion
    rl_attempted_completion_over = 1;

    // Avoid unused parameter warning
    (void)end;

    // Only provide command completion at the beginning of line or at certain specific positions
    if (start == 0 || (start > 0 && strncmp(rl_line_buffer, "show ", 5) == 0)) {
        return rl_completion_matches(text, command_generator);
    }

    return NULL;
}

// Remove leading and trailing whitespace characters from string
void trim_whitespace(char *str) {
    char *end;

    // Remove leading whitespace
    while (isspace((unsigned char)*str)) str++;

    // If string is all whitespace
    if (*str == 0) return;

    // Remove trailing whitespace
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Add terminator
    end[1] = '\0';
}

// Parse command line arguments
int parse_arguments(char *input, char **args) {
    int argc = 0;
    char *token = strtok(input, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc] = token;
        argc++;
        token = strtok(NULL, " \t\n");
    }

    args[argc] = NULL;
    return argc;
}

// Identify command type
CommandType identify_command(const char *cmd) {
    if (strcmp(cmd, "init") == 0) {
        return CMD_INIT;
    } else if (strcmp(cmd, "add") == 0) {
        return CMD_ADD_NODE;
    } else if (strcmp(cmd, "del") == 0) {
        return CMD_DEL_NODE;
    } else if (strcmp(cmd, "show") == 0) {
        return CMD_SHOW_NODES;  // Needs further parsing
    } else if (strcmp(cmd, "help") == 0) {
        return CMD_HELP;
    } else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        return CMD_QUIT;
    } else {
        return CMD_UNKNOWN;
    }
}

// Show help information
void show_help(void) {
    printf("\nGoogle Maglev Simulator Commands:\n");
    printf("  init <size>          - Initialize lookup table with given size\n");
    printf("  add <name>           - Add a new node (error if exists)\n");
    printf("  del <name>           - Delete a node (ignore if not exists)\n");
    printf("  show nodes           - Show current nodes\n");
    printf("  show maglev          - Show complete maglev lookup table\n");
    printf("  show maglev-color    - Show maglev lookup table with colored nodes\n");
    printf("  help                 - Show this help message\n");
    printf("  quit/exit            - Exit the simulator\n");
    printf("\nExample:\n");
    printf("  > init 37\n");
    printf("  > add server1\n");
    printf("  > add server2\n");
    printf("  > show nodes\n");
    printf("  > show maglev\n");
    printf("  > show maglev-color\n");
    printf("  > del server1\n");
    printf("\n");
}

// Handle init command
void handle_init_command(int argc, char **args) {
    if (argc != 2) {
        printf("Usage: init <table_size>\n");
        return;
    }

    char *endptr;
    long table_size = strtol(args[1], &endptr, 10);

    if (*endptr != '\0' || table_size <= 0 || table_size > UINT32_MAX) {
        printf("Error: Invalid table size '%s'\n", args[1]);
        return;
    }

    if (!maglev_init((uint32_t)table_size)) {
        printf("Error: Failed to initialize Maglev table\n");
    }
}

// Handle add command
void handle_add_command(int argc, char **args) {
    if (argc != 2) {
        printf("Usage: add <node_name>\n");
        return;
    }

    maglev_add_node(args[1]);
}

// Handle del command
void handle_del_command(int argc, char **args) {
    if (argc != 2) {
        printf("Usage: del <node_name>\n");
        return;
    }

    maglev_remove_node(args[1]);
}

// Handle show command
void handle_show_command(int argc, char **args) {
    if (argc != 2) {
        printf("Usage: show <nodes|maglev|maglev-color>\n");
        return;
    }

    if (strcmp(args[1], "nodes") == 0) {
        maglev_show_nodes();
    } else if (strcmp(args[1], "maglev") == 0) {
        maglev_show_table();
    } else if (strcmp(args[1], "maglev-color") == 0) {
        maglev_show_table_colored();
    } else {
        printf("Usage: show <nodes|maglev|maglev-color>\n");
    }
}

// Process a single command
void process_command(char *input) {
    char *args[MAX_ARGS];
    int argc = parse_arguments(input, args);

    if (argc == 0) {
        return;  // Empty command
    }

    CommandType cmd_type = identify_command(args[0]);

    switch (cmd_type) {
        case CMD_INIT:
            handle_init_command(argc, args);
            break;

        case CMD_ADD_NODE:
            handle_add_command(argc, args);
            break;

        case CMD_DEL_NODE:
            handle_del_command(argc, args);
            break;

        case CMD_SHOW_NODES:
            handle_show_command(argc, args);
            break;

        case CMD_HELP:
            show_help();
            break;

        case CMD_QUIT:
            printf("Goodbye!\n");
            exit(0);
            break;

        case CMD_UNKNOWN:
        default:
            printf("Unknown command: %s\n", args[0]);
            printf("Type 'help' for available commands.\n");
            break;
    }
}

// Initialize readline
void init_readline(void) {
    // Set history file path
    char *home = getenv("HOME");
    if (home) {
        char history_file[512];
        snprintf(history_file, sizeof(history_file), "%s/.maglev_history", home);
        read_history(history_file);
    }

    // Set program name
    rl_readline_name = "maglev-simulator";

    // Set command completion function
    rl_attempted_completion_function = command_completion;

    // Limit history record count
    stifle_history(100);
}

// Clean up readline
void cleanup_readline(void) {
    // Save history to file
    char *home = getenv("HOME");
    if (home) {
        char history_file[512];
        snprintf(history_file, sizeof(history_file), "%s/.maglev_history", home);
        write_history(history_file);
    }

    // Clean up history memory
    clear_history();
}

// File execution result enumeration
typedef enum {
    FILE_EXEC_CONTINUE,    // Continue interactive mode
    FILE_EXEC_QUIT,        // File ends with quit, should exit
    FILE_EXEC_ERROR        // File error (such as not existing)
} FileExecResult;

// Execute commands from file
FileExecResult execute_commands_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file '%s'\n", filename);
        return FILE_EXEC_ERROR;
    }

    printf("Executing commands from file: %s\n", filename);

    char line[MAX_INPUT_LEN];
    bool has_quit = false;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline and leading/trailing whitespace
        trim_whitespace(line);

        // Skip empty lines and comment lines
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }

        // Display the command being executed
        printf("> %s\n", line);

        // Check if it's an exit command
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) {
            has_quit = true;
        }

        // Execute command
        process_command(line);

        // If command is exit, stop processing subsequent commands
        if (has_quit) {
            break;
        }
    }

    fclose(file);
    return has_quit ? FILE_EXEC_QUIT : FILE_EXEC_CONTINUE;
}

// Show usage help
void show_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] [COMMAND]\n", program_name);
    printf("\nOptions:\n");
    printf("  -C <file>    Execute commands from file, then continue interactively\n");
    printf("               if the file doesn't end with 'quit'\n");
    printf("  -h, --help   Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s                                  # Interactive mode\n", program_name);
    printf("  %s help                             # Execute single command\n", program_name);
    printf("  %s -C scripts/batch_commands.txt    # Execute commands from file\n", program_name);
    printf("\nFile format:\n");
    printf("  # This is a comment\n");
    printf("  init 37\n");
    printf("  add node server1\n");
    printf("  show nodes\n");
    printf("  # If no 'quit' at end, continues to interactive mode\n");
}

// Main function
int main(int argc, char *argv[]) {
        printf("Google Maglev Simulator\n");

    const char *command_file = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-C") == 0) {
            if (i + 1 < argc) {
                command_file = argv[i + 1];
                i++; // Skip filename parameter
            } else {
                printf("Error: -C option requires a filename\n");
                show_usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return 0;
        } else {
            // Compatible with original single command mode
            char command[MAX_INPUT_LEN] = "";
            for (int j = i; j < argc; j++) {
                strcat(command, argv[j]);
                if (j < argc - 1) {
                    strcat(command, " ");
                }
            }

            printf("Type 'help' for available commands, 'quit' to exit.\n");
            printf("Use UP/DOWN arrows to navigate command history.\n\n");
            printf("Executing: %s\n", command);
            process_command(command);

            // Exit after single command mode execution
            maglev_cleanup();
            return 0;
        }
    }

    printf("Type 'help' for available commands, 'quit' to exit.\n");
    printf("Use UP/DOWN arrows to navigate command history.\n");
    if (command_file) {
        printf("Use -h for command line options.\n");
    }
    printf("\n");

    // If command file is specified, execute commands from file first
    if (command_file) {
        FileExecResult result = execute_commands_from_file(command_file);

        if (result == FILE_EXEC_ERROR) {
            // File error, exit program
            maglev_cleanup();
            return 1;
        } else if (result == FILE_EXEC_QUIT) {
            // File ends with quit, exit normally
            maglev_cleanup();
            return 0;
        }

        // Continue interactive mode
        printf("\n--- Entering interactive mode ---\n");
    }

    // Initialize readline
    init_readline();

    // Interactive mode
    char *input;
    while ((input = readline("> ")) != NULL) {
        // Remove leading and trailing whitespace
        trim_whitespace(input);

        // If input is not empty, add to history
        if (strlen(input) > 0) {
            add_history(input);
        }

        // Process command
        process_command(input);

        // Free memory allocated by readline
        free(input);
    }

    // User pressed Ctrl+D to exit
    printf("\nGoodbye!\n");

    // Clean up resources
    cleanup_readline();
    maglev_cleanup();
    return 0;
}