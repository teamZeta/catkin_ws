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
using namespace std;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	printf("Sem v callback in imam -- %d -- obrazov \n", (int)markerArray->markers.size());
	
}

int main(int argc, char** argv){
  	ros::init(argc, argv, "findFaces");
	ros::NodeHandle nh;
	
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/markers", 100, callback);	
	ros::spin();
}