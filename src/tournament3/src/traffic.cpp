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

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    if (markerArray->markers[0].id == 4) {      // horn
        sound_play::SoundClient sc;
        sc.play(sound_play::SoundRequest::NEEDS_UNPLUGGING);
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "traffic");
    //ros::NodeHandle nh;
		
		sound_play::SoundClient sc;
    sc.play(sound_play::SoundRequest::NEEDS_UNPLUGGING);
    //ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    //ros::spin();

  	return 0;
}
