cmake_minimum_required(VERSION 3.8)
project(mecabot_bt_cpp)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)

add_executable(bt_example src/main.cpp)

ament_target_dependencies(bt_example rclcpp behaviortree_cpp_v3)

install(TARGETS bt_example
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY trees/
  DESTINATION share/${PROJECT_NAME}/trees)

ament_package()

