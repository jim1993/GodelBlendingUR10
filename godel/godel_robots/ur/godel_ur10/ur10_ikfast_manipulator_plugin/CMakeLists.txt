cmake_minimum_required(VERSION 2.8.12)
project(ur10_ikfast_manipulator_plugin)
add_definitions(-std=c++11)

set(LAPACK_lapack_WORKS TRUE)

find_package(catkin REQUIRED COMPONENTS
        moveit_core
        pluginlib
        roscpp
        tf_conversions
        )

find_package(LAPACK)
include_directories(${catkin_INCLUDE_DIRS})

catkin_package(
        INCLUDE_DIRS
            include
        LIBRARIES
            ${LAPACK_LIBRARIES}
        CATKIN_DEPENDS
            moveit_core
            pluginlib
            roscpp
            tf_conversions
)

include_directories(include
        ${catkin_INCLUDE_DIRS}
        )

set(IKFAST_LIBRARY_NAME ur10_manipulator_moveit_ikfast_plugin)

find_package(LAPACK REQUIRED)

add_library(${IKFAST_LIBRARY_NAME} src/ur10_manipulator_ikfast_moveit_plugin.cpp)

target_link_libraries(${IKFAST_LIBRARY_NAME} ${catkin_LIBRARIES} ${Boost_LIBRARIES} ${LAPACK_LIBRARIES})

install(TARGETS ${IKFAST_LIBRARY_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})


install(
        FILES
        ur10_manipulator_moveit_ikfast_plugin_description.xml
        DESTINATION
        ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
