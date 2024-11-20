# Specifies the minimum version of CMake required to process this file.
# The VERSION argument ensures compatibility with features available in the specified CMake version.
cmake_minimum_required(VERSION 3.22)

# Defines a new project named "RoverRun" and specifies that it uses the C programming language.
# 'project()' initializes settings like the project name and language.
# Additionally, the name of the project ("RoverRun") is stored in the built-in variable ${CMAKE_PROJECT_NAME}.
# This variable is automatically populated and can be used later to refer to the project name dynamically.
project(RoverRun C)

# Sets the C standard to C99 for the project. This ensures that all C source files in the project are compiled using the C99 standard.
# 'CMAKE_C_STANDARD' is a built-in CMake variable for defining the C standard version.
set(CMAKE_C_STANDARD 99)

# Recursively finds all C source files within the `src` directory of the current project directory.
# 'file()' is a CMake command used to perform operations on files. Here, 'GLOB_RECURSE' searches all subdirectories.
# The resulting list of source files is stored in the variable 'SOURCES'.
# Variables in CMake, such as ${SOURCES}, are referenced with the ${} syntax.
# This syntax allows you to use the value of a variable dynamically in other parts of the script.
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Adds an executable target for the project. The name of the executable is dynamically assigned using ${CMAKE_PROJECT_NAME}.
# Using ${CMAKE_PROJECT_NAME} ensures that the target name matches the project name, making the script more reusable and adaptable.
# '${SOURCES}' specifies the source files to be used for this target.
# 'add_executable()' creates a build target that generates an executable binary.
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# Specifies include directories for the target.
# The `PRIVATE` keyword limits the scope of these include directories to only this target (RoverRun).
# 'target_include_directories()' is used to add include paths for header files.
# `${CMAKE_CURRENT_SOURCE_DIR}` is a built-in CMake variable that points to the root directory of the current CMake script.
# `${CMAKE_CURRENT_SOURCE_DIR}/include` dynamically constructs the path to the "include" directory in the project's root.
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)