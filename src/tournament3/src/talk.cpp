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
#include <unistd.h>
#include <stdlib.h> 

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
static int osebeDimensions = 6;
static string osebe[][6] = {{"harry", "hairy", "perry","harriet","Ferry","Carrie"},
							{"philip", "philips", "phillip","philip","philip","philip"},
							{"tina", "pina", "dina","dana","tina","tina"},
							{"peter", "peter", "peter","peter","peter","peter"},
							{"tom", "home", "dome","stone","tome","phone"},
							{"ellen", "alen", "allen","erin","aaron","ellen"},
							{"kim", "kim", "kim","kim","kim","kim"},
							{"scarlet", "scarlett", "scarlets","scotland","scarlet","scarlet"},
							{"matthew", "matthew", "matthew","matthew","matthew","matthew"}};

static int barveDimensions = 9;
static string barve[][9] = {{"red", "rat", "rad", "read", "right","bread","rath","road","red"},
							{"green", "grill","grant","green","green","green","green","green","green"},
							{"blue", "blow", "blah", "block", "blood", "bloor","bloodstream","bluff","blue"},
							{"yellow", "narrow", "yellow", "yellow", "yellow", "narrow","yellow","yellow","yellow"}};
static ros::Publisher setGoal;

bool cmp(string s1, string s2) {
    int i;
    for (i=0; s1[i]; i++)
    	s1[i] = tolower(s1[i]);
    for (i=0; s1[i]; i++)
    	s2[i] = tolower(s2[i]);
    return !s1.compare(s2);
}

static bool color(std::string barva) {
	for (int i=0; i<4; i++) {
		for (int j=0; j<barveDimensions; j++) {
			if (cmp(barva, barve[i][j])) {
			    return true;
			}
		}
	}
	return false;
}

static int colorIndex(std::string barva) {
	for (int i=0; i<4; i++) {
		for (int j=0; j<barveDimensions; j++) {
			if (cmp(barva, barve[i][j])) {
			    return i;
			}
		}
	}
	return -1;
}

static bool person(std::string trenutnaOseba) {
	for (int i=0; i<9; i++) {
		for (int j=0; j<osebeDimensions; j++) {
			if (cmp(trenutnaOseba, osebe[i][j])) {
			    return true;
			}
		}
	}
	return false;
}

static int personIndex(std::string trenutnaOseba) {
	for (int i=0; i<9; i++) {
		for (int j=0; j<osebeDimensions; j++) {
			if (cmp(trenutnaOseba, osebe[i][j])) {
			    return i;
			}
		}
	}
	return -1;
}

static bool female(std::string trenutnaOseba) {
	if (personIndex(trenutnaOseba) == 2 || personIndex(trenutnaOseba) == 5 || personIndex(trenutnaOseba) == 7)
		return true;
	return false;
}

static bool personTake(std::string oseb, std::string trenutnaOseba) {
	int st = -1;
	for (int i=0; i<9; i++) {
		for (int j=0; j<osebeDimensions; j++) {
			if (cmp(trenutnaOseba, osebe[i][j])) {
			    st = i;
			    break;
			}
		}
		if (st != -1)
			break;
	}
	if (cmp(oseb, osebe[st][0]))
		return true;
	return false;
}

