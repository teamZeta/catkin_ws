#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <math.h>
#include <std_msgs/String.h>
#include <sstream>
#include <cstdlib>
#include <sys/timeb.h>

int main(int argc, char** argv){
    ros::init(argc, argv, "cancel");
    
    int i = system("rostopic pub /move_base/cancel actionlib_msgs/GoalID '{}'");
  
    ros::spin();
  
  }