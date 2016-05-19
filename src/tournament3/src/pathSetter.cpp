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

static move_base_msgs::MoveBaseGoal goals[4];
static move_base_msgs::MoveBaseGoal searchingGoal[16];
static move_base_msgs::MoveBaseGoal goal;
static int currentGoal = -1;
static bool foundFace = false;
static ros::Publisher updateTaxi;
static string osebe[] = {"Peter", "Tina", "Kim", "Harry", "Matthew", "Scarlet", "Ellen", "Filip", "Forrest"};
static int iskanaOsebaID = 0;
static string trenutnaOseba = "";
/*
    OSEBE ID:
    "Peter"     1
    "Tina"      2
    "Kim"       3
    "Harry"     4
    "Matthew"   5
    "Scarlet"   6
    "Ellen"     7
    "Filip"     8
    "Forrest"   9
*/

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

static move_base_msgs::MoveBaseGoal createGoal(float xRobot, float yRobot, float xDirection, float yDirection){
    move_base_msgs::MoveBaseGoal goal;
    goal.target_pose.header.frame_id = "/map";
    tf::Vector3 v1 = tf::Vector3(1, 0 ,0);
    tf::Vector3 v2 = tf::Vector3(xDirection+0.0000001, yDirection+0.0000001,0);
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

// ::::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::::: CALLBACK ::::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::::
void callback (const std_msgs::String::ConstPtr& msg) {
    string array[3];
    int i = 0;
    stringstream strings(msg->data.c_str());
    while (strings.good() && i<3) {
        strings >> array[i];
        i++;
    }

    // ::::::::::::::::::::::
    // doloci katero osebo iscemo
    trenutnaOseba = array[1];
    if (!array[0].compare("pick")) {
        for (int i=0; i<9; i++) {
            if (!trenutnaOseba.compare(osebe[i])) {
                iskanaOsebaID = i+1;
                break;
            }
        }
    }

    // ::::::::::::::::::::::
    // doloci kam mora it
    int searchStart = 0;
    int searchEnd = 0;
    if (!array[2].compare("red")) {
        currentGoal = 0;
        searchStart = 0;
        searchEnd = 3;
    } else if (!array[2].compare("green")) {
        currentGoal = 1;
        searchStart = 4;
        searchEnd = 7;
    } else if (!array[2].compare("blue")) {
        currentGoal = 2;
        searchStart = 8;
        searchEnd = 11;
    } else if (!array[2].compare("yellow")) {
        currentGoal = 3;
        searchStart = 12;
        searchEnd = 15;
    } else {
        printf("pathSetter shouldnt come here (Doloci barvo ulice).\n");
    }

    // ::::::::::::::::::::::
    // nastavi goal
    MoveBaseClient ac("move_base", true);
    while(!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("Waiting for the move_base action server to come up");
    }
    //printf("Goal: %f %f\n",xTarget,yTarget);
    //ROS_INFO("Sending path goal %d",nGoal);
    ac.sendGoal(goals[currentGoal]);
    ac.waitForResult();
    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
        ROS_INFO("Reached goal.");
       // sleep(1.1);
    } else {
        printf("No go %s\n",ac.getState().toString().c_str());
        // mogoce rabi while, ki bo setal goal dokler mu ne uspe prit do tam
    }

    // ::::::::::::::::::::::
    // zacni izvajat iskanje prave osebe
    if (!array[0].compare("pick") && searchStart<=searchEnd) {
        while (!foundFace) {
            // dokler ne najde face se premikaj po goalih
            MoveBaseClient ac("move_base", true);
            while(!ac.waitForServer(ros::Duration(5.0))){
                ROS_INFO("Waiting for the move_base action server to come up");
            }
            //printf("Goal: %f %f\n",xTarget,yTarget);
            //ROS_INFO("Sending path goal %d",nGoal);
            ac.sendGoal(searchingGoal[searchStart]);
            ac.waitForResult();
            if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
                ROS_INFO("Reached goal.");
               // sleep(1.1);
            } else {
                printf("No go %s\n",ac.getState().toString().c_str());
                // mogoce rabi while, ki bo setal goal dokler mu ne uspe prit do tam
            }
            searchStart++;

        }
        // ko najde faco izracunaj vektor in se priblizaj
        foundFace = false;
    }
    // ::::::::::::::::::::::
    // zacni izvajat iskanje prave zgradbe
    else if (!array[0].compare("take")) {



        // ko najde zgradbo, vrzi osebo iz avta
        printf("ROBOT: 'Sayonara %s.'\n", array[1].c_str());
        ros::Rate loop_rate(10);
        while (ros::ok()) {
            printf("posiljam search\n");
            std_msgs::String msg;
            std::stringstream ss;
            ss << array[1];
            msg.data = ss.str();
            updateTaxi.publish(msg);
            
            ros::spinOnce();

            loop_rate.sleep();
        }

    } else {
        printf("pathSetter shouldnt come here (Take osebo).\n");
    }


}
// ::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::: FOUND FACE ::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::
void callbackFoundFace (const visualization_msgs::MarkerArrayConstPtr& markerArray) {

    if (iskanaOsebaID != markerArray->markers[0].id) {
        return;
    }

    foundFace = true;

   /* ros::NodeHandle node;
    ros::Publisher vis_pub = node.advertise<visualization_msgs::Marker>( "visualization_marker", 1 );
    */
    goal.target_pose.header.frame_id = "/map";
    goal.target_pose.header.stamp = ros::Time::now();
    tf::TransformListener listener;
    tf::StampedTransform transform;
    listener.waitForTransform("/map", "/base_link", ros::Time(0), ros::Duration(10) );
    listener.lookupTransform("/map", "/base_link", ros::Time(0), transform);
    
    float xRobot = transform.getOrigin().x();
    float yRobot = transform.getOrigin().y();
    float xFace = markerArray->markers[0].pose.position.x;
    float yFace = markerArray->markers[0].pose.position.y;
    float zFace = markerArray->markers[0].pose.position.z;

    tf::Stamped<tf::Pose> poseMarkerMap;
    try{
        tf::Stamped<tf::Pose> poseMarker(
        tf::Pose(tf::Quaternion(0, 0, 0, 1), tf::Vector3(xFace, 0, markerArray->markers[0].pose.position.z)),
        markerArray->markers[0].header.stamp, markerArray->markers[0].header.frame_id);

        listener.transformPose("/map", poseMarker, poseMarkerMap);
        printf("Time works\n");
    }catch(tf2::ExtrapolationException e){
        //printf("Error\n");

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
    MoveBaseClient ac("move_base", true);
    //wait for the action server to come up
    while(!ac.waitForServer(ros::Duration(5.0))){
        ROS_INFO("Waiting for the move_base action server to come up");
    }

    ROS_INFO("Sending goal");
    ac.sendGoal(goal);

    ac.waitForResult(ros::Duration(5.0));
    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
        printf("ROBOT: '%s vstopi v taxi.' \n", trenutnaOseba.c_str());
    } else {
        ROS_INFO("Goal unreachable: %s\n",ac.getState().toString().c_str());
    }

}

