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
#include <unistd.h>
#include <std_msgs/Header.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <string>

using namespace std;

static bool isciOsebo = false;
static bool isciHotel = false;
static int onceId = -1;
static move_base_msgs::MoveBaseGoal goal;
static int currentGoal = -1;
static bool foundFace = false;
static bool foundHotel = false;
static ros::Publisher updateTaxi;
static string osebe[] = {"harry", "philip", "tina", "peter", "tom", "ellen", "kim", "scarlet", "matthew"};
static string streetName[] = {"red","green","blue","yellow"};
static int iskanaOsebaID = 0;
static int iskanHotelID = 0; //rgby 
static int goalInd=0;
static bool oseba = true;
static bool found = false;
static ros::Publisher pathSearch;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

bool cmp(string s1, string s2){
    int i;
    for (i=0; s1[i]; i++) s1[i] = tolower(s1[i]);
    for (i=0; s1[i]; i++) s2[i] = tolower(s2[i]);
    return !s1.compare(s2);
}

/*void callbackTeleport(const geometry_msgs::PoseWithCovarianceStamped msg){ //Zamenja goale na trenutno mapo
	printf("Callback za menjavo goalov\n");
    changeGoalMap((float)msg.pose.pose.position.x,(float)msg.pose.pose.position.y);
}*/
// ::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::: FOUND FACE ::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::
void callbackFoundFace (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    if (!isciOsebo)
        return;
    if(onceId!=markerArray->markers[0].id){
        printf("Detected: %s\n",osebe[markerArray->markers[0].id-1].c_str());
        onceId=markerArray->markers[0].id;
    }
    if (iskanaOsebaID != markerArray->markers[0].id) {
        return;
    }
    found = true;

    //printf("posiljam stop\n");
    std_msgs::String msg;
    std::stringstream ss;
    ss << "face";
    msg.data = ss.str();
    pathSearch.publish(msg);

    foundFace = true;
    printf("I found %s.\n", osebe[iskanaOsebaID-1].c_str());

   /* ros::NodeHandle node;
    ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );
    */
    goal.target_pose.header.frame_id = "/map";
    goal.target_pose.header.stamp = ros::Time::now();
    tf::TransformListener listener;
    tf::StampedTransform transform;
    listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(5));
    listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
                

    float xRobot = transform.getOrigin().x();
    float yRobot = transform.getOrigin().y();
    float xFace = markerArray->markers[0].pose.position.x;
    float yFace = markerArray->markers[0].pose.position.y;
    float zFace = markerArray->markers[0].pose.position.z;

    tf::Stamped<tf::Pose> poseMarkerMap;
    try {
        tf::Stamped<tf::Pose> poseMarker(
        tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[0].pose.position.z)),
        markerArray->markers[0].header.stamp, markerArray->markers[0].header.frame_id);

        listener.transformPose("/map", poseMarker, poseMarkerMap);
        printf("Time works\n");
    } catch(tf2::ExtrapolationException e) {
        tf::Stamped<tf::Pose> poseMarker(
        tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[0].pose.position.z)),
        ros::Time(0), markerArray->markers[0].header.frame_id);

        listener.transformPose("/map", poseMarker, poseMarkerMap);
    }

    xFace = poseMarkerMap.getOrigin().x();
    yFace = poseMarkerMap.getOrigin().y();
    zFace = poseMarkerMap.getOrigin().z();

    float xTarget,yTarget;

    xTarget=xFace-xRobot;
    yTarget=yFace-yRobot;
    float dolzina = sqrt(pow(xTarget,2)+pow(yTarget,2));

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
    goal.target_pose.pose.position.x = xTarget;
    goal.target_pose.pose.position.y = yTarget;
/*
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
*/
    //MoveBaseClient ac("move_base", true);
    //wait for the action server to come up
   /* while(!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("Waiting for the move_base action server to come up");
    }*/


}

