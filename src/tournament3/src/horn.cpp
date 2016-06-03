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

using namespace std;
static bool enkrat = true;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	if (enkrat) {
	    if (markerArray->markers[0].id == 5) {      // horn
	        ros::NodeHandle nh2;
	        sound_play::SoundClient sc;
	        if(nh2.ok()){
	            sleep(1);
	            sound_play::Sound s3 = sc.builtinSound(sound_play::SoundRequest::NEEDS_UNPLUGGING_BADLY);
	            s3.play();
	            enkrat = false;
	            sleep(30);
	            s3.stop();
	        }
	    } 
	} else {
    	enkrat = true;
    }

}

int main(int argc, char** argv){
    ros::init(argc, argv, "horn");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    ros::spin();
    
    //return 0;
}