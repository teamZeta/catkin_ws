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

using namespace std;

static move_base_msgs::MoveBaseGoal redGoals[5];
static move_base_msgs::MoveBaseGoal greenGoals[9];
static move_base_msgs::MoveBaseGoal blueGoals[7];
static move_base_msgs::MoveBaseGoal yellowGoals[4];
static int redSize = 5;
static int greenSize = 9;
static int blueSize = 7;
static int yellowSize = 4;
static move_base_msgs::MoveBaseGoal goal;
static int currentGoal = -1;
static bool foundFace = false;
static ros::Publisher updateTaxi;
static string osebe[] = {"harry", "philip", "tina", "peter", "forrest", "ellen", "kim", "scarlet", "matthew"};
static string streetName[] = {"red","green","blue","yellow"};
static int iskanaOsebaID = 0;
static string trenutnaOseba = "";
static float diff=6.05;
static float dynamicDiff;
static ros::Publisher posit;
static bool reset = false;
static int goalInd=0;
static int whereTo = 0;     // 0 - undefined, 1234 - rgby
static bool once = false;


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

bool changeMap(float x, float y){
    printf("Racunam novo mapo\n");
    int mapInd=0;
    if(y>1.1)
        mapInd=1;
    else if(y<-3.2)
        mapInd=-1;
    int destInd=1;
    if(x<-2)
        destInd=-1;
	if(reset){
		destInd=0;
		reset=false;
	}

	dynamicDiff=(destInd-mapInd)*diff;
	printf("Koordinate %f , %f ",x,y);
	printf("Sem v: %d hocem v: %d teleport: %f",mapInd,destInd,dynamicDiff);
	return dynamicDiff!=0;
}

void callbackSign (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
	printf("vidim znak: %d\n", markerArray->markers[0].id);
    if (markerArray->markers[0].id == 3 || markerArray->markers[0].id == 1) {      // one way
    	printf("uh oh one way\n");
        once=true;
    }
}
void changeGoals(move_base_msgs::MoveBaseGoal Goals[],int size){
    for(int i=0;i<size;i++){
        Goals[i].target_pose.pose.position.y+=dynamicDiff;
    }
}

void callbackPose(const geometry_msgs::PoseWithCovarianceStamped msg){
    printf("c\n");
    if(once) {
        printf("...........................ONCE\n");
        if (changeMap((float)msg.pose.pose.position.x,(float)msg.pose.pose.position.y)) {
            printf("changeMap\n");
            geometry_msgs::PoseWithCovarianceStamped pos;
            pos.pose.pose.position.x = msg.pose.pose.position.x;
            pos.pose.pose.position.y = msg.pose.pose.position.y+dynamicDiff;
            pos.pose.pose.position.z = msg.pose.pose.position.z;
            pos.pose.pose.orientation.x = msg.pose.pose.orientation.x;
            pos.pose.pose.orientation.y = msg.pose.pose.orientation.y;
            pos.pose.pose.orientation.z = msg.pose.pose.orientation.z;
            pos.pose.pose.orientation.w = msg.pose.pose.orientation.w;

            changeGoals(redGoals,redSize);
            changeGoals(greenGoals,greenSize);
            changeGoals(blueGoals,blueSize);
            changeGoals(yellowGoals,yellowSize);

            posit.publish(pos);
            once = false;
        }
    }
}

static move_base_msgs::MoveBaseGoal createGoal(float xRobot, float yRobot, float xDirection, float yDirection){
    move_base_msgs::MoveBaseGoal goalC;
    goalC.target_pose.header.frame_id = "/map";
    tf::Vector3 v1 = tf::Vector3(1, 0 ,0);
    tf::Vector3 v2 = tf::Vector3(xDirection+0.0000001, yDirection+0.0000001,0);
    tf::Vector3 a = v1.cross(v2);
    tf::Quaternion q(a.x(), a.y(), a.z(), sqrt(v1.length2() * v2.length2()) + v1.dot(v2));
    q.normalize();
    goalC.target_pose.pose.orientation.x = q.x();
    goalC.target_pose.pose.orientation.y = q.y();
    goalC.target_pose.pose.orientation.z = q.z();
    goalC.target_pose.pose.orientation.w = q.w();
    goalC.target_pose.pose.position.x = xRobot;
    goalC.target_pose.pose.position.y = yRobot;
    return goalC;
}

