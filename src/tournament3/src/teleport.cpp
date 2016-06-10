#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <std_msgs/String.h>
#include <sstream>
#include <cstdlib>
#include <sys/timeb.h>
#include <sound_play/sound_play.h>
#include <unistd.h>
#include <std_msgs/Header.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace std;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

static ros::Publisher posit;
static bool once = false;
static float diff=6.05;
static float dynamicDiff;
static bool reset=false;
static ros::Publisher chGoals;
static int destInd;

void changeMap(float x, float y){
    printf("Racunam novo mapo\n");
    int mapInd=0;
    if(y>3.6)
        mapInd=1;
    else if(y<-2.5)
        mapInd=-1;
    destInd=1;
    if(x<-2)
        destInd=-1;
	if(reset){
		destInd=0;
		reset=false;
	}

	dynamicDiff=(destInd-mapInd)*diff;
	printf("Koordinate %f , %f ",x,y);
	printf("Sem v: %d hocem v: %d teleport: %f",mapInd,destInd,dynamicDiff);
	if(dynamicDiff==0)
		once=false;
}

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	printf("vidim znak\n");
    if (markerArray->markers[0].id == 3 || markerArray->markers[0].id == 1) {      // one way
    	printf("uh oh one way\n");
        once=true;
        ros::NodeHandle nh;

    }
}

void callbackPose(const geometry_msgs::PoseWithCovarianceStamped msg){
	if(once)
		changeMap((float)msg.pose.pose.position.x,(float)msg.pose.pose.position.y);
    if (once) {
        geometry_msgs::PoseWithCovarianceStamped pos;
        pos.pose.pose.position.x = msg.pose.pose.position.x;
        pos.pose.pose.position.y = msg.pose.pose.position.y+dynamicDiff;
        pos.pose.pose.position.z = msg.pose.pose.position.z;
        pos.pose.pose.orientation.x = msg.pose.pose.orientation.x;
        pos.pose.pose.orientation.y = msg.pose.pose.orientation.y;
        pos.pose.pose.orientation.z = msg.pose.pose.orientation.z;
        pos.pose.pose.orientation.w = msg.pose.pose.orientation.w;

        posit.publish(pos);
        once = false;

        //MoveBaseClient ac("move_base", true);
        //ac.cancelGoal();

        //int i = system("rostopic pub /move_base/cancel actionlib_msgs/GoalID '{}'");

        std_msgs::String msg;
        std::stringstream ss;
        ss << destInd;
        msg.data = ss.str();
        chGoals.publish(msg);

        //Da goal na trenutnem mestu, da prekine prejsnji goal na stari mapi

        move_base_msgs::MoveBaseGoal goal;
        goal.target_pose.header.frame_id = "/map";
        goal.target_pose.pose.orientation.x = pos.pose.pose.orientation.x;
        goal.target_pose.pose.orientation.y = pos.pose.pose.orientation.y;
        goal.target_pose.pose.orientation.z = pos.pose.pose.orientation.z;
        goal.target_pose.pose.orientation.w = pos.pose.pose.orientation.w;
        goal.target_pose.pose.position.x = pos.pose.pose.position.x;
        goal.target_pose.pose.position.y = pos.pose.pose.position.y;
        goal.target_pose.pose.position.z = pos.pose.pose.position.z;

        MoveBaseClient ac("move_base", true);
            while(!ac.waitForServer(ros::Duration(5.0))){
                ROS_INFO("Waiting for the move_base action server to come up");
            }
            printf("Aborting current goal\n");
            ac.sendGoal(goal);
            ac.waitForResult();
            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
                ROS_INFO("Reached goal.");}

    }
}

void callbackReset(const std_msgs::String::ConstPtr& msg){
	reset=true;
    once = true;
}

int main(int argc, char** argv){
    ros::init(argc, argv, "teleport");
    ros::NodeHandle nh,nh2,nh3,nh4;
    posit = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);

    sleep(5);

    ros::Subscriber sub = nh2.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);
    ros::Subscriber sub2 = nh3.subscribe<geometry_msgs::PoseWithCovarianceStamped> ("/amcl_pose", 1, callbackPose);
    ros::Subscriber sub3 = nh4.subscribe<std_msgs::String>("/reset", 1, callbackReset);
    chGoals = nh.advertise<std_msgs::String>("/changeGoals", 1);
  
    ros::spin();
    
}
