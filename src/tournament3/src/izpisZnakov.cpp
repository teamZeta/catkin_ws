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
static int once=-1;
void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
        
        if (markerArray->markers[0].id == 1 && once!=1) {      // slow   rosrun dynamic_reconfigure dynparam set /navigation_velocity_smoother speed_lim_v 0.1
            printf("Obvezna levo\n");
            once=1;
        } else if (markerArray->markers[0].id == 2 && once!=2) {      // stop
            printf("Omejitev hitrosti\n");
            once=2;
        } else if (markerArray->markers[0].id == 3 && once!=3) {      // stop
            printf("Enosmerna\n");
            once=3;
        } else if (markerArray->markers[0].id == 4 && once!=4) {      // stop
            printf("Stop\n");
            once=4;
        } else if (markerArray->markers[0].id == 5 && once!=5) {      // stop
            printf("Obvezno potrobi\n");
            once=5;
        }

}
int main(int argc, char** argv){
    ros::init(argc, argv, "izpisZnakov");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    ros::spin();

    //return 0;
}