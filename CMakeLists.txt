#=========================== General Settings ==========================
# Set the minimum version of CMake
cmake_minimum_required(VERSION 3.16)

project(graphbuilder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_EXECUTABLE_SUFFIX ".out")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src)

# TODO: remove in release
# Set the default CMAKE_BUILD_TYPE to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D_DEBUG -ggdb3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Waggressive-loop-optimizations -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconversion -Wempty-body -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wopenmp-simd -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wsign-conversion -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wswitch-default -Wswitch-enum -Wsync-nand -Wunused -Wundef -Wunreachable-code -Wvariadic-macros -Wno-missing-field-initializers -Wno-narrowing -Wno-varargs -Wstack-protector -fcheck-new -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -pie -fPIE -Werror=vla")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr")
endif()
#=======================================================================


#=========================== Add executables ===========================
set(SRCS_MAIN main.cpp)
add_executable(rbtree ${SRCS_MAIN})
#=======================================================================


#=============================== Tests =================================
enable_testing()

# Adding gtest
find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)

set(SRCS_STRESS_TESTS src/tests/GoogleStressTests.cpp)
add_executable(stress_tests ${SRCS_STRESS_TESTS})

target_include_directories(stress_tests SYSTEM PUBLIC Threads::Threads ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})
set_property(TARGET stress_tests PROPERTY RUNTIME_OUTPUT_DIRECTORY "")

target_link_libraries(stress_tests ${GTEST_LIBRARIES})
add_test(NAME GoogleStressTests COMMAND stress_tests)

# Adding format test
set(CLANG_FORMAT_SCRIPT src/tests/clang_format_tests.sh)

set(SRCS_FOR_FORMAT 
    ${SRCS_MAIN} 
    src/RBtree.hpp
)
add_test(
    NAME FormatCheck
    COMMAND ${CLANG_FORMAT_SCRIPT} ${SRCS_FOR_FORMAT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Adding tidy test
set(CLANG_TIDY_SCRIPT 
    src/tests/clang_tidy_tests.sh
)
set(SRCS_FOR_TIDY
    ${SRCS_MAIN}  
)
add_test(
    NAME Tidycheck
    COMMAND ${CLANG_TIDY_SCRIPT} ${SRCS_FOR_TIDY}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
#=======================================================================