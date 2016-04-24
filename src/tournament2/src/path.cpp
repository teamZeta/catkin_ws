#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <math.h>
#include <std_msgs/String.h>

using namespace std;

static bool goalApproved = false;
static int nGoal=0;
static move_base_msgs::MoveBaseGoal goal[47];
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

	if (!strcmp(msg->data.c_str(),"search")) {
		printf("Grem do naslednjega gola!\n");

		MoveBaseClient ac("move_base", true);
		while(!ac.waitForServer(ros::Duration(5.0))){
			ROS_INFO("Waiting for the move_base action server to come up");
		}
  		//printf("Goal: %f %f\n",xTarget,yTarget);
		ROS_INFO("Sending path goal %d",nGoal);
		ac.sendGoal(goal[nGoal]);
		ac.waitForResult();
		if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
			ROS_INFO("Reached goal %d",nGoal);
			sleep(0.1);
		} else
			ROS_INFO("No go");

		std_msgs::String msg;
    	std::stringstream ss;
    	ss << "pathEnded";
    	msg.data = ss.str();
    	pathEnded.publish(msg);
    	printf("posiljam pathEnded\n");

		nGoal++;

	}	

}

void goalInit() {

  	goal[0]=createGoal(0,0,1,-1);
  	goal[1]=createGoal(0,0,0,-1);
  	goal[2]=createGoal(0,0,-1,-1);
  	goal[3]=createGoal(0,0,-1,0);
  	goal[4]=createGoal(0,0,-1,1);

  	goal[5]=createGoal(1.2,0,1,-1);
  	goal[6]=createGoal(1.2,0,0,-1);
  	goal[7]=createGoal(1.2,0,-1,-1);

  	goal[8]=createGoal(2.0,0,1,-1);
  	goal[9]=createGoal(2.0,0,0,-1);
  	goal[10]=createGoal(2.0,0,-1,-1);

  	goal[11]=createGoal(3.2,0,1,-1);
  	goal[12]=createGoal(3.2,0,0,-1);
  	goal[13]=createGoal(3.2,0,-1,-1);

  	goal[14]=createGoal(3.9,0,1,-1);
  	goal[15]=createGoal(3.9,0,0,-1);
  	goal[16]=createGoal(3.9,0,-1,-1);

  	goal[17]=createGoal(4.5,0,1,-1);
  	goal[18]=createGoal(4.5,0,0,-1);
  	goal[19]=createGoal(4.5,0,-1,-1);
  	goal[20]=createGoal(4.5,0,-1,0);
  	goal[21]=createGoal(4.5,0,0,1);
  	goal[22]=createGoal(4.5,0,1,1);


  	goal[23]=createGoal(3.3,1.1,0,1);
  	goal[24]=createGoal(3.3,1.1,1,1);
  	goal[25]=createGoal(3.3,1.1,1,0);
  	goal[26]=createGoal(3.3,1.1,-1,1);

  	goal[27]=createGoal(2.2,1,1,1);
  	goal[28]=createGoal(2.2,1,0,1);
  	goal[29]=createGoal(2.2,1,-1,1);

  	goal[30]=createGoal(1.1,1,1,1);
  	goal[31]=createGoal(1.1,1,0,1);
  	goal[32]=createGoal(1.1,1,-1,1);


  	goal[33]=createGoal(0.1,0.8,1,1);
  	goal[34]=createGoal(0.1,0.8,0,1);
  	goal[35]=createGoal(0.1,0.8,-1,1);
  	goal[36]=createGoal(0.1,0.8,-1,0);
  	goal[37]=createGoal(0.1,0.8,-1,-1);


  	goal[38]=createGoal(-0.5,2,1,0);
  	goal[39]=createGoal(-0.5,2,1,-1);
  	goal[40]=createGoal(-0.5,2,-1,1);
  	goal[41]=createGoal(-0.5,2,-1,0);
  	goal[42]=createGoal(-0.5,2,-1,-1);

  	goal[43]=createGoal(-0.8,3,1,0);
  	goal[44]=createGoal(-0.8,3,0,1);
  	goal[45]=createGoal(-0.8,3,-1,1);
  	goal[46]=createGoal(-0.8,3,0,-1);

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "path");
	ros::NodeHandle nh;
	//printf("asdas");

  	goalInit();

  	ros::Subscriber sub = nh.subscribe("/tournament2/search", 1, callback);
  	pathEnded = nh.advertise<std_msgs::String>("/tournament2/talk", 2);

  	std_msgs::String msg;
    std::stringstream ss;
    ss << "pathStart";
    msg.data = ss.str();
    pathEnded.publish(msg);
    printf("posiljam pathStart\n");

  	ros::spin();


  	//return 0;
}
