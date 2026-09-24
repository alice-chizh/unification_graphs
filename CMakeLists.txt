cmake_minimum_required(VERSION 3.28)
project(generalization_of_graph_unification)

set(CMAKE_CXX_STANDARD 17)

add_executable(generalization_of_graph_unification main.cpp
        Term.cpp
        Term.h
        Unifier.cpp
        Unifier.h
        Parser.cpp
        Bisimulation.cpp
)
