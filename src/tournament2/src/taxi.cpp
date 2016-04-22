#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <math.h> 
using namespace std;

static int s_id = 0;
static int m_id = -1;
static int countm = 0;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {

	ros::NodeHandle node;
	ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );

	MoveBaseClient ac("move_base", true);
	//wait for the action server to come up
	while(!ac.waitForServer(ros::Duration(5.0))){
		ROS_INFO("Waiting for the move_base action server to come up");
	}
	

		

		//tell the action client that we want to spin a thread by default

		move_base_msgs::MoveBaseGoal goal;
		//we'll send a goal to the robot to move 1 meter forward
		goal.target_pose.header.frame_id = "/map";
		goal.target_pose.header.stamp = ros::Time::now();

		
		if(true) {
			


			//float xFace = markerArray->markers[0].pose.position.x;
			//float yFace = markerArray->markers[0].pose.position.y;

			//float xFace = markerArray->markers[0].pose.position.x;
			//printf("Face: %f %f\n",xFace,yFace);

			tf::TransformListener listener;
			tf::StampedTransform transform;
			listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10.0) );
			listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
			//listener.waitForTransform("/map", "/base_link", markerArray->markers[0].header.stamp, ros::Duration(10.0) );
			//listener.lookupTransform("/map", "/base_link", markerArray->markers[0].header.stamp, transform);

			float xRobot = transform.getOrigin().x();
			float yRobot = transform.getOrigin().y();

			printf("Robot: %f %f\n",xRobot,yRobot);

			

			float xFace = markerArray->markers[0].pose.position.x;
			float yFace = markerArray->markers[0].pose.position.y;
			float zFace = markerArray->markers[0].pose.position.z;

			//tf::TransformListener listener2;
			//tf::StampedTransform transform2;
			//transform2.setOrigin( tf::Vector3(xRobot-xFace, yRobot-yFace, 0.0) );
	 		//transform2.setRotation( tf::Quaternion(0, 0, 0, 1) );
	 		//listener2.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10.0) );
			//listener2.lookupTransform("/map", "/base_link",ros::Time(0), transform2);
			tf::Stamped<tf::Pose> poseMarkerMap;
			try{
				tf::Stamped<tf::Pose> poseMarker(
			    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[0].pose.position.z)),
			    markerArray->markers[0].header.stamp, markerArray->markers[0].header.frame_id);

				listener.transformPose("/map", poseMarker, poseMarkerMap);
				printf("uspelo\n");
			}catch(tf2::ExtrapolationException e){
				printf("Error\n");

				tf::Stamped<tf::Pose> poseMarker(
			    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[0].pose.position.z)),
			    ros::Time(0), markerArray->markers[0].header.frame_id);

				listener.transformPose("/map", poseMarker, poseMarkerMap);
			}

			/*tf::Stamped<tf::Pose> posev1(
			    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, -1)),
			    ros::Time(0), markerArray->markers[0].header.frame_id);*/

			/*tf::Stamped<tf::Pose> trposev1;
				listener.transformPose("/map", posev1, trposev1);*/

			printf("POSE %f %f\n", poseMarkerMap.getOrigin().x(), poseMarkerMap.getOrigin().y());
			xFace = poseMarkerMap.getOrigin().x();
			yFace = poseMarkerMap.getOrigin().y();
			zFace = poseMarkerMap.getOrigin().z();
			printf("Face: %f %f\n",xFace,yFace);


			
			// geometry_msgs::pose = markerArray->markers[0].pose;

			
			float xTarget,yTarget;

			xTarget=xFace-xRobot;
			yTarget=yFace-yRobot;
			//xTarget -= 0.3;
			//yTarget -= 0.3;
			float dolzina = sqrt(pow(xTarget,2)+pow(yTarget,2));
			if(dolzina > 0.2 && zFace < 0.5 && zFace > 0.2){
				s_id = 1;
				xTarget /= dolzina;
				yTarget /= dolzina;

				xTarget *= (dolzina-0.5);
				yTarget *= (dolzina-0.5);

				

				
				

				
				tf::Vector3 v1 = tf::Vector3(1, 0 ,0);
				tf::Vector3 v2 = tf::Vector3(xTarget, yTarget ,0);
				tf::Vector3 a = v1.cross(v2);
				tf::Quaternion q(a.x(), a.y(), a.z(), sqrt(v1.length2() * v2.length2()) + v1.dot(v2));
				q.normalize();

				xTarget+=xRobot;
				yTarget+=yRobot;

				goal.target_pose.pose.orientation.x = q.x();//transform.getRotation().x();
				goal.target_pose.pose.orientation.y = q.y();//transform.getRotation().y();
				goal.target_pose.pose.orientation.z = q.z();//transform.getRotation().z();
				goal.target_pose.pose.orientation.w = q.w();//transform.getRotation().w();
				//goal.target_pose.pose.orientation = q;
				goal.target_pose.pose.position.x = xTarget;
				goal.target_pose.pose.position.y = yTarget;
				//goal.target_pose.pose =  markerArray->markers[0].pose;

				// Postavi marker kje je goal
				visualization_msgs::Marker  marker;
				marker.header.frame_id = "/map";
				marker.header.stamp = ros::Time();
				marker.ns = "my_namespace";
				marker.id = 1;
				marker.type = visualization_msgs::Marker::SPHERE;
				marker.action = visualization_msgs::Marker::ADD;
				marker.pose.position.x = xTarget;
				marker.pose.position.y = yTarget;
				marker.pose.position.z = 0;
				marker.scale.x = 0.2;
				marker.scale.y = 0.2;
				marker.scale.z = 0.2;
				marker.color.r = 0.0f;
	  			marker.color.g = 1.0f;
	  			marker.color.b = 0.0f;
				marker.color.a = 1.0; 
				marker.lifetime = ros::Duration();
				vis_pub.publish(marker);
				// konec markerja kje je goal


				printf("Goal: %f %f\n",xTarget,yTarget);
				ROS_INFO("Sending goal");
				ac.sendGoal(goal);

				ac.waitForResult();
				if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
					ROS_INFO("Hello Potter");
					s_id = 0;
					countm = 0;
				}
	    
			 	else
			    	ROS_INFO("Goal unreachable");
		}

	  
	  
	  
	}

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "taxi");
	ros::NodeHandle nh;

  	// Create a ROS subscriber for the input point cloud
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/facemapper/markers", 1, callback);
	ros::spin();

  	//return 0;
}
