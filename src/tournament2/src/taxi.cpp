#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {

	


  //tell the action client that we want to spin a thread by default
  MoveBaseClient ac("move_base", true);
  //wait for the action server to come up
  while(!ac.waitForServer(ros::Duration(5.0))){
    ROS_INFO("Waiting for the move_base action server to come up");
  }
  move_base_msgs::MoveBaseGoal goal;
  //we'll send a goal to the robot to move 1 meter forward
  goal.target_pose.header.frame_id = "map";
  goal.target_pose.header.stamp = ros::Time::now();

  goal.target_pose.pose = markerArray->markers[0].pose;

  ROS_INFO("Sending goal");
  ac.sendGoal(goal);

  ac.waitForResult();

  if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    ROS_INFO("Hello Potter");
  else
    ROS_INFO("No face");

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "taxi");
	ros::NodeHandle nh;

  	// Create a ROS subscriber for the input point cloud
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/facemapper/markers", 1, callback);
	ros::spin();

  	//return 0;
}
