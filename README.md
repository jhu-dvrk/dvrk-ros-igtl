da Vinci Research Kit ROS with OpenIGTLink
==========================================

This repository has code related to daVinci Research Kit (dVRK)
See https://github.com/jhu-dvrk/sawIntuitiveResearchKit/wiki

# Install 
* Download and compile the cisst libraries and SAW components for the dVRK, see the dVRK tutorial wiki: 
* Download in `catkin_ws/src`:
  * cisst-ros:
  * dvrk-ros:
  * dvrk-ros-igtl:
* Build: 
  ```sh
  catkin build
  ```

# How to run the code

```sh
  rosrun dvrk_robot_igtl dvrk_console_json -j <dvrk_console_config>.json -o <saw_openigtl_config>.json
```

