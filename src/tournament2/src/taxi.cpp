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
static int size2 = 27;
static float listArray[27][4];//=9;
static bool goalApproved = false;
static int pathGoals=0;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;
bool inRange(float originalPoint, float point){
	float range = 0.25;
	if(originalPoint-range<point&&originalPoint+range>point)
		return true;
	return false;
}
void initList(){
	for(int i=0;i<size2;i++){
		for(int j=0;j<2;j++){
			listArray[i][j]=-80215;
		}
		listArray[i][2]=0;
		listArray[i][3]=0;
	}
}
void addToList(float x, float y){
	bool pointFound = false;
		for(int i=0;i<size2;i++){
			if(inRange(listArray[i][0],x)&&inRange(listArray[i][1],y)){
				if(listArray[i][3]==1){
					pointFound = true;
					continue;
				}
				if(listArray[i][2]>3){
					listArray[i][3]=1;
					goalApproved = true;
					printf("10 entries found: %f %f \n",x,y);
				}
				listArray[i][2]++;
				pointFound = true;
				printf("Same face at [%d]: %f %f: %f\n",i,x,y,listArray[i][2]);

			}
		}
		if(!pointFound){
			int min=0;
			bool added = false;
			while(!added){
				for(int i=0;i<size2;i++){
					if(listArray[i][2]==min){
						listArray[i][0]=x;
						listArray[i][1]=y;
						listArray[i][2]=1;
						listArray[i][3]=0;
						printf("New face at [%d]: %f %f\n",i,x,y);
						added=true;
						break;
					}
				}
				min++;
			}
		}
	
}
			static move_base_msgs::MoveBaseGoal createGoal(float xRobot, float yRobot, float xDirection, float yDirection){
				move_base_msgs::MoveBaseGoal goal;
				goal.target_pose.header.frame_id = "/map";
				tf::Vector3 v1 = tf::Vector3(1, 0 ,0);
				tf::Vector3 v2 = tf::Vector3(xDirection, yDirection+0.0000001,0);
				tf::Vector3 a = v1.cross(v2);
				tf::Quaternion q(a.x(), a.y(), a.z(), sqrt(v1.length2() * v2.length2()) + v1.dot(v2));
				q.normalize();
				goal.target_pose.pose.orientation.x = q.x();
				goal.target_pose.pose.orientation.y = q.y();
				goal.target_pose.pose.orientation.z = q.z();
				goal.target_pose.pose.orientation.w = q.w();
				goal.target_pose.pose.position.x = xRobot;
				goal.target_pose.pose.position.y = yRobot;
			return goal;
			}


