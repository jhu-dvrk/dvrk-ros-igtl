#
# (C) Copyright 2015 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 2.8.3)
project (dvrk_robot_igtl)

## find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package (catkin REQUIRED
              COMPONENTS
              cisst_msgs
              cisst_ros_bridge
              dvrk_robot
              geometry_msgs
              roscpp
              std_msgs
              roslib
              )

# find cisst and make sure the required libraries have been compiled
set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstCommonXML
     cisstVector
     cisstVectorQt
     cisstOSAbstraction
     cisstMultiTask
     cisstMultiTaskQt
     cisstParameterTypes
     cisstParameterTypesQt 
     cisstRobot
     cisstNumerical
     cisstQt
     )

find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  include (${CISST_USE_FILE})

  # include_directories(include)
  include_directories (${catkin_INCLUDE_DIRS})

  catkin_package (CATKIN_DEPENDS cisst_msgs cisst_ros_bridge # dvrk_utilities
                                 geometry_msgs roscpp std_msgs
                  DEPENDS ${REQUIRED_CISST_LIBRARIES})

  # sawRobotIO1394 has been compiled within cisst, we should find it automatically
  cisst_find_saw_component (sawRobotIO1394 1.1.0 REQUIRED)
  cisst_find_saw_component (sawIntuitiveResearchKit 1.1.0 REQUIRED)
  cisst_find_saw_component (sawControllers 1.1.0 REQUIRED)
  cisst_find_saw_component (sawConstraintController REQUIRED)
  cisst_find_saw_component (sawOpenIGTLink REQUIRED)

  include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${sawRobotIO1394_INCLUDE_DIR}
    ${sawControllers_INCLUDE_DIR}
    ${sawConstraintController_INCLUDE_DIR}
    ${sawIntuitiveResearchKit_INCLUDE_DIR}
    ${sawOpenIGTLink_INCLUDE_DIR}
    )

  link_directories (
    ${sawRobotIO1394_LIBRARY_DIR}
    ${sawControllers_LIBRARY_DIR}
    ${sawConstraintController_LIBRARY_DIR}
    ${sawIntuitiveResearchKit_LIBRARY_DIR}
    ${sawOpenIGTLink_LIBRARY_DIR}
    )

  add_executable (dvrk_console_json src/dvrk_console_json.cpp)
  target_link_libraries (
      dvrk_console_json
      ${sawIntuitiveResearchKit_LIBRARIES}
      ${sawConstraintController_LIBRARIES}
      ${sawControllers_LIBRARIES}
      ${sawRobotIO1394_LIBRARIES}
      ${sawOpenIGTLink_LIBRARIES}
      ${catkin_LIBRARIES}
  )
  cisst_target_link_libraries (dvrk_console_json ${REQUIRED_CISST_LIBRARIES})

endif (cisst_FOUND_AS_REQUIRED)