void callbackHotel (const visualization_msgs::MarkerConstPtr& marker) {
    if (!isciHotel)
        return;
    printf("Detected: %s\n",streetName[marker->id-1].c_str());
    if (iskanHotelID != marker->id) {
        return;
    }
    found = true;

    //printf("posiljam stop\n");
    std_msgs::String msg;
    std::stringstream ss;
    ss << "hotel";
    msg.data = ss.str();
    pathSearch.publish(msg);

    if (!foundHotel) {
        printf("I found the %s Hotel.'\n",streetName[iskanHotelID-1].c_str());
        std::stringstream ss3;
        ss3 << "rosrun sound_play say.py \"I found the " << streetName[iskanHotelID-1].c_str() << " hotel\"";
        int i = std::system(ss3.str().c_str());
    }

    foundHotel = true;

   /* ros::NodeHandle node;
    ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );
    */
    goal.target_pose.header.frame_id = "/map";
    goal.target_pose.header.stamp = ros::Time::now();
    tf::TransformListener listener;
    tf::StampedTransform transform;
    listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(5));
    listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
                

    float xRobot = transform.getOrigin().x();
    float yRobot = transform.getOrigin().y();
    float xFace = marker->pose.position.x;
    float yFace = marker->pose.position.y;
    float zFace = marker->pose.position.z;

    tf::Stamped<tf::Pose> poseMarkerMap;
    try {
        tf::Stamped<tf::Pose> poseMarker(
        tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, marker->pose.position.z)),
        marker->header.stamp, marker->header.frame_id);

        listener.transformPose("/map", poseMarker, poseMarkerMap);
        printf("Time works\n");
    } catch(tf2::ExtrapolationException e) {
        tf::Stamped<tf::Pose> poseMarker(
        tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, marker->pose.position.z)),
        ros::Time(0), marker->header.frame_id);

        listener.transformPose("/map", poseMarker, poseMarkerMap);
    }

    xFace = poseMarkerMap.getOrigin().x();
    yFace = poseMarkerMap.getOrigin().y();
    zFace = poseMarkerMap.getOrigin().z();

    float xTarget,yTarget;

    xTarget=xFace-xRobot;
    yTarget=yFace-yRobot;
    float dolzina = sqrt(pow(xTarget,2)+pow(yTarget,2));

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
    goal.target_pose.pose.position.x = xTarget;
    goal.target_pose.pose.position.y = yTarget;
/*
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
*/
   /* MoveBaseClient ac("move_base", true);
    //wait for the action server to come up
    while(!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("Waiting for the move_base action server to come up");
    }

    ROS_INFO("Sending goal");
    ac.sendGoal(goal);

    ac.waitForResult(ros::Duration(5.0));*/
    

}
void callbackIdSearch(const std_msgs::String::ConstPtr& msg){
    string array[3];
    int i = 0;
    stringstream strings(msg->data.c_str());
    while (strings.good() && i<3) {
        strings >> array[i];
        i++;
    }
    //printf("%s %s\n",array[0].c_str(),array[1].c_str());
    if(cmp(array[0],"hotel")){
        iskanHotelID = atoi(array[1].c_str());
        iskanaOsebaID = atoi(array[2].c_str());
        isciHotel=true;
        oseba = false;
        //printf("iscem hotel\n");
    }else if(cmp(array[0],"oseba")){
        iskanaOsebaID = atoi(array[1].c_str());
        isciOsebo=true;
        oseba = true;
        //printf("iscem osebo\n");
    }
}