void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {

	ros::NodeHandle node;
	ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );

	MoveBaseClient ac("move_base", true);
	//wait for the action server to come up
	while(!ac.waitForServer(ros::Duration(5.0))){
		ROS_INFO("Waiting for the move_base action server to come up");
	}
	
		for(int i=0; i < markerArray->markers.size(); i++){

		printf("%d marker\n",(int)markerArray->markers.size());

			move_base_msgs::MoveBaseGoal goal;
			goal.target_pose.header.frame_id = "/map";
			goal.target_pose.header.stamp = ros::Time::now();

				tf::TransformListener listener;
				tf::StampedTransform transform;
				listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10.0) );
				listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
				//listener.waitForTransform("/map", "/base_link", markerArray->markers[0].header.stamp, ros::Duration(10.0) );
				//listener.lookupTransform("/map", "/base_link", markerArray->markers[0].header.stamp, transform);

				float xRobot = transform.getOrigin().x();
				float yRobot = transform.getOrigin().y();

				printf("Robot: %f %f\n",xRobot,yRobot);

				

				float xFace = markerArray->markers[i].pose.position.x;
				float yFace = markerArray->markers[i].pose.position.y;
				float zFace = markerArray->markers[i].pose.position.z;

				tf::Stamped<tf::Pose> poseMarkerMap;
				try{
					tf::Stamped<tf::Pose> poseMarker(
				    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[i].pose.position.z)),
				    markerArray->markers[i].header.stamp, markerArray->markers[i].header.frame_id);

					listener.transformPose("/map", poseMarker, poseMarkerMap);
					printf("Time works\n");
				}catch(tf2::ExtrapolationException e){
					//printf("Error\n");

					tf::Stamped<tf::Pose> poseMarker(
				    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[i].pose.position.z)),
				    ros::Time(0), markerArray->markers[i].header.frame_id);

					listener.transformPose("/map", poseMarker, poseMarkerMap);
				}

				/*tf::Stamped<tf::Pose> posev1(
				    tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(0, 0, -1)),
				    ros::Time(0), markerArray->markers[0].header.frame_id);*/
				//printf("POSE %f %f\n", poseMarkerMap.getOrigin().x(), poseMarkerMap.getOrigin().y());
				xFace = poseMarkerMap.getOrigin().x();
				yFace = poseMarkerMap.getOrigin().y();
				zFace = poseMarkerMap.getOrigin().z();
				printf("Face: %f %f\n",xFace,yFace);
				if (zFace < 0.5 && zFace > 0.2)
					addToList(xFace,yFace);
				
				if(goalApproved){
					goalApproved=false;
					float xTarget,yTarget;

					xTarget=xFace-xRobot;
					yTarget=yFace-yRobot;
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

}
void pathMaker(move_base_msgs::MoveBaseGoal goal[]){
	for(;pathGoals<47;pathGoals++){
		//ros::spinOnce();
		//ros::getGlobalCallbackQueue()->callAvailable(ros::WallDuration(0));
		MoveBaseClient ac("move_base", true);
		while(!ac.waitForServer(ros::Duration(5.0))){
			ROS_INFO("Waiting for the move_base action server to come up");
		}
  		//printf("Goal: %f %f\n",xTarget,yTarget);
		ROS_INFO("Sending path goal %d",pathGoals);
		ac.sendGoal(goal[pathGoals]);
		ac.waitForResult();
		if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
			ROS_INFO("Reached goal %d",pathGoals);
			
			sleep(0.1);
			}
		else
			ROS_INFO("No go");
  	}

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "taxi");
	ros::NodeHandle nh;
	initList();
  	// Create a ROS subscriber for the input point cloud
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/facemapper/markers", 50, callback);
	//ros::spinOnce();

  	move_base_msgs::MoveBaseGoal goal[47];
  	goal[0]=createGoal(0,0,1,-1);
  	goal[1]=createGoal(0,0,0,-1);
  	goal[2]=createGoal(0,0,-1,-1);
  	goal[3]=createGoal(0,0,-1,0);
  	goal[4]=createGoal(0,0,-1,1);

  	goal[5]=createGoal(1.2,0,1,-1);
  	goal[6]=createGoal(1.2,0,0,-1);
  	goal[7]=createGoal(1.2,0,-1,-1);

  	goal[8]=createGoal(2.0,0,1,-1);
  	goal[9]=createGoal(2.0,0,0,-1);
  	goal[10]=createGoal(2.0,0,-1,-1);

  	goal[11]=createGoal(3.2,0,1,-1);
  	goal[12]=createGoal(3.2,0,0,-1);
  	goal[13]=createGoal(3.2,0,-1,-1);

  	goal[14]=createGoal(3.9,0,1,-1);
  	goal[15]=createGoal(3.9,0,0,-1);
  	goal[16]=createGoal(3.9,0,-1,-1);

  	goal[17]=createGoal(4.5,0,1,-1);
  	goal[18]=createGoal(4.5,0,0,-1);
  	goal[19]=createGoal(4.5,0,-1,-1);
  	goal[20]=createGoal(4.5,0,-1,0);
  	goal[21]=createGoal(4.5,0,0,1);
  	goal[22]=createGoal(4.5,0,1,1);


  	goal[23]=createGoal(3.3,1.1,0,1);
  	goal[24]=createGoal(3.3,1.1,1,1);
  	goal[25]=createGoal(3.3,1.1,1,0);
  	goal[26]=createGoal(3.3,1.1,-1,1);

  	goal[27]=createGoal(2.2,1,1,1);
  	goal[28]=createGoal(2.2,1,0,1);
  	goal[29]=createGoal(2.2,1,-1,1);

  	goal[30]=createGoal(1.1,1,1,1);
  	goal[31]=createGoal(1.1,1,0,1);
  	goal[32]=createGoal(1.1,1,-1,1);


  	goal[33]=createGoal(0.1,0.8,1,1);
  	goal[34]=createGoal(0.1,0.8,0,1);
  	goal[35]=createGoal(0.1,0.8,-1,1);
  	goal[36]=createGoal(0.1,0.8,-1,0);
  	goal[37]=createGoal(0.1,0.8,-1,-1);


  	goal[38]=createGoal(-0.5,2,1,0);
  	goal[39]=createGoal(-0.5,2,1,-1);
  	goal[40]=createGoal(-0.5,2,-1,1);
  	goal[41]=createGoal(-0.5,2,-1,0);
  	goal[42]=createGoal(-0.5,2,-1,-1);

  	goal[43]=createGoal(-0.8,3,1,0);
  	goal[44]=createGoal(-0.8,3,0,1);
  	goal[45]=createGoal(-0.8,3,-1,1);
  	goal[46]=createGoal(-0.8,3,0,-1);

  	pathMaker(goal);

	

  	//return 0;
}
