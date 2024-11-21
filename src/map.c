//
// Created by flasque on 19/10/2024.
// Modified by Eliott Belinguier on 21/11/2024.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "map.h"
#include "loc.h"
#include "queue.h"

/**
 * @brief Safely closes a file pointer when it goes out of scope.
 *
 * ### What is `__attribute__`?
 * - `__attribute__` is an extension provided by GCC and compatible compilers, allowing developers to attach specific metadata
 *   or behaviors to variables, functions, or types.
 * - Examples include aligning memory, optimizing functions, or automatically managing resources.

 * ### What is `__attribute__((cleanup))`?
 * - `cleanup` is a specific attribute that assigns a cleanup function to a variable.
 * - When the variable goes out of scope, the specified cleanup function is called automatically.
 * - This cleanup function takes a pointer to the variable as its argument.

 * ### Why Use It Here?
 * - In this case, `__attribute__((cleanup))` ensures that file pointers are always properly closed when they go out of scope,
 *   even if the function exits early due to an error.
 * - It reduces the risk of resource leaks, simplifies error handling, and ensures consistent behavior across all exit paths.

 * ### Relevance in This File:
 * - File pointers are critical resources that must be closed to avoid file descriptor exhaustion.
 * - By using `__attribute__((cleanup))`, we guarantee that every opened file is closed, regardless of how the function exits.

 * @param file Pointer to the file to be closed.
 */
static void _cleanup_close_file(FILE **file)
{
    if (*file)
        fclose(*file);
}

/**
 * @brief Frees a dynamically allocated 2D array when it goes out of scope.
 *
 * ### Why Use `__attribute__((cleanup))` for Arrays?
 * - Dynamically allocated memory, especially 2D arrays, requires explicit cleanup to avoid memory leaks.
 * - Manual cleanup can be error-prone, particularly in functions with multiple exit paths.
 * - Using `__attribute__((cleanup))` ensures that cleanup is automatic and consistent.

 * ### Relevance in This File:
 * - The `soils` and `costs` arrays in this file are dynamically allocated to represent a map's terrain and movement costs.
 * - Using this mechanism prevents memory leaks by ensuring proper deallocation, even if an error occurs during the function's execution.

 * @param ptr Pointer to the 2D array to free.
 */
static void _cleanup_ptr_2d_array_free(void *ptr)
{
    void ***ptr_real = (void ***) ptr;
    void **current;

    if (!*ptr_real)
        return;
    for (current = *ptr_real; *current; ++current)
        free(*current);
    free(current);
}

/**
 * @brief Allocates memory for a 2D array.
 *
 * @param dim_y Number of rows in the array.
 * @param dim_x Number of columns in the array.
 * @param element_size Size of each element in the array.
 * @return Pointer to the allocated 2D array or NULL on failure.
 *
 * ### What is `size_t`?
 * - `size_t` is an unsigned integer type defined in the standard library (`<stddef.h>`).
 * - It represents sizes, lengths, and counts, such as the size of an array or the result of the `sizeof` operator.
 * - On most platforms:
 *   - On 32-bit systems, `size_t` is typically equivalent to `unsigned int`.
 *   - On 64-bit systems, `size_t` is typically equivalent to `unsigned long long`.

 * ### Why Use `size_t`?
 * - **Non-Negative Representation**: Sizes and dimensions, such as `dim_y` and `dim_x`, cannot be negative. `size_t` enforces this at the type level.
 * - **Portability**: It adapts to the platform's architecture, ensuring compatibility with large arrays on 64-bit systems.
 * - **Alignment with Standards**: Functions like `malloc` and the `sizeof` operator use `size_t`, making it the logical choice for sizes and lengths.

 * ### Relevance in This File:
 * - The `dim_y` and `dim_x` parameters represent the dimensions of a 2D array, which are always non-negative.
 * - By using `size_t`, we ensure these dimensions are properly constrained and aligned with the memory allocation functions.

 */
static void *_2d_array_allocation(size_t dim_y, size_t dim_x, size_t element_size)
{
    void **array = malloc(dim_y * sizeof(void *));
    void *array_line;

    if (!array)
        return 0;
    for (size_t i = 0; i < dim_y; ++i) {
        array_line = malloc(dim_x * element_size);
        if (!array_line) {
            _cleanup_ptr_2d_array_free(&array);
            return 0;
        }
        array[i] = array_line;
    }
    return array;
}

