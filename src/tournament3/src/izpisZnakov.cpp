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
#include <move_base/move_base.h>
#include <stdlib.h> 

using namespace std;
static bool enkrat = true;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    
        if (markerArray->markers[0].id == 1) {      // slow   rosrun dynamic_reconfigure dynparam set /navigation_velocity_smoother speed_lim_v 0.1
            printf("ZNAK: Obvezna levo\n");
        } else if (markerArray->markers[0].id == 2) {      // stop
            printf("ZNAK: Omejitev hitrosti\n");
        } else if (markerArray->markers[0].id == 3) {      // stop
            printf("ZNAK: Enosmerna\n");
        } else if (markerArray->markers[0].id == 4) {      // stop
            printf("ZNAK: Stop\n");
        } else if (markerArray->markers[0].id == 5) {      // stop
            printf("ZNAK: Obvezno potrobi\n");
        }

}
int main(int argc, char** argv){
    ros::init(argc, argv, "izpisZnakov");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    ros::spin();

    //return 0;
}