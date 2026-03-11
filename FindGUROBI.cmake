find_path(GUROBI_INCLUDE_DIRS
        NAMES gurobi_c.h
        HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
        PATH_SUFFIXES include)

find_library(GUROBI_LIBRARY
        NAMES gurobi gurobi100 gurobi110 gurobi120 gurobi130 gurobi140
        HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
        PATH_SUFFIXES lib)

find_library(GUROBI_CXX_LIBRARY
        NAMES gurobi_c++ gurobi_g++8.5 gurobi_g++9 gurobi_g++10 gurobi_g++11 gurobi_g++12 gurobi_g++13 gurobi_g++14
        HINTS ${GUROBI_DIR} $ENV{GUROBI_HOME}
        PATH_SUFFIXES lib)
set(GUROBI_CXX_DEBUG_LIBRARY ${GUROBI_CXX_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GUROBI DEFAULT_MSG
        GUROBI_LIBRARY
        GUROBI_INCLUDE_DIRS
        GUROBI_CXX_LIBRARY
)