void startSearch(move_base_msgs::MoveBaseGoal Goals[],int size){
    int k = 0;
    while (!foundFace && k<size) {
        MoveBaseClient ac("move_base", true);
        while(!ac.waitForServer(ros::Duration(5.0))){
            ROS_INFO("Waiting for the move_base action server to come up");
        }
        printf("Goal  %.2f - %.2f \n", Goals[k].target_pose.pose.position.x, Goals[k].target_pose.pose.position.y );
        ac.sendGoal(Goals[k]);
        ac.waitForResult();
        if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
            ROS_INFO("Reached goal.");
            sleep(1);
          //  reset=true;
        } else {
            printf("No go, retrying %s\n",ac.getState().toString().c_str());
            while(!ac.waitForServer(ros::Duration(5.0))){
                    ROS_INFO("Waiting for the move_base action server to come up");
                }
                printf("Goal  %.2f - %.2f \n", Goals[k].target_pose.pose.position.x, Goals[k].target_pose.pose.position.y );
                ac.sendGoal(Goals[k]);
                ac.waitForResult();
                if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
                    ROS_INFO("Reached goal.");
                    sleep(1);
                  //  reset=true;
                } else {
                    printf("No go %s\n",ac.getState().toString().c_str());

                }


        }
        k++;
        ros::spinOnce();
    }

}

bool cmp(string s1, string s2){
    int i;
    for (i=0; s1[i]; i++) s1[i] = tolower(s1[i]);
    for (i=0; s1[i]; i++) s2[i] = tolower(s2[i]);
    return !s1.compare(s2);
}


void callback (const std_msgs::String::ConstPtr& msg) {
    string array[3];
    int i = 0;
    stringstream strings(msg->data.c_str());
    while (strings.good() && i<3) {
        strings >> array[i];
        i++;
    }

    printf("Callback: %s, %s, %s\n", array[0].c_str(), array[1].c_str(), array[2].c_str());

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
    if (!array[0].compare("pick")) {
        for(int i=0;i<4;i++){
            if(cmp(array[2],streetName[i])){
                if (i == 0) {
                    startSearch(redGoals,redSize);
                } else if (i == 1) {
                    startSearch(greenGoals,greenSize);
                } else if (i == 2) {
                    startSearch(blueGoals,blueSize);
                } else if (i == 3) {
                    startSearch(yellowGoals,yellowSize);
                }
                // ko najde faco izracunaj vektor in se priblizaj
                foundFace = false;
            }
        }
    }
    //printf("Goal: %f %f\n",xTarget,yTarget);
    //ROS_INFO("Sending path goal %d",nGoal);


    // ::::::::::::::::::::::
    // zacni izvajat iskanje prave osebe

    // ::::::::::::::::::::::
    // zacni izvajat iskanje prave zgradbe
   else if (!array[0].compare("take")) {

        // TODO: iskanje zgradbe

        // ko najde zgradbo, vrzi osebo iz avta
        printf("ROBOT: 'Sayonara %s.'\n", array[1].c_str());
        //ros::Rate loop_rate(10);
        //while (ros::ok()) {
            //printf("posiljam search\n");
            std_msgs::String msg;
            std::stringstream ss;
            ss << array[1];
            msg.data = ss.str();
            updateTaxi.publish(msg);
            
            ros::spinOnce();

            //loop_rate.sleep();
        //}

    } else {
        printf("pathSetter shouldnt come here (Take osebo).\n");
    }


}




