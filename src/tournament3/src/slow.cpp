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

using namespace std;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
        ros::NodeHandle nh;
        geometry_msgs::Twist cmd_vel;
        ros::Publisher vel_pub_;
        vel_pub_ = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);

    if (markerArray->markers[0].id == 2) {      // slow
        cmd_vel.linear.x = 0.1;
        cmd_vel.linear.y = 0.1;
        cmd_vel.angular.z = 0.1;
    }else if (markerArray->markers[0].id == 4) {      // stop
        cmd_vel.linear.x = 0.0;
        cmd_vel.linear.y = 0.0;
        cmd_vel.angular.z = 0.0;
    }

    vel_pub_.publish(cmd_vel);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "slow");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    ros::Publisher vel_pub_;
    vel_pub_ = nh.advertise<geometry_msgs::Twist>("/mobile_base/commands/velocity", 1);
    geometry_msgs::Twist cmd_vel;
    cmd_vel.linear.x = 0.1;
    cmd_vel.linear.y = 0.1;
    cmd_vel.angular.z = 0.1;
    while(ros::ok())
        vel_pub_.publish(cmd_vel);


    ros::spin();

    //return 0;
}