/**
 * @brief Finds the position of the base station on the map.
 *
 * This function scans the map to locate the first occurrence of the base station.
 *
 * @param map The map structure containing grid information.
 * @return A `position_s` structure representing the base station's position,
 *         or an invalid position if the base station is not found.
 */
static position_s _get_base_station_pos(map_s map)
{
    // Store the map's dimensions in local variables.
    // Accessing `map.height` and `map.width` repeatedly would involve dereferencing the `map` structure multiple times.
    // By using `map_height` and `map_width`, we improve performance and make the code easier to read.
    unsigned int map_height = map.height; // Number of rows in the map.
    unsigned int map_width = map.width;   // Number of columns in the map.

    // Cache the `map.soils` array in a local variable for similar reasons.
    // This avoids repeated dereferencing of `map.soils` in the loop.
    soil_e **soils = map.soils;

    // Iterate over each row of the map.
    for (unsigned int y = 0; y < map_height; ++y) {
        // Iterate over each column of the current row.
        for (unsigned int x = 0; x < map_width; x++) {
            // Check if the current cell is the base station.
            if (soils[y][x] == BASE_STATION)
                // Check if the current cell is the base station.
                return (position_s) {.y = y, .x = x};
        }
    }

    // If no base station is found, print an error message and return an invalid position.
    fprintf(stderr, "Error: base station not found in the map\n");
    return (position_s) {.y = 0xFFFFFFFF, .x = 0xFFFFFFFF};
}

/**
 * @brief Calculates the movement cost for a specific cell on the map.
 *
 * This function determines the movement cost of a given cell based on:
 * - The cost of its neighbors.
 * - The soil type of the cell itself.
 *
 * ### How It Works:
 * 1. The function starts by retrieving the current cell's position (`position`) and its soil type cost from the `_soil_cost` array.
 * 2. It iterates over the four possible neighboring cells (NORTH, EAST, SOUTH, WEST) using predefined direction vectors.
 * 3. For each neighboring cell:
 *    - Checks if the neighbor's position is valid using the `POSITION_IS_VALID` macro.
 *    - If valid:
 *      - Compares the neighbor's cost to the current minimum cost (`cost_min`) and updates `cost_min` if it's smaller.
 *      - If the neighbor's cost is undefined (`COST_UNDEF`) and a queue is provided, the neighbor is enqueued for processing, and its cost is temporarily marked as visited (`COST_UNDEF - 1`).
 * 4. The movement cost is calculated by adding the soil type cost of the current cell to the smallest neighboring cost (`cost_min`).
 *    - If the soil type cost is `0` (e.g., base station), the cost for this cell is set to `0`.
 *
 * ### Why Is This Necessary?
 * - The function is central to the propagation of movement costs across the map.
 * - It ensures that each cell's cost is computed based on its terrain difficulty and the shortest path from a reachable base station or starting point.
 *
 * ### Why Use a Queue and What Is Its Role?
 * - The queue (`t_queue`) is used to track cells with undefined costs (`COST_UNDEF`) that need further processing.
 * - It ensures that cost propagation occurs in a **breadth-first manner**, starting with cells closest to the base station and spreading outward.
 * - Cells with undefined costs are added to the queue when they are identified as neighbors of the current cell.
 * - The queue ensures that these cells are processed in the correct order, maintaining the shortest path computation logic.
 *
 * ### Edge Cases:
 * - If the cell is unreachable or surrounded by impassable terrain, its cost remains undefined until valid neighbors propagate costs.
 * - Cells with a soil cost of `0` (e.g., base station) always have a computed cost of `0`.
 *
 * @param map The map structure containing the grid, soil types, and movement cost information.
 * @param position The position of the cell to compute.
 * @param queue A queue to enqueue neighboring cells with undefined costs for further processing.
 * @return The calculated movement cost for the specified cell.
 */
