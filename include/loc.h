//
// Created by flasque on 19/10/2024.
// Modified by Eliott Belinguier on 20/11/2024.
//

#ifndef _ROVERRUN_LOC_H_
#define _ROVERRUN_LOC_H_

/**
 * @file loc.h
 * @brief Header file defining the structures and macros for managing a rover's localization.
 *
 * This file includes definitions for position, orientation, and macros to initialize and validate positions
 * as well as perform directional movements.
 *
 * ## What is a Macro?
 * A macro in C is a preprocessor directive that allows the definition of reusable code snippets.
 * When the code is compiled, the preprocessor replaces macros with their expanded form.
 *
 * ### Advantages of Macros:
 * - **Performance**: Macros are expanded inline, avoiding the overhead of a function call.
 * - **Code Simplification**: Useful for repetitive tasks or default initialization.
 * - **Flexibility**: Can handle a wide range of inputs without requiring specific types.
 *
 * ### When to Use Macros:
 * - **Simple Operations**: Ideal for constant values, inline initialization, or repetitive code patterns.
 * - **Generic Behavior**: When a function signature might restrict inputs, macros allow type-agnostic operations.
 * - **Performance-Critical Scenarios**: For operations where function call overhead needs to be avoided.
 *
 * ### When to Avoid Macros:
 * - **Complex Logic**: Functions are better for maintainability and debugging in complex cases.
 * - **Type Safety**: Functions provide better type checking than macros.
 * - **Scope Management**: Functions allow proper variable scoping, avoiding potential side effects.
 */

/**
 * @brief Enumeration representing the possible orientations of the rover.
 */
typedef enum orientation {
    NORTH, ///< Facing upwards on the map
    EAST,  ///< Facing right on the map
    SOUTH, ///< Facing downwards on the map
    WEST   ///< Facing left on the map
} orientation_e;

/**
 * @brief Structure representing a position on a 2D grid.
 *
 * The position is represented using `unsigned int` for the x and y coordinates.
 *
 * ### What is `unsigned int`?
 * - `unsigned int` is a type that represents non-negative integer values (0 and above).
 * - It provides a larger range for positive values compared to `int` because it does not allocate bits for representing negative numbers.
 *
 * ### When to Use `unsigned int`:
 * - **Non-Negative Values Only**: When values are logically non-negative (e.g., coordinates, sizes, counters).
 * - **Avoiding Errors**: Prevents accidental negative values that might result in invalid computations.
 * - **Memory Optimization**: Allows for a slightly larger range of values when working with fixed-width integers.
 *
 * ### Why It Is Relevant Here:
 * - Coordinates on a 2D map are inherently non-negative (you cannot have a negative position on a grid).
 * - Using `unsigned int` ensures that invalid negative positions are not possible, increasing the robustness of the code.
 */
typedef struct position {
    unsigned int x; ///< X-coordinate (horizontal axis)
    unsigned int y; ///< Y-coordinate (vertical axis)
} position_s;

/**
 * @brief Structure representing the rover's localization.
 *
 * This includes its position and orientation on the map.
 */
typedef struct localisation {
    position_s pos;      ///< Current position of the rover
    orientation_e ori;   ///< Current orientation of the rover
} localisation_s;

/**
 * @brief Array representing movement vectors for each direction.
 *
 * This constant array defines the movement deltas for the four possible orientations:
 * - `NORTH`: {-1, 0} (move up in the grid)
 * - `EAST`: {1, 0} (move right in the grid)
 * - `SOUTH`: {0, -1} (move down in the grid)
 * - `WEST`: {0, 1} (move left in the grid)
 *
 * This approach centralizes directional logic, making it easy to apply or update.
 */
static const int _direction_vectors[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

/**
 * @brief Macro to initialize a localisation_s instance.
 *
 * @param pos_x The initial x-coordinate of the rover
 * @param pos_y The initial y-coordinate of the rover
 * @param orientation The initial orientation of the rover (orientation_e)
 *
 * @return A `localisation_s` instance with the specified position and orientation.
 */
#define LOCALISATION_INIT(pos_x, pos_y, orientation) \
    ((localisation_s) {.pos = {.x = pos_x, .y = pos_y}, .ori = orientation})

/**
 * @brief Macro to check if a position is valid within the map boundaries.
 *
 * @param position The position_s structure to validate
 * @param map_width The maximum x-coordinate (width of the map)
 * @param map_height The maximum y-coordinate (height of the map)
 *
 * @return A boolean value (true if valid, false otherwise).
 */
#define POSITION_IS_VALID(position, map_width, map_height) ((position).x < map_width && (position).y < map_height)

/**
 * @brief Macro to calculate the position to the left of a given position.
 *
 * @param position The current position_s structure
 * @return A new position_s structure shifted to the left.
 */
#define LEFT(position) ((position_s) {.x = (position).x - 1, .y = (position).y});

/**
 * @brief Macro to calculate the position to the right of a given position.
 *
 * @param position The current position_s structure
 * @return A new position_s structure shifted to the right.
 */
#define RIGHT(position) ((position_s) {.x = (position).x + 1, .y = (position).y});

/**
 * @brief Macro to calculate the position above a given position.
 *
 * @param position The current position_s structure
 * @return A new position_s structure shifted upwards.
 */
#define UP(position) ((position_s) {.x = (position).x, .y = (position).y - 1});

/**
 * @brief Macro to calculate the position below a given position.
 *
 * @param position The current position_s structure
 * @return A new position_s structure shifted downwards.
 */
#define DOWN(position) ((position_s) {.x = (position).x, .y = (position).y + 1});

#endif /* _ROVERRUN_LOC_H_ */