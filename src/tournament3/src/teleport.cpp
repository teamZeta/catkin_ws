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
#include <geometry_msgs/PoseWithCovarianceStamped.h>

using namespace std;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    if (markerArray->markers[0].id == 3) {      // one way
        
        ros::NodeHandle nh;

    }
}

void callback2(const geometry_msgs::PoseWithCovarianceStamped msg){
    //msg.pose.pose.position.y+=110;
    geometry_msgs::PoseWithCovarianceStamped pos;
    pos.pose.pose.position.x = msg.pose.pose.position.x;
    pos.pose.pose.position.y = msg.pose.pose.position.y+110;
    pos.pose.pose.position.z = msg.pose.pose.position.z;
    pos.pose.pose.orientation.x = msg.pose.pose.orientation.x;
    pos.pose.pose.orientation.y = msg.pose.pose.orientation.y;
    pos.pose.pose.orientation.z = msg.pose.pose.orientation.z;
    pos.pose.pose.orientation.w = msg.pose.pose.orientation.w;

    ros::NodeHandle nh;
    ros::Publisher pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    pub.publish(pos);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "oneway");
    ros::NodeHandle nh;

    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callback);
    ////////////////////
    ros::NodeHandle nh2;
    ros::Subscriber sub2 = nh2.subscribe<geometry_msgs::PoseWithCovarianceStamped> ("/amcl_pose", 1, callback2);


    /*geometry_msgs::PoseWithCovarianceStamped pos;
    pos.pose.pose.position.x = transform.getOrigin().x();
    pos.pose.pose.position.y = transform.getOrigin().y()+110;
    pos.pose.pose.position.z = transform.getOrigin().z();
    pos.pose.pose.orientation.x = transform.getRotation().x();
    pos.pose.pose.orientation.y = transform.getRotation().y();
    pos.pose.pose.orientation.z = transform.getRotation().z();
    pos.pose.pose.orientation.w = transform.getRotation().w();
    //position.pose.position.y+=110;

    ros::Publisher pub = nh.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    pub.publish(pos);*/





    //////////////////////////////

    ros::spin();
    
    //return 0;
}
