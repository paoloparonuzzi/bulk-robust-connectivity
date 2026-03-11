cmake_minimum_required(VERSION 3.16)
project(bulkRobustSpan LANGUAGES CXX)

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Build type default (se non specificato) ----
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- Gurobi ----
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
find_package(GUROBI REQUIRED)

message(STATUS "GUROBI_INCLUDE_DIRS   = ${GUROBI_INCLUDE_DIRS}")
message(STATUS "GUROBI_LIBRARY        = ${GUROBI_LIBRARY}")
message(STATUS "GUROBI_CXX_LIBRARY    = ${GUROBI_CXX_LIBRARY}")
message(STATUS "GUROBI_CXX_DEBUG_LIB  = ${GUROBI_CXX_DEBUG_LIBRARY}")

# ---- Executable ----
add_executable(bulkRobustSpan
        main.cpp
        Instance.cpp
        Problem.cpp
        flowModel.cpp
        flowSubProblem.cpp
        kruskal.cpp
        PolySubProblem.cpp
        masterModel.cpp
        BendersCut.cpp
        linearRelaxations.cpp
)

# ---- Include dirs ----
target_include_directories(bulkRobustSpan PRIVATE
        ${GUROBI_INCLUDE_DIRS}
        /usr/include/lemon
)

# ---- Compile options (solo GCC/Clang) ----
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bulkRobustSpan PRIVATE
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Debug>:-g>
    )
endif()

# ---- Link ----
target_link_libraries(bulkRobustSpan PRIVATE
        optimized ${GUROBI_CXX_LIBRARY}
        debug     ${GUROBI_CXX_DEBUG_LIBRARY}
        ${GUROBI_LIBRARY}
        lemon
        pthread
        m
        dl
)