
cmake_minimum_required (VERSION 3.5)
project (RAYLIB_RAYTRACING)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/raylib ${CMAKE_BINARY_DIR}/raylib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/json ${CMAKE_BINARY_DIR}/json)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pugixml ${CMAKE_BINARY_DIR}/pugixml)

# Main Program
set(MAIN_SRC 
    src/main.cpp

    src/utility/tracelog.cpp
    src/utility/utility.cpp
    src/utility/utility-json.cpp
)

add_executable(RAYLIB_RAYTRACING ${MAIN_SRC})

# Link libraries
target_link_libraries(RAYLIB_RAYTRACING raylib)
target_link_libraries(RAYLIB_RAYTRACING nlohmann_json::nlohmann_json)