/*void callbackTeleport(const geometry_msgs::PoseWithCovarianceStamped msg){ //Zamenja goale na trenutno mapo
	printf("Callback za menjavo goalov\n");
    changeGoalMap((float)msg.pose.pose.position.x,(float)msg.pose.pose.position.y);
}*/
// ::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::: FOUND FACE ::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::
void callbackFoundFace (const visualization_msgs::MarkerArrayConstPtr& markerArray) {
    printf("iskanaOsebaID: %d, zaznan id face: %d\n", iskanaOsebaID, markerArray->markers[0].id);
    if (iskanaOsebaID != markerArray->markers[0].id) {
        return;
    }

    foundFace = true;
    printf("ROBOT: 'I found the person.'\n");

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
        iskanaOsebaID = 0;
        sleep(2);
    } else {
        ROS_INFO("Goal unreachable: %s\n",ac.getState().toString().c_str());
    }

}


void redGoalsInit() {
    int i=0;
    redGoals[i++]=createGoal(-3.5,2.3,1,0); 
    redGoals[i++]=createGoal(-3.5,2.3,0,1);
    redGoals[i++]=createGoal(-3.5,2.3,-1,0);
    redGoals[i++]=createGoal(-3.5,1.4,1,0);
    redGoals[i++]=createGoal(-3.5,1.4,-1,0);
}

void greenGoalsInit() {
    int i=0;
    greenGoals[i++]=createGoal(-2,-0.2,0,1);
    greenGoals[i++]=createGoal(-2,-0.2,0,-1);
    greenGoals[i++]=createGoal(-0.9,-0.25,0,1);
    greenGoals[i++]=createGoal(-0.4,-0.7,1,0);
    greenGoals[i++]=createGoal(0.17,-0.3,0,1);
    greenGoals[i++]=createGoal(0.7,-0.3,0,-1);
    greenGoals[i++]=createGoal(1.6,-0.3,0,1);
    greenGoals[i++]=createGoal(1.6,-0.3,1,0);
    greenGoals[i++]=createGoal(1.6,-0.3,-1,0);
}

void blueGoalsInit() {
    int i=0;
    blueGoals[i++]=createGoal(1.5,-1.4,1,0);    
    blueGoals[i++]=createGoal(1.5,-1.4,0,-1);  
    blueGoals[i++]=createGoal(0.7,-1.7,0,-1); 
    blueGoals[i++]=createGoal(0.7,-1.7,0,1);
    blueGoals[i++]=createGoal(-0.45,-1.4,0,-1); 
    blueGoals[i++]=createGoal(-1.9,-1.6,0,-1);
    blueGoals[i++]=createGoal(-1.9,-1.6,0,1);
}

void yellowGoalsInit() {
    int i=0;
    yellowGoals[i++]=createGoal(-3.3,-1.55,-1,0); 
    yellowGoals[i++]=createGoal(-3.3,-1.55,0,-1); 
    yellowGoals[i++]=createGoal(-3.4,-0.87,-1,0); 
    yellowGoals[i++]=createGoal(-3.4,-0.87,1,0);
}

int main(int argc, char** argv){
    ros::init(argc, argv, "pathSetter");
    ros::NodeHandle nh;
    ros::NodeHandle nh2;
    ros::NodeHandle nh3;
    ros::NodeHandle nh4;
    ros::NodeHandle nh5;
    ros::NodeHandle nh6;

    redGoalsInit();
    blueGoalsInit();
    greenGoalsInit();
    yellowGoalsInit();

    ros::Subscriber sub = nh.subscribe<std_msgs::String>("/newGoal", 1, callback);
    ros::Subscriber sub2 = nh2.subscribe<visualization_msgs::MarkerArray> ("/foundFace", 1, callbackFoundFace);
    updateTaxi = nh3.advertise<std_msgs::String>("/person", 1);
    posit = nh6.advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1);
    sleep(5);
    ros::Subscriber sub3 = nh4.subscribe<visualization_msgs::MarkerArray> ("/sign", 1, callbackSign);
    ros::Subscriber sub4 = nh5.subscribe<geometry_msgs::PoseWithCovarianceStamped> ("/amcl_pose", 1, callbackPose);

    //ros::spin();
    ros::Rate r(10);
    while (ros::ok()){
      ros::spinOnce();               
      r.sleep();
    }

  	//return 0;
}
