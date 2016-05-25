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
static int order = 0;			// 0 - undefined, 1 - pick up, 2 - take to

static ros::Publisher setGoal;

// ::::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::::: CALLBACK ::::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::::
void callback (const std_msgs::String::ConstPtr& msg) {
	ROS_INFO("I heard: [%s]", msg->data.c_str());
	//std::cout<<msg->data.c_str()<<std::endl;

	if (newOrder) {

		string array[5];
		int i = 0;
		stringstream strings(msg->data.c_str());
		while (strings.good() && i<5) {
			strings >> array[i];
			i++;
		}
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POBERI OSEBO ::::::::::::::::
		// ::::::::::::::::::::::::::::::::::::::::::::::

		if (!array[0].compare("find")) {
			if ((!array[1].compare("Peter") || !array[1].compare("Tina") || !array[1].compare("Kim") || 
				!array[1].compare("Harry") || !array[1].compare("Matthew") || !array[1].compare("Scarlet") || !array[1].compare("Scarlett") || 
				!array[1].compare("Ellen") || !array[1].compare("Philip") || !array[1].compare("Tom")) && 
				(!array[3].compare("Red") || !array[3].compare("Green") || !array[3].compare("Blue") || !array[3].compare("red") || !array[3].compare("green") || !array[3].compare("blue") || 
				!array[3].compare("Yellow") || array[3].compare("yellow"))) {

				if (!person1.compare("") && !person2.compare("")) {							// Oba sedeza sta prazna
					person1 = array[1];
					street1 = array[3];
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Ellen")) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}
					nPerson = 1;
					newOrder = false;
					order = 1;
					printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());

				} else if (!person1.compare("")) {										// Samo prvi sedez je prazen
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Ellen")) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}
					// Preveri ce sta razlicen spol
					if (sex1 == sex2) {
						sex1 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");
					} else {
						nPerson = 1;
						person1 = array[1];
						street1 = array[3];
						newOrder = false;
						order = 1;
						printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());
					}

				} else if (!person2.compare("")) {											// Samo drugi sedez je prazen
					if (!array[1].compare("Tina") || !array[1].compare("Scarlet") || !array[1].compare("Scarlett") || !array[1].compare("Ellen")) {
						sex2 = 2;
					} else {
						sex2 = 1;
					}
					// Preveri ce sta razlicen spol
					if (sex1 == sex2) {
						sex2 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");						
					} else {
						nPerson = 2;
						person2 = array[1];
						street2 = array[3];
						newOrder = false;
						order = 1;
						printf("ROBOT: 'Should I pick %s on %s Street?'\n", array[1].c_str(), array[3].c_str());	
					}

				} else {																	// Oba sedeza sta zasedena
					printf("ROBOT: 'There is no space left in the taxi.'\n");
				}
			}

		// :::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: ODPELJI OSEBO ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::::
			
		} else if (!array[0].compare("take")) {
			if (!person1.compare(array[1])) {
				if (!array[4].compare("red") || !array[4].compare("green") || !array[4].compare("blue") || 
					!array[4].compare("yellow") || !array[4].compare("Red") || !array[4].compare("Green") || !array[4].compare("Blue") || 
					!array[4].compare("Yellow")) {

					nPerson = 1;
					building1 = array[4];
					newOrder = false;
					printf("ROBOT: 'Should I take %s to the %s Building?'\n", array[1].c_str(), array[4].c_str());
				}
			} else if (!person2.compare(array[1])) {
				if (!array[4].compare("red") || !array[4].compare("green") || !array[4].compare("blue") || 
					!array[4].compare("yellow") || !array[4].compare("Red") || !array[4].compare("Green") || !array[4].compare("Blue") || 
					!array[4].compare("Yellow")) {

					nPerson = 2;
					building2 = array[4];
					newOrder = false;
					printf("ROBOT: 'Should I take %s to the %s Building?'\n", array[1].c_str(), array[4].c_str());
				}
			} else {
				printf("No person with such name in taxi.\n");
			}
			printf("ROBOT: 'I didn't understand the order. Pls repeat it.'\n");
		} else {
			printf("ROBOT: 'I didn't understand the order. Pls repeat it.'\n");
		}

		// :::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POTRDI UKAZ ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::

	} else {

		string array[1];
		int i = 0;
		stringstream strings(msg->data.c_str());
		while (strings.good() && i<1) {
			strings >> array[i];
			i++;
		}

		if (order == 1) {															// poberi osebo na neki ulici
			if (!array[0].compare("yes")) {			// izvedi ukaz
				order = 0;
				newOrder = true;
				if (nPerson == 1) {
					nPerson = 0;
					//ros::Rate loop_rate(10);
					//while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "pick " << person1 << " " << street1;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    	//ros::spinOnce();

				    	//loop_rate.sleep();
				  //}
				} else if (nPerson == 2) {
					nPerson = 0;
					ros::Rate loop_rate(10);
					while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "pick" << person2 << " " << street2;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    	ros::spinOnce();

				    	loop_rate.sleep();
				  	}
				} else {
					printf("Program shouldn't come here. (Izvedi ukaz, nPerson != 1,2)\n");
				}

			} else if (!array[0].compare("no")) {	// izbrisi zapomnjeno
				order = 0;
				newOrder = true;
				if (nPerson == 1) {
					nPerson = 0;
					person1 = "";
					street1 = "";
					sex1 = 0;

				} else if (nPerson == 2) {
					nPerson = 0;
					person2 = "";
					street2 = "";
					sex2 = 0;

				} else {
					printf("Program shouldn't come here. (nPerson != 1,2)\n");
				}
			} else {
				printf("ROBOT: 'Please repeat the confirmation.\n'");
			}
		} else if (order == 2) {														// odpelji osebo do zgradbe
			if (!array[0].compare("yes")) {			// izvedi ukaz
				order = 0;
				newOrder = true;
				if (nPerson == 1) {
					nPerson = 0;
					ros::Rate loop_rate(10);
					while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "take " << person1 << " " << building1;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    	ros::spinOnce();

				    	loop_rate.sleep();
				  	}

				} else if (nPerson == 2) {
					nPerson = 0;
					ros::Rate loop_rate(10);
					while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "take " << person2 << " " << building2;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    	ros::spinOnce();

				    	loop_rate.sleep();
				  	}
				} else {
					printf("Program shouldn't come here. (Izvedi ukaz, nPerson != 1,2)\n");
				}

			} else if (!array[0].compare("no")) {	// izbrisi zapomnjeno
				order = 0;
				newOrder = true;
				if (nPerson == 1) {
					nPerson = 0;
					building1 = "";

				} else if (nPerson == 2) {
					nPerson = 0;
					building2 = "";

				} else {
					printf("Program shouldn't come here. (nPerson != 1,2)\n");
				}
			} else {
				printf("ROBOT: 'Please repeat the confirmation.\n'");
			}
		} else {
			printf("Program shouldn't come here. (order != 1,2) \n");
		}


	/*
		if (!array[1].compare("Peter")) {
			printf("%s\n", array[1].c_str());
		}*/
	}

}

