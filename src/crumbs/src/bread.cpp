#include <ros/ros.h>
//#include <tf/transform_broadcaster.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <turtlesim/Spawn.h>
using namespace std;


int main(int argc, char** argv){

  ros::init(argc, argv, "bread");

  ros::NodeHandle node;

//http://answers.ros.org/question/67492/marker-array-not-visible-in-rviz-but-is-getting-published/

	ros::Publisher vis_pub = node.advertise<visualization_msgs::MarkerArray>( "visualization_marker_array", 1 );


	tf::TransformListener listener;
 	visualization_msgs::MarkerArray ma;
	int st = 0;

  while (node.ok()){
 		 tf::StampedTransform transform;
    try{
			listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10.0) );
      listener.lookupTransform("/map", "/base_link",ros::Time(0), transform);
			visualization_msgs::Marker  marker;
			marker.header.frame_id = "/map";
			marker.header.stamp = ros::Time();
			marker.ns = "my_namespace";
			marker.id = st++;
			marker.type = visualization_msgs::Marker::SPHERE;
			marker.action = visualization_msgs::Marker::ADD;
			marker.pose.position.x = transform.getOrigin().x();
			marker.pose.position.y = transform.getOrigin().y();
			//marker.pose.position.x = 0;
			//marker.pose.position.y = 0;
			marker.pose.position.z = 0;
			marker.scale.x = 0.2;
			marker.scale.y = 0.2;
			marker.scale.z = 0.2;
			marker.color.r = 1.0f;
  		marker.color.g = 1.0f;
  		marker.color.b = 0.0f;
			marker.color.a = 1.0; 
			marker.lifetime = ros::Duration();
			ma.markers.push_back(marker);//=marker;
			vis_pub.publish(ma);
			sleep(2);
    }
    catch (tf::TransformException &ex) {
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
      continue;
    }
	}

  ros::spin();
  return 0;
}
