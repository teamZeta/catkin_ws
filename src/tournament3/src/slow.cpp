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

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    ros::NodeHandle nh;
    geometry_msgs::Twist cmd_vel;
    ros::Publisher vel_pub_;
    vel_pub_ = nh.advertise<geometry_msgs::Twist>("/mobile_base/commands/velocity", 1);
    int st=0;
    if (markerArray->markers[0].id == 2) {      // slow   rosrun dynamic_reconfigure dynparam set /navigation_velocity_smoother speed_lim_v 0.1
        cmd_vel.linear.x = 0.1;
        cmd_vel.linear.y = 0.1;
        cmd_vel.angular.z = 0.0;
        st =0;
        while(ros::ok()&&st++<2000000)
            vel_pub_.publish(cmd_vel);
    }else if (markerArray->markers[0].id == 4) {      // stop
        cmd_vel.linear.x = 0.0;
        cmd_vel.linear.y = 0.0;
        cmd_vel.angular.z = 0.0;
        st =0;
        while(ros::ok()&&st++<2000000)
            vel_pub_.publish(cmd_vel);
    }
    sleep(15);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "slow");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);

    int i = system("rosrun dynamic_reconfigure dynparam set /navigation_velocity_smoother speed_lim_v 0.1");
/*
    ros::Publisher vel_pub_;
    vel_pub_ = nh.advertise<geometry_msgs::Twist>("/mobile_base/commands/velocity", 1);
    geometry_msgs::Twist cmd_vel;
    cmd_vel.linear.x = 0.0;
    cmd_vel.linear.y = 0.0;
    cmd_vel.angular.z = 0.0;
    int st =0;
    while(ros::ok()&&st++<2000000)
        vel_pub_.publish(cmd_vel);

    cmd_vel.linear.x = 0.1;
    cmd_vel.linear.y = 0.1;
    cmd_vel.angular.z = 0.0;
    st =0;
        while(ros::ok()&&st++<2000000)
        vel_pub_.publish(cmd_vel);
    ros::shutdown();
*/
    ros::spin();

    //return 0;
}