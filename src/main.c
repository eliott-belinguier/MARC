#include <stdio.h>
#include "map.h"

int main() {
    map_s map;

    // The following preprocessor directive checks if the code is being compiled on a Windows system.
    // If either _WIN32 or _WIN64 is defined, it means we are on a Windows platform.
    // On Windows, file paths use backslashes (\), hence we use the appropriate file path for Windows.
#if defined(_WIN32) || defined(_WIN64)
    map = map_from_file("..\\maps\\example1.map");
#else
    map = map_from_file("../maps/example1.map");
#endif

    printf("Map created with dimensions %d x %d\n", map.height, map.width);
    for (int i = 0; i < map.height; i++)
    {
        for (int j = 0; j < map.width; j++)
        {
            printf("%d ", map.soils[i][j]);
        }
        printf("\n");
    }
    // printf the costs, aligned left 5 digits
    for (int i = 0; i < map.height; i++)
    {
        for (int j = 0; j < map.width; j++)
        {
            printf("%-5d ", map.costs[i][j]);
        }
        printf("\n");
    }
    map_display(map);
    return 0;
}
