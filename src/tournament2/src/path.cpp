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

static bool goalApproved = false;
static int nGoal=0;
static move_base_msgs::MoveBaseGoal goal[50];
static int maxGoal = 50;
static ros::Publisher pathEnded;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

static move_base_msgs::MoveBaseGoal createGoal(float xRobot, float yRobot, float xDirection, float yDirection){
	move_base_msgs::MoveBaseGoal goal;
	goal.target_pose.header.frame_id = "/map";
	tf::Vector3 v1 = tf::Vector3(1, 0 ,0);
	tf::Vector3 v2 = tf::Vector3(xDirection, yDirection+0.0000001,0);
	tf::Vector3 a = v1.cross(v2);
	tf::Quaternion q(a.x(), a.y(), a.z(), sqrt(v1.length2() * v2.length2()) + v1.dot(v2));
	q.normalize();
	goal.target_pose.pose.orientation.x = q.x();
	goal.target_pose.pose.orientation.y = q.y();
	goal.target_pose.pose.orientation.z = q.z();
	goal.target_pose.pose.orientation.w = q.w();
	goal.target_pose.pose.position.x = xRobot;
	goal.target_pose.pose.position.y = yRobot;
return goal;
}

void callback (const std_msgs::String::ConstPtr& msg) {

	if (!strcmp(msg->data.c_str(),"search") && nGoal < maxGoal) {
		printf("Grem do naslednjega gola!\n");

		MoveBaseClient ac("move_base", true);
		while(!ac.waitForServer(ros::Duration(5.0))){
			ROS_INFO("Waiting for the move_base action server to come up");
		}
  		//printf("Goal: %f %f\n",xTarget,yTarget);
		ROS_INFO("Sending path goal %d",nGoal);
		ac.sendGoal(goal[nGoal++]);
		ac.waitForResult();
		if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
			ROS_INFO("Reached goal %d",nGoal);
			sleep(1.1);
		} else
			ROS_INFO("No go");

		ros::Rate loop_rate(10);
		while (ros::ok()) {
			std_msgs::String msg;
	    	std::stringstream ss;
	    	ss << "pathEnded";
	    	msg.data = ss.str();
	    	pathEnded.publish(msg);
	    	printf("posiljam pathEnded\n");
			sleep(6);

			ros::spinOnce();
			
			loop_rate.sleep();
    	}

	} else if (!strcmp(msg->data.c_str(),"stop")) {
		ros::Rate loop_rate(10);
		while (ros::ok()) {
			std_msgs::String msg;
	    	std::stringstream ss;
	    	ss << "pathEnded";
	    	msg.data = ss.str();
	    	pathEnded.publish(msg);
	    	printf("posiljam pathEnded\n");
			sleep(6);

			ros::spinOnce();
			
			loop_rate.sleep();
    	}
		
	}
	if (nGoal > maxGoal) {
		ros::shutdown();
	}

}

void goalInit() {
	int i=0;
  	goal[i++]=createGoal(-0.2,0.2,0,-1);
  	goal[i++]=createGoal(-0.2,0.2,-1,0);
  	goal[i++]=createGoal(-0.2,0.2,-1,1);
//3
  	goal[i++]=createGoal(0.4,0.2,1,-1);
  	goal[i++]=createGoal(0.4,0.2,-1,-1);
//5
  	goal[i++]=createGoal(1,0.2,1,-1);
  	goal[i++]=createGoal(1,0.2,0,-1);
  	goal[i++]=createGoal(1,0.2,-1,-1);
//8
  	goal[i++]=createGoal(1.8,0.2,-1,-1);

  	goal[i++]=createGoal(2.2,0.2,0,-1);

  	goal[i++]=createGoal(2.8,0.2,1,-1);
  	goal[i++]=createGoal(2.8,0.2,-1,-1);

  	goal[i++]=createGoal(3,0.2,0,-1);

  	goal[i++]=createGoal(3.9,0,1,1);
  	goal[i++]=createGoal(3.9,0,1,-1);
  	goal[i++]=createGoal(3.9,0,0,-1);
  	goal[i++]=createGoal(3.9,0,-1,-1);

  	goal[i++]=createGoal(4.5,0,1,-1);
  	goal[i++]=createGoal(4.5,0,0,-1);
  	goal[i++]=createGoal(4.5,0,-1,-1);
  	goal[i++]=createGoal(4.5,0,-1,0);
  	goal[i++]=createGoal(4.5,0,0,1);
  	goal[i++]=createGoal(4.5,0,1,1);


  	goal[i++]=createGoal(3.3,0.9,0,1);
  	goal[i++]=createGoal(3.3,0.9,1,1);
  	goal[i++]=createGoal(3.3,0.9,1,0);
  	goal[i++]=createGoal(3.3,0.9,-1,1);

	goal[i++]=createGoal(2.8,0.9,1,0);
  	goal[i++]=createGoal(2.8,0.9,1,1);
  	goal[i++]=createGoal(2.8,0.9,-1,1);

  	goal[i++]=createGoal(2.2,0.8,0,1);

  	goal[i++]=createGoal(1.7,0.8,1,1);
  	goal[i++]=createGoal(1.7,0.8,-1,1);

  	goal[i++]=createGoal(0.9,0.8,0,1);


  	goal[i++]=createGoal(0.1,0.8,1,1);
  	goal[i++]=createGoal(0.1,0.8,0,1);
  	goal[i++]=createGoal(0.1,0.8,-1,1);
  	goal[i++]=createGoal(0.1,0.8,-1,0);
  	goal[i++]=createGoal(0.1,0.8,-1,-1);


  	goal[i++]=createGoal(-0.5,2,1,0);
  	goal[i++]=createGoal(-0.5,2,1,-1);
  	goal[i++]=createGoal(-0.5,2,-1,1);
  	goal[i++]=createGoal(-0.5,2,-1,0);
  	goal[i++]=createGoal(-0.5,2,-1,-1);

  	goal[i++]=createGoal(-0.8,3,1,0);
  	goal[i++]=createGoal(-0.8,3,0,1);
  	goal[i++]=createGoal(-0.8,3,-1,1);
  	goal[i++]=createGoal(-0.8,3,0,-1);

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "path");
	ros::NodeHandle nh;
	ros::NodeHandle nh2;
	//printf("asdas");

  	goalInit();

  	ros::Subscriber sub = nh.subscribe<std_msgs::String>("/tournament2/search", 1, callback);
  	pathEnded = nh2.advertise<std_msgs::String>("/tournament2/talk", 1);

  	ros::Rate loop_rate(10);

  	while (ros::ok()) {

  		std_msgs::String msg;
    	std::stringstream ss;
    	ss << "pathStart";
    	msg.data = ss.str();
    	pathEnded.publish(msg);
    	printf("posiljam pathStart\n");

    	ros::spinOnce();

    	loop_rate.sleep();

  	}

  	

  	ros::spin();


  	//return 0;
}