void callbackTalk (const std_msgs::String::ConstPtr& msg1) {
    //printf("TAXI: dobil sem msg\n");
    printf("Dobil sem %s\n",msg1->data.c_str());
    if (!strcmp(msg1->data.c_str(),"pathEnded") && found){

        MoveBaseClient ac("move_base", true);
        //wait for the action server to come up
        while(!ac.waitForServer(ros::Duration(5.0))){
            ROS_INFO("Waiting for the move_base action server to come up");
        }

        ROS_INFO("Sending goal");
        ac.sendGoal(goal);

        ac.waitForResult(ros::Duration(5.0));
        if(oseba){
            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
                //printf("Searching for%d\n",iskanaOsebaID);
                printf("ROBOT: '%s vstopi v taxi.' \n", osebe[iskanaOsebaID-1].c_str());
                std::stringstream ss2;
                ss2 << "rosrun sound_play say.py \"Hello " << osebe[iskanaOsebaID-1].c_str() << " come on in it's cold outside\"";
                int i = std::system(ss2.str().c_str());
               // int i = system("rosrun sound_play say.py \"Hello "+trenutnaOseba.c_str()+"\"");
                iskanaOsebaID = 0;
                isciOsebo = false;
                sleep(2);
            } else {
                ROS_INFO("Goal unreachable: %s\n",ac.getState().toString().c_str());
                printf("ROBOT: '%s vstopi v taxi.' \n", osebe[iskanaOsebaID-1].c_str());
                std::stringstream ss2;
                ss2 << "rosrun sound_play say.py \"Hello " << osebe[iskanaOsebaID-1].c_str() << " come on in it's cold outside\"";
                int i = std::system(ss2.str().c_str());
               // int i = system("rosrun sound_play say.py \"Hello "+trenutnaOseba.c_str()+"\"");
                iskanaOsebaID = 0;
                isciOsebo = false;
            }
        }else{
            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
                    printf("ROBOT: 'Sayonara %s.' \n", osebe[iskanaOsebaID-1].c_str());
                    std::stringstream ss3;
                    ss3 << "rosrun sound_play say.py \"Sayonara " << osebe[iskanaOsebaID-1].c_str() << "\"";
                    int i = std::system(ss3.str().c_str());
                   // int i = system("rosrun sound_play say.py \"Sayonara "+trenutnaOseba.c_str()+"\"");
                    iskanHotelID = 0;
                    isciHotel = false;

                    ros::spinOnce();
                    sleep(2);
                } else {
                    ROS_INFO("Goal unreachable: %s\n",ac.getState().toString().c_str());
                    printf("ROBOT: 'Sayonara %s.\n", osebe[iskanaOsebaID-1].c_str());
                    std::stringstream ss3;
                    ss3 << "rosrun sound_play say.py \"Sayonara " << osebe[iskanaOsebaID-1].c_str() << ".\"";
                    int i = std::system(ss3.str().c_str());
                }
                foundHotel=false;
                std_msgs::String msg;
                std::stringstream ss;
                ss << osebe[iskanaOsebaID-1];
                msg.data = ss.str();
                updateTaxi.publish(msg);

        }
        found = false;
        iskanHotelID = 0;
        isciHotel = false;
        iskanaOsebaID = 0;
        isciOsebo = false;
    }
}

int main(int argc, char** argv){
    ros::init(argc, argv, "approach");
    ros::NodeHandle nh,nh2,nh3,nh4,nh5;


    ros::Subscriber sub = nh.subscribe<visualization_msgs::MarkerArray> ("/foundFace", 1, callbackFoundFace);
    ros::Subscriber sub2 = nh2.subscribe<visualization_msgs::Marker> ("/hotel", 1, callbackHotel);
    ros::Subscriber sub3 = nh3.subscribe<std_msgs::String>("/idSearch", 1,callbackIdSearch);
    ros::Subscriber subTalk = nh4.subscribe<std_msgs::String>("/tournament3/talk", 1, callbackTalk);
    updateTaxi = nh5.advertise<std_msgs::String>("/person", 1);

    pathSearch = nh2.advertise<std_msgs::String>("/tournament3/search", 1);
    //ros::spin();
    ros::Rate r(10);
    while (ros::ok()){
      ros::spinOnce();               
      r.sleep();
    }

  	//return 0;
}
