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
    if (markerArray->markers[0].id == 5) {      // one way
        
        ros::NodeHandle nh4;
        tf::TransformListener listener(nh4,ros::Duration(5),true);
        tf::StampedTransform transform;
        listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10));
        listener.lookupTransform("/map", "/base_link", ros::Time::now(), transform);
                
        float xRobot = transform.getOrigin().x();
        float yRobot = transform.getOrigin().y();
        float zRobot = transform.getOrigin().z();

        transform.setOrigin(tf::Vector3(xRobot, yRobot+110, zRobot));
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "oneway");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    ros::spin();
    
    //return 0;
}