static unsigned int _compute_cell_cost(map_s map, position_s position, t_queue *queue)
{
    // Cache commonly accessed fields of the map structure for performance optimization.
    unsigned int map_width = map.width;   // The width of the map (number of columns).
    unsigned int map_height = map.height; // The height of the map (number of rows).
    unsigned int **costs = map.costs;     // Pointer to the 2D array of movement costs.

    // Cache commonly accessed fields of the map structure for performance optimization.
    unsigned int position_current_x = position.x; // X-coordinate of the current cell.
    unsigned int position_current_y = position.y; // Y-coordinate of the current cell.

    // Get the movement cost of the current cell's soil type.
    unsigned int soil_current_cost = _soil_cost[map.soils[position_current_y][position_current_x]];

    const int *direction_vector;        // Holds the direction vector for traversing neighbors.
    position_s compute_pos;             // Position of the neighboring cell being processed.
    unsigned int compute_pos_cost;      // Cost of the neighboring cell.
    unsigned int cost_min = 0xFFFFFFFF; // Initialize minimum cost to the maximum possible value.

    // Iterate over all four possible directions (NORTH, EAST, SOUTH, WEST).
    for (size_t i = 0; i < 4; ++i) {
        direction_vector = _direction_vectors[i]; // Get the direction vector for the current iteration.

        // Compute the neighboring cell's position.
        compute_pos = (position_s) {
            .x = position_current_x + direction_vector[0],
            .y = position_current_y + direction_vector[1]
        };

        // Check if the neighboring position is valid (within map bounds).
        if (POSITION_IS_VALID(compute_pos, map_width, map_height)) {
            compute_pos_cost = costs[compute_pos.y][compute_pos.x];

            // Update the minimum cost if the neighbor's cost is smaller.
            if (compute_pos_cost < cost_min)
                cost_min = compute_pos_cost;

            // If the neighbor's cost is undefined, enqueue it for future processing.
            else if (queue && compute_pos_cost == COST_UNDEF) {
                enqueue(queue, compute_pos);
                costs[compute_pos.y][compute_pos.x] = COST_UNDEF - 1;
            }
        }
    }

    // Calculate the movement cost for the current cell.
    // Add the soil cost to the minimum neighboring cost, unless the soil cost is zero.
    return soil_current_cost != 0 ? cost_min + soil_current_cost : 0;
}

/**
 * @brief Recalculates invalid movement costs on the map.
 *
 * This function identifies cells with invalid or excessive movement costs
 * and recalculates them by propagating correct costs from neighboring cells.
 * It ensures that all cells have valid and consistent movement costs.
 *
 * ### How It Works:
 * - The function iterates over all cells in the map grid.
 * - For each cell:
 *   - If the cell is not a crevasse (`CREVASSE`) and its cost is greater than a defined threshold (e.g., `10000`), it is considered invalid.
 *   - The function recalculates the cost for this cell by calling `_compute_cell_cost`.
 * - The process repeats in a loop until no more invalid costs are found (`find` flag ensures iterative processing).

 * ### Why Is This Necessary?
 * - During the cost computation phase, some cells may be left with invalid or undefined costs due to incomplete propagation
 *   (e.g., cells that are initially unreachable but become reachable later).
 * - This function ensures that all reachable cells have accurate movement costs, which is critical for pathfinding algorithms
 *   or any operation dependent on consistent cost values.

 * ### Edge Cases:
 * - Crevasse cells (`CREVASSE`) are ignored, as they are impassable and do not require cost recalculations.
 * - Cells with valid costs (≤ `10000`) are skipped to avoid unnecessary recomputation.

 * @param map The map structure containing the grid and movement cost information.
 */
static void _remove_false_costs(map_s map)
{
    // Cache the `map.soils` array in a local variable for optimization.
    // Accessing `map.soils` repeatedly through the structure would involve
    // additional memory operations. Using `soils` locally improves performance
    // and makes the code more concise.
    soil_e **soils = map.soils;

    // Cache the `map.costs` array in a local variable for similar reasons.
    // This reduces repetitive dereferencing and simplifies access within the loops.
    unsigned int **costs = map.costs;

    // Iterate over each row of the map.
    for (unsigned int y = 0; y < map.height; ++y) {
        // Iterate over each column of the map.
        for (unsigned int x = 0; x < map.width; ++x) {
            // Check if the cell is not a crevasse and has an invalid cost.
            if (soils[y][x] != CREVASSE && costs[y][x] > 10000)
                // Recalculate the movement cost for the current cell.
                costs[y][x] = _compute_cell_cost(map, (position_s) {.x = x, .y = y}, 0);
        }
    }
}

/**
 * @brief Computes movement costs for all cells on the map.
 *
 * This function starts at the base station and propagates the movement costs
 * to neighboring cells using a queue for breadth-first traversal.
 *
 * @param map The map structure containing the grid and cost information.
 */