void callbackUpdatePeople (const std_msgs::String::ConstPtr& msg) {
	//ROS_INFO("I heard: [%s]", msg->data.c_str());
	//std::cout<<msg->data.c_str()<<std::endl;

	string array[1];
	int i = 0;
	stringstream strings(msg->data.c_str());
	while (strings.good() && i<1) {
		strings >> array[i];
		i++;
	}

	if (!array[0].compare(person1)) {
		person1 = "";
		street1 = "";
		building1 = "";
		sex1 = 0;
	} else if (!array[0].compare(person2)) {
		person2 = "";
		street2 = "";
		building2 = "";
		sex2 = 0;
	} else {
		printf("callbackUpdatePeople shouldnt come here.\n");
	}
}

int main(int argc, char** argv){
  	ros::init(argc, argv, "talk");
	ros::NodeHandle nh;
	ros::NodeHandle nh2;
	ros::NodeHandle nh3;

	ros::Subscriber sub = nh.subscribe<std_msgs::String>("/command", 1, callback);
	ros::Subscriber sub2 = nh2.subscribe<std_msgs::String>("/person", 1, callbackUpdatePeople);
	setGoal = nh3.advertise<std_msgs::String>("/newGoal", 1);
	ros::spin();
}

// "Find Peter on Blue Street"
// "Take Peter to the Blue Building"
// "Yes" / "No"
