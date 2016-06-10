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

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char** argv){
    ros::init(argc, argv, "clearCostmap");
    
    ros::NodeHandle nh;

    ros::ServiceClient client = nh.serviceClient<MoveBaseAction::Foo>("/move_base/clear_costmaps");
	MoveBaseAction::Foo foo;

	client.call(foo)
	
    //int i = system("rostopic pub /move_base/cancel actionlib_msgs/GoalID '{}'");
	MoveBaseClient ac("move_base", true);
    ac.cancelGoal();  

    ros::spin();
  
  }