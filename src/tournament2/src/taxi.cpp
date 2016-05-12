#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <math.h> 
#include <std_msgs/String.h>
#include <sstream>
#include <cstdlib>
#include <sys/timeb.h>
using namespace std;

static int size2 = 27;
static float listArray[27][4];//=9;
static bool goalApproved = false;
static ros::Publisher pathSearch;
static bool faceFound = false;
//static bool pocakaj = false;
//static bool goToFace = false;
static int faceCount =0;
static move_base_msgs::MoveBaseGoal goal;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;


static int start;
static int getMilliCount(){
	timeb tb;
	ftime(&tb);
	int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
	return nCount;
}

static int getMilliSpan(int nTimeStart){
	int nSpan = getMilliCount() - nTimeStart;
	if(nSpan < 0)
		nSpan += 0x100000 * 1000;
	return nSpan;
}


bool inRange(float originalPoint, float point){
	float range = 0.4;
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

				int entries = 6;
				if(listArray[i][3]==1){
					pointFound = true;
					continue;
				}
				if(listArray[i][2]>=entries){
					listArray[i][3]=1;
					goalApproved = true;
					faceFound = true;
					printf("%d entries found: %f %f \n",entries,x,y);
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

void callback (const visualization_msgs::MarkerArrayConstPtr& markerArray) {

	ros::NodeHandle node;
	ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );
	//printf(ros::Time::now());
	/*MoveBaseClient ac("move_base", true);
	//wait for the action server to come up
	while(!ac.waitForServer(ros::Duration(5.0))){
		ROS_INFO("Waiting for the move_base action server to come up");
	}*/
		printf("Sem v callback in imam -- %d -- obrazov \n", (int)markerArray->markers.size());
		int milliSecondsElapsed = getMilliSpan(start);
		printf("Elapsed time = %u milliseconds\n", milliSecondsElapsed);
		for(int i=0; i < markerArray->markers.size(); i++){

		//printf("%d marker\n",(int)markerArray->markers.size());
			/*
			//move_base_msgs::MoveBaseGoal goal;
			goal.target_pose.header.frame_id = "/map";
			goal.target_pose.header.stamp = ros::Time::now();

				tf::TransformListener listener;
				tf::StampedTransform transform;
				//listener.waitForTransform("/map", "/base_link", ros::Time::now(), ros::Duration(10.0) );
				//listener.lookupTransform("/map", "/base_link", ros::Time::now(), transform);
				listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10.0) );
				listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
				//listener.waitForTransform("/map", "/base_link", markerArray->markers[0].header.stamp, ros::Duration(10.0) );
				//listener.lookupTransform("/map", "/base_link", markerArray->markers[0].header.stamp, transform);

				float xRobot = transform.getOrigin().x();
				float yRobot = transform.getOrigin().y();

				//printf("Robot: %f %f\n",xRobot,yRobot);

				

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
				/*
				xFace = poseMarkerMap.getOrigin().x();
				yFace = poseMarkerMap.getOrigin().y();
				zFace = poseMarkerMap.getOrigin().z();

				float xTarget,yTarget;

				xTarget=xFace-xRobot;
				yTarget=yFace-yRobot;
				float dolzina = sqrt(pow(xTarget,2)+pow(yTarget,2));

				printf("I see a face: %f %f\n",xFace,yFace);
				if (zFace < 0.45 && zFace > 0.2 && dolzina<1.8)
					addToList(xFace,yFace);
	// TODO: goal approved za pravi marker			
				if(goalApproved && dolzina<1.8){
					if(zFace < 0.45 && zFace > 0.2 && dolzina<1.8){
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
					    printf("\nSetting Goal: %f %f\n\n",xTarget,yTarget);
					    goalApproved = false;
				}

	 	 }
	  
	 */ 
	}

}

void callbackTalk (const std_msgs::String::ConstPtr& msg1) {
	//printf("TAXI: dobil sem msg\n");



	printf("Dobil sem %s\n",msg1->data.c_str());
	if (!strcmp(msg1->data.c_str(),"pathEnded") && faceFound){

		//ros::Rate loop_rate(10);
		//while (ros::ok()) {
			printf("posiljam stop\n");
	  		std_msgs::String msg;
	    	std::stringstream ss;
	    	ss << "stop";
	    	msg.data = ss.str();
	    	pathSearch.publish(msg);
	    	
	    	//ros::spinOnce();

	  //  	loop_rate.sleep();
	  //	}

		MoveBaseClient ac("move_base", true);
		//wait for the action server to come up
		while(!ac.waitForServer(ros::Duration(5.0))){
			ROS_INFO("Waiting for the move_base action server to come up");
		}



		ROS_INFO("Sending goal");
		ac.sendGoal(goal);

		ac.waitForResult(ros::Duration(5.0));
		if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
			faceCount++;
			printf("\n::::::::::::::::::::::::::::::::::::::\n");
			ROS_INFO("Hello Potter");
			printf("::::::::::::::::::::::::::::::::::::::\n");
			//if(faceCount>=4)
			//	ros::shutdown();
			sleep(2);
		}else{
			ROS_INFO("Goal unreachable: %s\n",ac.getState().toString().c_str());

			ac.sendGoal(goal);
			ac.waitForResult(ros::Duration(5.0));
			if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
				faceCount++;
				printf("\n::::::::::::::::::::::::::::::::::::::\n");
				ROS_INFO("Hello Potter");
				printf("::::::::::::::::::::::::::::::::::::::\n");
				//if(faceCount>=3)
				//	ros::shutdown();
				sleep(2);
			}else{
				ROS_INFO("Goal unreachable2: %s\n",ac.getState().toString().c_str());
			}

			sleep(2);
		}

		faceFound = false;

		/*while (ros::ok()) {
		    printf("prisu sem do face, isci naprej\n");
			std_msgs::String msg;
	    	std::stringstream ss;
	    	ss << "search";
	    	msg.data = ss.str();
			pathSearch.publish(msg);
	    	ros::Rate loop_rate(10);
			ros::spinOnce();
			loop_rate.sleep();
		}*/

	}

	if (!faceFound) {
		//printf("posiljam search\n");
		ros::Rate loop_rate(10);
	  	while (ros::ok()) {
	  		printf("posiljam search\n");
	  		std_msgs::String msg;
	    	std::stringstream ss;
	    	ss << "search";
	    	msg.data = ss.str();
	    	pathSearch.publish(msg);
	    	
    		ros::spinOnce();

	    	loop_rate.sleep();
	  	}

	}
		if (faceFound) {
			ros::Rate loop_rate(10);
			while (ros::ok()) {
				printf("posiljam stop\n");
		  		std_msgs::String msg;
		    	std::stringstream ss;
		    	ss << "stop";
		    	msg.data = ss.str();
		    	pathSearch.publish(msg);
		    	
		    	ros::spinOnce();

		    	loop_rate.sleep();
		  	}
	}
}

int main(int argc, char** argv){
	start = getMilliCount();
  	ros::init(argc, argv, "taxi");
  	ros::NodeHandle nh3("nh3");
	ros::NodeHandle nh2;
	ros::NodeHandle nh(nh3, "nh");
	initList();
  	// Create a ROS subscriber for the input point cloud
  	//pathSearch = nh2.advertise<std_msgs::String>("/tournament2/search", 1);
  //	ros::Subscriber subTalk = nh3.subscribe<std_msgs::String>("/tournament2/talk", 1, callbackTalk);
  	ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/markers", 100, callback);	
  	//printf("sem pred spinom\n");
	ros::spin();

  	//return 0;
}
