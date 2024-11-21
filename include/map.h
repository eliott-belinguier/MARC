//
// Created by flasque on 19/10/2024.
// Modified by Eliott Belinguier on 21/11/2024.
//

#ifndef _ROVERRUN_MAP_H_
#define _ROVERRUN_MAP_H_

/**
 * @file map.h
 * @brief Header file for map representation and related utilities.
 *
 * This file defines the structure of a map, including its dimensions, soil types, and movement costs.
 * It also includes utility functions and macros for loading and displaying a map.
 */

/**
 * @brief Special value representing an undefined cost.
 *
 * The constant `COST_UNDEF` is used to signify that the movement cost of a specific tile is not defined.
 * This value (`0xFFFFFFFF`) is the maximum value of an unsigned 32-bit integer.
 */
#define COST_UNDEF 0xFFFFFFFF

/**
 * @brief Enumeration of soil types on the map.
 */
typedef enum soil {
    BASE_STATION, ///< The base station tile (cost 0)
    PLAIN,        ///< Plain terrain (cost 1)
    ERG,          ///< Sand dunes (cost 2)
    REG,          ///< Rocky terrain (cost 4)
    CREVASSE      ///< Dangerous crevasse (cost 10000, impassable)
} soil_e;

/**
 * @brief Array defining the movement cost for each soil type.
 *
 * This static array associates each soil type with its respective movement cost:
 * - `BASE_STATION`: Cost 0 (no movement required)
 * - `PLAIN`: Cost 1
 * - `ERG`: Cost 2
 * - `REG`: Cost 4
 * - `CREVASSE`: Cost 10000 (effectively impassable)
 *
 * Using this array centralizes the cost logic, making it easier to update or reference.
 */
static const int _soil_cost[5] = {0, 1, 2, 4, 10000};

/**
 * @brief Structure representing a map in the simulation.
 *
 * The map is defined by its dimensions (width and height), its soil types, and the associated movement costs.
 *
 * ### What is `unsigned int`?
 * - **Definition**: An `unsigned int` is an integer data type in C that can only represent non-negative values.
 *   Unlike a regular `int` (which is signed and can represent both positive and negative numbers),
 *   an `unsigned int` uses all its bits to store positive values, effectively doubling its range for non-negative values.
 * - **Range**: On most systems, an `unsigned int` can represent values from 0 to 4,294,967,295 (32-bit systems).
 *
 * ### Why Use `unsigned int` in This Case?
 * - **Non-Negative Values**:
 *   - Map dimensions (`width` and `height`) cannot logically be negative.
 *   - Movement costs (`costs`) are also inherently non-negative, as they represent physical constraints or penalties.
 *   Using `unsigned int` enforces these constraints at the type level, ensuring that invalid negative values are not possible.
 * - **Logical Representation**: The use of `unsigned int` aligns with the semantics of the problem domain:
 *   - A map with negative dimensions does not make sense.
 *   - A movement cost of less than 0 would break the logic of cost-based calculations.
 * - **Larger Positive Range**: By avoiding the use of bits for negative values, `unsigned int` offers a larger range for positive values.
 *   This is particularly useful for:
 *   - Large maps with high dimensions.
 *   - High movement costs, such as representing impassable terrain (`COST_UNDEF`).
 * - **Error Prevention**: Accidental underflows (e.g., subtracting 1 from a 0 value) are easier to detect because they will wrap around
 *   to a large positive value (e.g., 0 - 1 becomes 4,294,967,295). This behavior is preferable to allowing negative values
 *   that could lead to invalid array indexing or other errors.
 *
 * ### Key Takeaways:
 * - `unsigned int` is a type that enforces non-negative values.
 * - It is particularly relevant for dimensions and costs because they are logically non-negative and may require a large range.
 * - This choice increases the robustness and clarity of the code, reducing the chance of logical or runtime errors.
 */
typedef struct map {
    unsigned int width;       ///< Width of the map (number of columns), must be non-negative
    unsigned int height;      ///< Height of the map (number of rows), must be non-negative
    soil_e **soils;           ///< 2D array representing the soil type for each tile
    unsigned int **costs;     ///< 2D array representing the movement cost for each tile
} map_s;

/**
 * @brief Function to load a map from a file.
 *
 * @param file_path The path to the file containing the map definition.
 * @return A `map_s` structure representing the loaded map.
 */
map_s map_from_file(const char *file_path);

/**
 * @brief Function to display the map.
 *
 * @param map The map to display.
 */
void map_display(map_s map);

/**
 * @brief Macro to load the training map.
 *
 * This macro abstracts the path differences between Windows and Unix-based systems.
 *
 * ### Preprocessor Conditions:
 * - `#if defined(_WIN32) || defined(_WIN64)`: Checks if the program is being compiled on a Windows system.
 * - If true, the path uses backslashes (`"\\") for compatibility with Windows file systems.
 * - Otherwise, the path uses forward slashes (`"/"`), the standard for Unix-based systems.
 *
 * ### Why Use This Approach?
 * - **Portability**: Ensures the code works seamlessly on different operating systems.
 * - **Maintainability**: Centralizes platform-specific differences in one place, reducing duplication.
 * - **Readability**: Simplifies the main logic by hiding the complexity of path handling.
 */
#if defined(_WIN32) || defined(_WIN64)
#define MAP_TRAINING() map_from_file("..\\maps\\training.map")
#else
#define MAP_TRAINING() map_from_file("../maps/training.map")
#endif

#endif /* _ROVERRUN_MAP_H_ */
