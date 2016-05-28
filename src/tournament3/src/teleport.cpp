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
#include <geometry_msgs/PoseWithCovariance.h>

using namespace std;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    if (markerArray->markers[0].id == 3) {      // one way
        
        ros::NodeHandle nh;

    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "oneway");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);
    ////////////////////

    tf::TransformListener listener(nh,ros::Duration(5),true);
    tf::StampedTransform transform;
    listener.waitForTransform("/map", "/base_link", ros::Time::now(), ros::Duration(10));
    listener.lookupTransform("/map", "/base_link", ros::Time::now(), transform);


    ros::Pose robotPos = transform.getOrigin().getPosition();
    robotPos.point.y+=110;

    ros::geometry_msgs::PoseWithCovarianceStamped position;
    position.pose = robotPos;

    pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/amcl/initialpose", 1);
    pub.publish(position);





    //////////////////////////////

    ros::spin();
    
    //return 0;
}