static void _compute_costs(map_s map)
{
    // Cache the movement cost array for efficient access.
    unsigned int **costs = map.costs;

    // Find the position of the base station (starting point).
    position_s base_station_pos = _get_base_station_pos(map);

    // Initialize a queue to store cells to be processed.
    // The queue ensures breadth-first propagation of costs.
    t_queue queue = createQueue(map.width * map.height);

    // Temporary variable to hold the current cell being processed.
    position_s current_pos;

    // Check if the base station was found.
    if (base_station_pos.x == 0xFFFFFFFF || base_station_pos.y == 0xFFFFFFFF) {
        fprintf(stderr, "No base station found in map.\n");
        return;
    }

    // Check if the base station was found.
    enqueue(&queue, base_station_pos);

    // Process the queue until all reachable cells have been handled.
    while (queue.first != queue.last) {
        // Dequeue the next cell to process.
        current_pos = dequeue(&queue);

        // Compute the movement cost for the current cell.
        // This will also enqueue any neighbors with undefined costs.
        costs[current_pos.y][current_pos.x] = _compute_cell_cost(map, current_pos, &queue);
    }
}

/**
 * @brief Reads and initializes a map from a file.
 *
 * This function uses `__attribute__((cleanup))` to automatically manage resources, such as
 * file pointers and dynamically allocated 2D arrays, ensuring proper cleanup even during errors.
 *
 * ### Relevance of `__attribute__((cleanup))`:
 * - Ensures that resources like the `file` pointer and `soils` and `costs` arrays are released automatically.
 * - Simplifies error handling by removing the need for explicit cleanup code in every exit path.
 *
 * @param file_path Path to the file containing the map data.
 * @return A fully initialized map structure.
 */
static int _map_fill_from_file(map_s *map, FILE *file)
{
    // Store the map dimensions in local variables.
    // This is done for performance reasons (avoiding repeated structure field accesses)
    // and to improve code readability by reducing repetitive calls to `map->height` and `map->width`.
    unsigned int map_height = map->height;
    unsigned int map_width = map->width;

    // Dynamically allocate the `soils` array to store the terrain types of each cell.
    // Use `__attribute__((cleanup))` to ensure the array is freed automatically on function exit.
    soil_e **soils __attribute__ ((cleanup (_cleanup_ptr_2d_array_free))) = 0;

    // Dynamically allocate the `costs` array to store the movement costs of each cell.
    // Similarly, use `__attribute__((cleanup))` to ensure automatic cleanup.
    unsigned int **costs __attribute__ ((cleanup (_cleanup_ptr_2d_array_free))) = 0;

    int fscanf_result;         // Stores the result of the `fscanf` operation.
    unsigned int value_current; // Temporary storage for reading terrain values from the file.

    // Allocate memory for the 2D array of terrain types (`soils`).
    soils = _2d_array_allocation(map_height, map_width, sizeof(unsigned int));
    if (!soils) {
        perror("Map allocation error"); // Print error message if memory allocation fails.
        return -1;
    }

    // Allocate memory for the 2D array of movement costs (`costs`).
    costs = _2d_array_allocation(map_height, map_width, sizeof(unsigned int));
    if (!costs) {
        perror("Map allocation error"); // Allocate memory for the 2D array of movement costs (`costs`).
        return -1;
    }

    // Allocate memory for the 2D array of movement costs (`costs`).
    for (unsigned int y = 0; y < map_height; ++y) {
        for (unsigned int x = 0; x < map_width; ++x) {
            // Read the next terrain value from the fil
            fscanf_result = fscanf(file, "%u", &value_current);

            // Read the next terrain value from the fil
            if (fscanf_result == -1) {
                errno = ERANGE; // Set error code for range issues.
                perror("Invalid Map size"); // Print error message for an invalid map size.
                return -1;
            } if (!fscanf_result) {
                fprintf(stderr, "Map formatting error.\n"); // Print error message for an invalid map size.
                return -1;
            }

            // Print error message for an invalid map size.
            soils[y][x] = value_current;
        }
    }

    // Initialize the `costs` array with a default value (undefined cost).
    for (size_t i = 0; i < map_height; ++i)
        memset(costs[i], 0xFF, map_width * sizeof(unsigned int));

    // Assign the dynamically allocated arrays to the map structure.
    map->soils = soils;
    map->costs = costs;

    // Assign the dynamically allocated arrays to the map structure.
    soils = 0;
    costs = 0;

    return 0; // Indicate success.
}

