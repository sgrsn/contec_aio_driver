cmake_minimum_required(VERSION 3.0.2)
project(contec_aio_driver)

## Compile as C++11, supported in ROS Noetic and newer
add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES my_modbus_pkg
  CATKIN_DEPENDS roscpp std_msgs
#  DEPENDS system_lib
)

include_directories(
 include
  ${catkin_INCLUDE_DIRS}
)

add_executable(adc_main src/adc_main.cpp)
target_link_libraries(adc_main ${catkin_LIBRARIES} caio)