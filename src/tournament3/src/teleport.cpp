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

static ros::Publisher posit;
static bool once = false;
static float diff=5.5;
static float dynamicDiff;
static bool reset=false;
void changeMap(float x, float y){
	int mapInd=0;
	if(y>3)
		mapInd=1;
	else if(y<-0.9)
		mapInd=-1;
	int destInd=1;
	if(x<-3)
		destInd=-1;
	if(reset)
		destInd=0;

	dynamicDiff=(destInd-mapInd)*diff;
	if(dynamicDiff==0)
		once=false;
}

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    if (markerArray->markers[0].id == 3) {      // one way
        once=true;
        ros::NodeHandle nh;

    }
}

void callback2(const geometry_msgs::PoseWithCovarianceStamped msg){
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
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "teleport");
    ros::NodeHandle nh,nh2,nh3;
    posit = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);

    sleep(5);

    ros::Subscriber sub = nh2.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);
    ros::Subscriber sub2 = nh3.subscribe<geometry_msgs::PoseWithCovarianceStamped> ("/amcl_pose", 1, callback2);
  
    ros::spin();
    
}
