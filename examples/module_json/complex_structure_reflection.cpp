#include "include/UTL/json.hpp"

#include <iostream>

// Set up some complex nested structs
struct Point {
    double x, y, z;
};

struct Task {
    std::string input_path;
    std::string output_path;
    double      time_limit;
};

struct TaskList {
    std::map<std::string, Task> map_of_tasks;
    // this is fine
    
    std::vector<std::vector<Point>> matrix_of_points;
    // this is also fine
    
    // std::vector<std::vector<std::vector<std::map<std::string, Point>>>> tensor_of_maps_of_points;
    // ... this would also be fine
    
    // std::array<std::unordered_map<std::string, Task>, 4> array_of_maps_of_tasks;
    // ... and so will be this
};

UTL_JSON_REFLECT(Point, x, y, z);
UTL_JSON_REFLECT(Task, input_path, output_path, time_limit);
UTL_JSON_REFLECT(TaskList, map_of_tasks, matrix_of_points);

int main() {
    using namespace utl;

    const TaskList task_list = {
        // Map of tasks
        {
            { "task_1", { "input_1.dat", "output_1.dat", 170. } },
            { "task_2", { "input_2.dat", "output_2.dat", 185. } }
        },
        // Matrix of 3D points
        {
            { { 0, 0, 0}, { 1, 0, 0 } },
            { { 0, 1, 0}, { 0, 0, 1 } }
        }
    };

    // Parse JSON from struct,
    // this also doubles as a way of stringifying structs for debugging
    std::cout << json::from_struct(task_list).to_string();
}