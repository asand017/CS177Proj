// Simulation of elevators in a building

#include <iostream>
#include "cpp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cmath>
#include <time.h>
#include <vector>
using namespace std;

#define NUM_SEATS 6      // number of seats available on shuttle
#define TINY 1.e-20      // a very small time period

//FINAL PROJECT Values
#define TIME_OPEN 2 //time to open elevator doors
#define TIME_CLOSE 3 //time to close elevator doors

long int LOBBY = 0;

mailbox service1("elevator 1"); // mailbox for elevator 1 - each floor sends their floor num
mailbox service2("elveator 2"); // mailbox for elevator 2 - each floor sends their floor num

//facility_set *buttons;

vector<facility> buttons;

buttons.push_back

facility idle1 ("idle elevator 1");           // dummy facility indicating an idle elevator
facility idle2 ("idle elevator 2");

event_set *get_off_now; //("get_off_now");  // all customers can get off shuttle

//event_set hop_on("board shuttle", 2);  // invite one customer to board at this stop
event_set *hop_on;
event boarded ("boarded");             // one customer responds after taking a seat

event_set *elevator_called;

void make_passengers(long whereami);       // passenger generator
vector<string> floors = {"L", "f1" , "f2", "f3", "f4", "f5", "f6", "f7", "f8"};
long group_size();

void passenger(long whoami);  // passenger trajectory
vector<string> people = {"coming", "going"}; // who's entering the elevator (coming) who's leaving (going) 

void elevator();                  // trajectory of the shuttle bus consists of...
void loop_around_airport(long & seats_used);      // ... repeated trips around airport
void load_elevator(long whereami, long & on_board); // posssibly loading passengers
qtable elevator_occ("elevator occupancy");  // time average of how full is the elevator

extern "C" void sim(int argc, char *argv[])      // main process
{
	  
      create("sim");
      elevator_occ.add_histogram(NUM_SEATS+1,0,NUM_SEATS);
      for(unsigned int i = 0; i < floors.size(); i++){
          make_passengers(i);  // generate a stream of arriving customers
      }
      elevator();              // create a single elevator
      hold (1440);             // wait for a whole day (in minutes) to pass
      report();
      //status_facilities();
  
}

// Model segment 1: generate groups of new passengers at specified location

void make_passengers(long whereami)
{
  cout << "new passenger made on floor " << whereami << endl;
  const char* myName=floors[whereami].c_str(); // hack because CSIM wants a char*
  create(myName);

  while(clock < 1440)          // run for one day (in minutes)
  {
    hold(expntl(10));           // exponential interarrivals, mean 10 minutes
    long group = group_size();
    for (long i=0;i<group;i++)  // create each member of the group
    {
	    passenger(whereami);      // new passenger appears at this location
	}
  }

}

// Model segment 2: activities followed by an individual passenger

void passenger(long whoami)
{
  const char* myName=people[whoami].c_str(); // hack because CSIM wants a char*
  create(myName);

  //(*buttons) [whoami].reserve();     // join the queue at my starting location
  (*elevator_called) [whoami].set();  // head of queue, so call shuttle
  (*hop_on) [whoami].queue();        // wait for shuttle and invitation to board
  (*elevator_called) [whoami].clear();// cancel my call; next in line will push 
  hold(uniform(0.5,1.0));        // takes time to get seated
  boarded.set();                 // tell driver you are in your seat
  //(*buttons) [whoami].release();     // let next person (if any) access button
  get_off_now->wait_any();            // everybody off when shuttle reaches next stop
}

// Elevator Process

void elevator() {
  create ("elevator");
  while(1) {  // loop forever
    idle1.reserve();                   // relax at Lobby till called from somewhere
	//cout << "test" << endl;	
	
    /*long who_pushed = elevator_called->wait_any();
    (*elevator_called) [who_pushed].set(); // loop exit needs to see event
    idle.release();                   // and back to work we go!

    long seats_used = 0;              // shuttle is initially empty
    elevator_occ.note_value(seats_used);

    hold(5);  // 5 minutes to reach car lot stop

    for(unsigned int i = 0; i < (*elevator_called).num_events(); i++){ //loop through every terminal + car lot
      if((*elevator_called) [i].state()==OCC){
        loop_around_airport(seats_used);
      }*/
	idle1.release();
  }
  
}

long group_size() {  // calculates the number of passengers in a group
  double x = prob();
  if (x < 0.3) return 1;
  else {
    if (x < 0.7) return 2;
    else return 5;
  }
}

void loop_around_airport(long & seats_used) { // one trip around the airport
  // Start by picking up departing passengers at car lot
  load_elevator(LOBBY, seats_used);
  elevator_occ.note_value(seats_used);

  /*for(unsigned int i = 0; i < floors.size(); i++){

      if(i == 1 || (*buttons) [i].name() == "Curb[1]"){
	  hold (uniform(3,5));
          continue;
      }

      hold (uniform(3,5));  // drive to airport terminal

  // drop off all departing passengers at airport terminal
      if(seats_used > 0 && (*buttons) [i].name() == ("Curb[" + to_string(i) + "]")) {
          (*get_off_now) [i].set(); // open door and let them off
          seats_used = 0; //seats_used - (*get_off_now) [i].wait_cnt();
          elevator_occ.note_value(seats_used);
      }

      if(seats_used == 0){


  // pick up arriving passengers at airport terminal
         load_elevator(i, seats_used);
         elevator_occ.note_value(seats_used);
      }
  }
  
  hold (uniform(3,5));  // drive to Hertz car lot
  hold (uniform(3,5));  
  // drop off all arriving passengers at car lot
  if(seats_used > 0) {
    (*get_off_now) [1].set(); // open door and let them off
    seats_used = 0;
    elevator_occ.note_value(seats_used);
  }*/
  // Back to starting point. Bus is empty. Maybe I can rest...
}

void load_elevator(long whereami, long & on_board)  // manage passenger loading
{
  // invite passengers to enter, one at a time, until all seats are full
  //while((on_board < NUM_SEATS) &&
    //((*buttons) [whereami].num_busy() + (*buttons) [whereami].qlength() > 0))
  //{
    (*hop_on) [whereami].set();// invite one person to board
    boarded.wait();  // pause until that person is seated
    on_board++;
    hold(TINY);  // let next passenger (if any) reset the button
  //}
}
