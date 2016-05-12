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
using namespace std;


static int start;
static int getMilliCount(){
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

static int getMilliSpan(int nTimeStart){
	int nSpan = getMilliCount() - nTimeStart;
	if(nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}


void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	
	printf("Sem v callback in imam -- %d -- obrazov \n", (int)markerArray->markers.size());
	int milliSecondsElapsed = getMilliSpan(start);
	printf("\n\nElapsed time = %u milliseconds", milliSecondsElapsed);
	
}

int main(int argc, char** argv){
  	ros::init(argc, argv, "findFaces");
	ros::NodeHandle nh;
	start = getMilliCount();
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/markers", 100, callback);	
	ros::spin();
}