cmake_minimum_required(VERSION 3.10)

# Message
message(STATUS "2021 UBC Ethan Pan")
message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS ${PROJECT_SOURCE_DIR})

# set the project name
project(OneStepRouter)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add src files
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# include_directories(${SRC_DIR}/../easygl)
set(SRC_FILES ${SRC_DIR}/Circuit.cpp ${SRC_DIR}/Cell.cpp ${SRC_DIR}/main.cpp ${SRC_DIR}/graphics.h ${SRC_DIR}/graphics.cpp ${SRC_DIR}/draw.cpp)

######################
# Check Dependencies #
######################
# uncommment this line if you are using self-compiled boost lib
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem program_options REQUIRED)

include_directories(/usr/openwin/lib)
# link_directories(${SRC_FILES}/../easygl)
include_directories(/usr/openwin/include)

# add X11
find_package(X11 REQUIRED)


include_directories(${X11_INCLUDE_DIR})

link_directories(${X11_LIBRARIES})
# add the executable
add_executable(OneStepRouter ${SRC_FILES})


# target_link_libraries(OneStepRouter ${X11_LIBRARIES})

# target_link_libraries(OneSteoRouter PUBLIC /usr/openwin/lib)
# target_link_libraries(OneSteoRouter PUBLIC /usr/openwin/include)

target_include_directories(OneStepRouter PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

# Boost
target_include_directories(OneStepRouter PUBLIC ${Boost_INCLUDE_DIR} /usr/openwin/lib /usr/openwin/include)
target_link_libraries(OneStepRouter PUBLIC ${Boost_LIBRARIES} ${X11_LIBRARIES})