/**
 * @brief Reads and initializes a map structure from a file.
 *
 * This function reads the dimensions of the map from the file, allocates memory for the map's
 * terrain and cost data, and fills the map by processing the file's contents. It ensures that
 * all resources (e.g., file pointers, memory) are properly managed and freed using
 * `__attribute__((cleanup))`.
 *
 * ### Steps:
 * 1. Opens the file and reads the map dimensions (height and width).
 * 2. Uses `_map_fill_from_file` to populate the `soils` and `costs` arrays of the map structure.
 * 3. Calls `_compute_costs` to calculate movement costs for all reachable cells.
 * 4. Calls `_remove_false_costs` to correct any inconsistencies in the cost data.
 *
 * ### Error Handling:
 * - If the file cannot be opened, a warning is printed, and an empty map is returned.
 * - If the file format is invalid or memory allocation fails, appropriate error messages are displayed.
 *
 * @param file_path Path to the file containing the map data.
 * @return A fully initialized map structure, or an empty map if an error occurs.
 */
map_s map_from_file(const char *file_path)
{
    // Initialize the map structure to default (empty) values.
    map_s map = {0, 0, 0, 0};

    // Open the file in read text mode.
    // Use `__attribute__((cleanup))` to ensure the file is automatically closed on function exit.
    FILE *file __attribute__ ((cleanup (_cleanup_close_file))) = fopen(file_path, "rt");

    // Check if the file was successfully opened.
    if (!file) {
        perror("Map cannot be read"); // Print an error if the file could not be opened.
        return map; // Return an empty map structure.
    }

    // Read the map dimensions (height and width) from the file.
    // The dimensions must be successfully read; otherwise, return an empty map.
    if (!fscanf(file, "%u", &map.height) || !fscanf(file, "%u", &map.width)) {
        fprintf(stderr, "Invalid map dimensions.\n"); // Print an error for invalid dimensions.
        return map;
    }

    // Fill the map structure's `soils` and `costs` arrays with data from the file.
    // If this step fails, return an empty map.
    if (_map_fill_from_file(&map, file) != 0) {
        fprintf(stderr, "Failed to load map data.\n"); // Print an error for loading failure.
        return map;
    }

    // Compute the movement costs for all cells on the map.
    _compute_costs(map);

    // Compute the movement costs for all cells on the map.
    _remove_false_costs(map);

    // Compute the movement costs for all cells on the map.
    return map;
}

/**
 * @brief Displays the map to the terminal in a human-readable format.
 *
 * This function renders the map's terrain grid as ASCII characters, showing
 * the type of terrain for each cell. The base station is displayed as 'B',
 * and other terrain types are represented with custom symbols.
 *
 * ### How It Works:
 * 1. The map's dimensions are scaled to display each terrain cell over multiple rows for better visibility.
 * 2. Terrain symbols are determined based on the soil type.
 * 3. Each row of the display grid is printed to the terminal line by line.
 *
 * @param map The map structure to display, containing terrain and cost data.
 */
void map_display(map_s map)
{
    // Symbols for displaying each terrain type.
    static unsigned char display_spe[] = {219,219,219}; // Special block for crevasse-like terrain.
    static char *display[] = {"   ", "---", "~~~", "^^^", (char *) display_spe};

    // Special block for crevasse-like terrain.
    unsigned int map_display_width = map.width;       // Width of the map.
    unsigned int map_display_height = map.height * 3; // Each row of the map is displayed as 3 rows.

    // Cache the `map.soils` array in a local variable for performance optimization.
    // Accessing `map.soils` repeatedly through the structure would involve dereferencing
    // the pointer multiple times. Using `map_soil` locally reduces this overhead,
    // improving the performance of the function, especially for large maps.
    soil_e **map_soil = map.soils;

    // Temporary variable to hold the current soil type being processed.
    soil_e soil_current;

    // Iterate over the scaled height of the map for rendering.
    for (unsigned int y = 0; y < map_display_height; ++y) {
        // Iterate over the scaled height of the map for rendering.
        for (unsigned int x = 0; x < map_display_width; ++x) {
            // Iterate over the scaled height of the map for rendering.
            soil_current = map_soil[y / 3][x]; // Iterate over the scaled height of the map for rendering.

            // Display the appropriate symbol for the soil type.
            if (soil_current > CREVASSE)
                write(1, "???", 3); // Unknown terrain type.
            else if (soil_current == BASE_STATION && y % 3 == 1)
                write(1, " B ", 3); // Unknown terrain type.
            else
                write(1, display[map_soil[y / 3][x]], 3); // Display the symbol for the terrain type.
        }

        // Print a newline after each row of the display grid.
        write(1, "\n", 1);
    }
}