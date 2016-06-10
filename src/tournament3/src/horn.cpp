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
#include <stdlib.h> 

using namespace std;
static bool enkrat = true;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	printf("callback\n");
	if (enkrat) {
	    if (markerArray->markers[0].id == 5) {      // horn
	     	/*  ros::NodeHandle nh2;
	        
	        if(nh2.ok()){
			sound_play::SoundClient sc;
	            sleep(1);
	            sound_play::Sound s3 = sc.builtinSound(sound_play::SoundRequest::NEEDS_UNPLUGGING_BADLY);
	            s3.play();
	            enkrat = false;
	            sleep(5);
	            s3.stop();
	        }*/
		int i = system("rosrun sound_play play.py /home/team_zeta/catkin_ws/src/horn.wav");
		enkrat = false;
		sleep(30);
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
   /* ros::Rate r(0.3);
    while (ros::ok()){
      ros::spinOnce();               
      r.sleep();
    }*/
    
    //return 0;
}