void goalInit() {
    int i=0;
    goals[i++]=createGoal(-0.2,0.2,0,-1);    // red street
    goals[i++]=createGoal(-0.2,0.2,-1,0);    // green street
    goals[i++]=createGoal(-0.2,0.2,-1,1);    // blue street
    goals[i++]=createGoal(-0.2,0.2,-1,1);    // yellow street
}

void searchingGoalInit() {
    int i=0;
    searchingGoal[i++]=createGoal(-0.2,0.2,0,-1);   // red street goals
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,0);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);

    searchingGoal[i++]=createGoal(-0.2,0.2,0,-1);   // green street goals
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,0);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);

    searchingGoal[i++]=createGoal(-0.2,0.2,0,-1);   // blue street goals
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,0);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);

    searchingGoal[i++]=createGoal(-0.2,0.2,0,-1);   // yellow street goals
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,0);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);
    searchingGoal[i++]=createGoal(-0.2,0.2,-1,1);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "pathSetter");
    ros::NodeHandle nh;
    ros::NodeHandle nh2;
    ros::NodeHandle nh3;

    goalInit();
    searchingGoalInit();

    ros::Subscriber sub = nh.subscribe<std_msgs::String>("/newGoal", 1, callback);
    ros::Subscriber sub2 = nh2.subscribe<visualization_msgs::MarkerArray> ("/foundFace", 1, callbackFoundFace);
    updateTaxi = nh3.advertise<std_msgs::String>("/person", 1);

    ros::spin();

  	//return 0;
}