// ::::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::::: CALLBACK ::::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::::
void callback (const std_msgs::String::ConstPtr& msg) {
	ROS_INFO("I heard: [%s]", msg->data.c_str());
	//std::cout<<msg->data.c_str()<<std::endl;

	string array[6];
	int i = 0;
	stringstream strings(msg->data.c_str());
	while (strings.good() && i<6) {
		strings >> array[i];
		i++;
	}
	if (!array[0].compare("details")) {
		printf("Sedez 1: %s, pobrana na: %s, namenjena na: %s\n", person1.c_str(), street1.c_str(), building1.c_str());
		printf("Sedez 2: %s, pobrana na: %s, namenjena na: %s\n", person2.c_str(), street2.c_str(), building2.c_str());
	}

	if (newOrder) {

		// ::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POBERI OSEBO ::::::::::::::::
		// ::::::::::::::::::::::::::::::::::::::::::::::

		if (!array[0].compare("find")) {
			if (person(array[1]) && color(array[3])) {

				if (!person1.compare("") && !person2.compare("")) {							// Oba sedeza sta prazna
					person1 = osebe[personIndex(array[1])][0];
					street1 = barve[colorIndex(array[3])][0];
					if (female(array[1])) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}
					nPerson = 1;
					newOrder = false;
					order = 1;
					printf("ROBOT: 'Should I pick %s on the %s Street?'\n", person1.c_str(), street1.c_str());
					std::stringstream ss;
					ss << "rosrun sound_play say.py \"Should I pick " << person1.c_str() << " on the " << street1.c_str() <<  " Street?\"";
					int i = std::system(ss.str().c_str());
					//int i = std::system(std::string("rosrun sound_play say.py \"Should i pick ")+std::string(person1.c_str())+std::string(" on the ")+std::string(street1.c_str())+std::string(" Street?\""));
					//auto x = ss.str();
				} else if (!person1.compare("")) {										// Samo prvi sedez je prazen
					if (female(array[1])) {
						sex1 = 2;
					} else {
						sex1 = 1;
					}
					// Preveri ce sta razlicen spol
					if (sex1 == sex2) {
						sex1 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");
						int i = std::system("rosrun sound_play say.py \"This is a no gays allowed taxi\"");
					} else {
						nPerson = 1;
						person1 = osebe[personIndex(array[1])][0];
						street1 = barve[colorIndex(array[3])][0];
						newOrder = false;
						order = 1;
						printf("ROBOT: 'Should I pick %s on the %s Street?'\n",person1.c_str(), street1.c_str());
						std::stringstream ss;
						ss << "rosrun sound_play say.py \"Should I pick " << person1.c_str() << " on the " << street1.c_str() <<  " Street?\"";
						int i = std::system(ss.str().c_str());
						//int i = system("rosrun sound_play say.py \"Should i pick "+person1.c_str()+" on the "+street1.c_str()+" Street?\"");
					}

				} else if (!person2.compare("")) {											// Samo drugi sedez je prazen
					if (female(array[1])) {
						sex2 = 2;
					} else {
						sex2 = 1;
					}
					// Preveri ce sta razlicen spol
					if (sex1 == sex2) {
						sex2 = 0;
						printf("ROBOT: 'This is a no gays allowed taxi. Can't take two of the same sex.'\n");
						int i = std::system("rosrun sound_play say.py \"This is a no gays allowed taxi\"");						
					} else {
						nPerson = 2;
						person2 = osebe[personIndex(array[1])][0];
						street2 = barve[colorIndex(array[3])][0];
						newOrder = false;
						order = 1;
						printf("ROBOT: 'Should I pick %s on the %s Street?'\n", person2.c_str(), street2.c_str());
						std::stringstream ss;
						ss << "rosrun sound_play say.py \"Should I pick " << person2.c_str() << " on the " << street2.c_str() <<  " street?\"";
						int i = std::system(ss.str().c_str());
						//int i = system("rosrun sound_play say.py \"Should i pick "+person2.c_str()+" on the "+street2.c_str()+" Street?\"");	
					}

				} else {																	// Oba sedeza sta zasedena
					printf("ROBOT: 'There is no space left in the taxi.'\n");
					//int i = system("rosrun sound_play say.py \"There is no space left in the taxi.\"");
				}
			} else {
				printf("ROBOT: 'I didn't understand the order. Please repeat it.'\n");
				//int i = system("rosrun sound_play say.py \"I didn't understand the order. Please repeat it.\"");
			}

		// :::::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: ODPELJI OSEBO ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::::
			
		} else if (!array[0].compare("take") && color(array[3])) {
			if (personTake(person1,array[1])) {
				nPerson = 1;
				building1 = barve[colorIndex(array[3])][0];;
				newOrder = false;
				order = 2;
				printf("ROBOT: 'Should I take %s to the %s Building?'\n", person1.c_str(), building1.c_str());
				std::stringstream ss;
				ss << "rosrun sound_play say.py \"Should I take " << person1.c_str() << " to the " << building1.c_str() <<  " building?\"";
				int i = std::system(ss.str().c_str());
				//int i = system("rosrun sound_play say.py \"Should i take "+person1.c_str()+" to the "+building1.c_str()+" Building?\"");
			} else if (personTake(person2,array[1])) {
				nPerson = 2;
				building2 = barve[colorIndex(array[3])][0];;
				newOrder = false;
				order = 2;
				printf("ROBOT: 'Should I take %s to the %s Building?'\n", person2.c_str(), building2.c_str());
				std::stringstream ss;
				ss << "rosrun sound_play say.py \"Should I take " << person2.c_str() << " to the " << building2.c_str() <<  " building?\"";
				int i = std::system(ss.str().c_str());
				//int i = system("rosrun sound_play say.py \"Should i take "+person1.c_str()+" to the "+building1.c_str()+" Building?\"");
			} else {
				printf("ROBOT: 'No person with such name in the taxi.'\n");
				int i = std::system("rosrun sound_play say.py \"No person with such name in the taxi.\"");
			}
		} else {
			printf("ROBOT: 'I didn't understand the order. Please repeat it.'\n");
		}

		// :::::::::::::::::::::::::::::::::::::::::::::
		// :::::::::::::::: POTRDI UKAZ ::::::::::::::::
		// :::::::::::::::::::::::::::::::::::::::::::::

	} else {

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
					//ros::Rate loop_rate(10);
					//while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "pick " << person2 << " " << street2;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    //	ros::spinOnce();

				    //	loop_rate.sleep();
				  	//}
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
					//ros::Rate loop_rate(10);
					//while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "take " << person1 << " " << building1;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    //	ros::spinOnce();

				    //	loop_rate.sleep();
				  	//}

				} else if (nPerson == 2) {
					nPerson = 0;
					//ros::Rate loop_rate(10);
					//while (ros::ok()) {
						//printf("posiljam stop\n");
				  		std_msgs::String msg;
				    	std::stringstream ss;
				    	ss << "take " << person2 << " " << building2;
				    	msg.data = ss.str();
				    	setGoal.publish(msg);
				    	
				    //	ros::spinOnce();

				    //	loop_rate.sleep();
				  	//}
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
	}
}

// ::::::::::::::::::::::::::::::::::::::::::::::
// ::::::::::: CALLBACK UPDATE PEOPLE :::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::::
void callbackUpdatePeople (const std_msgs::String::ConstPtr& msg) {

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

// ::::::::::::::::::::::::::::::::::::::::::::::
// :::::::::::::::::::: MAIN ::::::::::::::::::::
// ::::::::::::::::::::::::::::::::::::::::::::::
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
// "Take Peter to Blue Building"
// "Details"
// "Yes" / "No"
