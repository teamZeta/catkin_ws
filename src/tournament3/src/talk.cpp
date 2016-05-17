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
#include <iostream>
#include <string>

using namespace std;
using namespace boost;

// mode 0 - who to pick / where to drive
// mode 1 - confirm correct order
static bool newOrder = true;

// podatki o potnikih v taxiju
static std::string person1 = "";
static std::string street1 = "";
static std::string building1 = "";
static int sex1 = 0;			// 0 - undefined, 1 - male, 2 - female
static std::string person2 = "";
static std::string street2 = "";
static std::string building2 = "";
static int sex2 = 0;
static int nPerson = 0;


void callback (const std_msgs::String::ConstPtr& msg) {
	ROS_INFO("I heard: [%s]", msg->data.c_str());
	//std::cout<<msg->data.c_str()<<std::endl;

	if (newOrder) {

		string array[5];
		int i = 0;
		stringstream order(msg->data.c_str());
		while (order.good() && i<5) {
			order >> array[i];
			i++;
		}
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POBERI OSEBO ::::::::::::::::
		// ::::::::::::::::::::::::::::::::::::::::::::::

		if (!array[0].compare("Pick")) {
			if ((!array[1].compare("Peter") || !array[1].compare("Tina") || !array[1].compare("Kim") || 
				!array[1].compare("Harry") || !array[1].compare("Matthew") || !array[1].compare("Scarlet") || 
				!array[1].compare("Ellen") || !array[1].compare("Filip") || !array[1].compare("Forrest")) && 
				(!array[3].compare("Red") || !array[3].compare("Green") || !array[3].compare("Blue") || 
				!array[3].compare("Yellow"))) {

				if (!person1.compare("") && !person2.compare("")) {							// Oba sedeza sta prazna
					person1 = array[1];
					street1 = array[3];
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Ellen")) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}

				} else if (!person1.compare("")) {											// Samo prvi sedez je prazen
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Ellen")) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}
					if (sex1 == sex2) {
						sex1 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");
					} else {
						nPerson = 1;
						person1 = array[1];
						street1 = array[3];
						newOrder = false;
						printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());	
					}

				} else if (!person2.compare("")) {											// Samo drugi sedez je prazen
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Ellen")) {
						sex2 = 2;
					} else {
						sex2 = 1;
					}
					if (sex1 == sex2) {
						sex2 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");						
					} else {
						nPerson = 2;
						person2 = array[1];
						street2 = array[3];
						newOrder = false;
						printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());	
					}

				} else {																	// Oba sedeza sta zasedena
					printf("ROBOT: 'There is no space left in the taxi.'\n");
				}
			}
			printf("ROBOT: 'I didn't understand the order. Pls repeat it.'\n");

		// :::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: ODPELJI OSEBO ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::::
			
		} else if (!array[0].compare("Take")) {
			if (!person1.compare(""))
			if (!array[4].compare("Red") || !array[4].compare("Green") || !array[4].compare("Blue") || 
				!array[4].compare("Yellow")) {

				
				newOrder = false;
				printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());
			}
			printf("ROBOT: 'I didn't understand the order. Pls repeat it.'\n");
		} else {
			printf("ROBOT: 'I didn't understand the order. Pls repeat it.'\n");
		}

		// :::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POTRDI UKAZ ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::

	} else {
	/*
		if (!array[1].compare("Peter")) {
			printf("%s\n", array[1].c_str());
		}*/
	}

}

int main(int argc, char** argv){
  	ros::init(argc, argv, "talk");
	ros::NodeHandle nh;
	ros::Subscriber sub = nh.subscribe<std_msgs::String>("/command", 1, callback);
	ros::spin();
}

// "Pick Peter on Blue Street"
// "Take Peter to the Blue Building"
// "Yes" / "No"