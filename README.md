# Google Maglev Simulator

A Google Maglev load balancing algorithm simulator implemented in C and CMake.

## What is the Maglev Algorithm?

Google Maglev is a consistent hashing algorithm used for load balancing. Its core concepts are:
1. Generate a preference list for each node
2. Fill the lookup table through round-robin polling
3. Ensure each node is distributed as evenly as possible in the lookup table
4. Minimize impact on existing mappings when nodes are added or removed

## Build and Run

### Build Requirements
- C99 compatible compiler (such as GCC)
- CMake 3.10 or higher
- GNU Readline library (pre-installed on most Linux distributions)
  - Ubuntu/Debian: `sudo apt-get install libreadline-dev`
  - CentOS/RHEL: `sudo yum install readline-devel`

### Build Steps
```bash
mkdir -p build
cd build
cmake ..
make
```

### Running Methods
```bash
# Interactive mode (supports command history and auto-completion)
./maglev-simulator

# Batch mode (execute single command)
./maglev-simulator help
./maglev-simulator init 37

# Execute commands from file
./maglev-simulator -C scripts/batch_commands.txt

# Show help information
./maglev-simulator -h
```

### Interactive Features
- **Command History**: Use ↑↓ arrow keys to browse and repeat previous commands
- **Command Editing**: Support left/right arrow keys, Home/End, Backspace and other editing shortcuts
- **Auto-completion**: Press Tab to auto-complete command keywords
- **History Persistence**: Command history is saved in `~/.maglev_history` file
- **File Execution**: Use `-C` parameter to batch execute commands from file
- **Mixed Mode**: Continue interactive mode after file execution (if file doesn't end with quit)
- **Ctrl+D Exit**: Gracefully exit the simulator

## Supported Commands

### 1. init <size>
Reset and initialize the lookup table, removing all existing nodes.
- `size`: Size of the lookup table, program will automatically adjust to the nearest prime number
- Example: `init 37`

### 2. add <name>
Add a new node to the Maglev table.
- `name`: Node name (maximum 255 characters)
- Will report error if node already exists
- Example: `add server1`

### 3. del <name>
Remove specified node from the Maglev table.
- `name`: Name of the node to remove
- Will be ignored if node doesn't exist (no error reported)
- Example: `del server1`

### 4. show nodes
Display the list of all current nodes and basic information.

### 5. show maglev
Display the complete Maglev lookup table state, including:
- Distribution statistics for each node
- Detailed lookup table contents (shows first 100 slots)

### 6. show maglev-color
Display the Maglev lookup table with colored node names for better visualization:
- Same information as `show maglev` but with colored output
- Each node gets a unique color for easy identification
- Supports up to 128 different colors

### 7. help
Display help information for all available commands.

### 8. quit/exit
Exit the simulator.

## File Execution Feature

Use the `-C` parameter to batch execute commands from a file:

```bash
./maglev-simulator -C scripts/batch_commands.txt
```

### File Format
- One command per line
- Lines starting with `#` are comments and will be ignored
- Empty lines will be skipped
- If file ends with `quit` or `exit`, program will exit after execution
- If file doesn't end with `quit`, program will enter interactive mode

### Example File
```bash
# This is an example command file
# Initialize system
init 37

# Add server nodes
add web1
add web2
add web3

# View status
show nodes
show maglev

# Note: No quit here, so will continue to interactive mode
```

### Use Cases
1. **Quick Setup**: Predefine standard initialization steps
2. **Test Scripts**: Automate testing of specific scenarios
3. **Demo Preparation**: Quickly prepare demo environment
4. **Debug Reproduction**: Reproduce specific bug scenarios

## Usage Examples

```bash
# Start simulator
./maglev-simulator

# Initialize lookup table with size 37
> init 37

# Add three nodes
> add server1
> add server2
> add server3

# View node list
> show nodes

# View complete Maglev table distribution
> show maglev

# View colored table distribution
> show maglev-color

# Remove a node
> del server2

# View table changes again
> show maglev

# Exit
> quit
```

## Algorithm Characteristics

1. **Consistency**: When nodes change, only affected parts will be remapped
2. **Even Distribution**: Algorithm ensures each node gets roughly equal load
3. **Fast Lookup**: O(1) time complexity lookup operations
4. **Prime Table Size**: Uses prime numbers as table size to improve hash distribution uniformity

## Technical Implementation

- **Hash Functions**: Uses DJB2 and SDBM hash algorithms to generate preference lists
- **Memory Management**: Dynamic memory allocation, supports arbitrary sized lookup tables
- **Error Handling**: Complete error checking and user-friendly error messages
- **Interactive Interface**: Supports both interactive and batch execution modes

## Project Structure

```
├── CMakeLists.txt          # CMake configuration file
├── README.md              # Project documentation
├── include/               # Header files directory
│   ├── maglev.h          # Main data structures and function declarations
│   ├── node.h            # Node management functions
│   └── hash.h            # Hash function declarations
└── src/                  # Source code directory
    ├── main.c            # Main program and command line parsing
    ├── maglev.c          # Maglev algorithm core implementation
    ├── node.c            # Node management implementation
    └── hash.c            # Hash function implementation
```

## Notes

- Table size is automatically adjusted to prime numbers to improve distribution uniformity
- Node names support up to 255 characters
- Maximum support for 1000 nodes
- Memory usage is proportional to table size and number of nodes
