cmake_minimum_required(VERSION 2.8.3)
project(spark_test)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  pcl_ros
  
)


###################################
## catkin specific configuration ##
###################################=
catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES spark_test
#  CATKIN_DEPENDS roscpp rospy std_msgs
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include)
include_directories(
  ${catkin_INCLUDE_DIRS}
)



add_executable(spark_test_topic_node src/spark_test_topic.cpp)
target_link_libraries(spark_test_topic_node ${catkin_LIBRARIES})#if u want to play sound and asr, u need to add "nx_voioce and "msc".
add_dependencies(spark_test_topic_node ${PROJECT_NAME}_generate_messages_cpp spark_base)

add_executable(spark_test_motion_node src/spark_test_motion.cpp)
target_link_libraries(spark_test_motion_node ${catkin_LIBRARIES})#if u want to play sound and asr, u need to add "nx_voioce and "msc".
add_dependencies(spark_test_motion_node ${PROJECT_NAME}_generate_messages_cpp spark_base)


#############
## Install ##
#############

install(TARGETS spark_test_topic_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(TARGETS spark_test_motion_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)
 
install(DIRECTORY
  launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
