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

facility_set *buttons;
facility idle ("idle");
event_set going_up[2];
event_set going_dn[2];
event_set *get_off_now; 
event_set *hop_on;
event boarded ("boarded"); 

event Wakeup("Wakeup");

int want_up[8];
int want_dn[8];
int want_off[2][8];

void make_passengers(long whereami);

vector<string> floors = {"L", "f2", "f3", "f4", "f5", "f6", "f7", "f8"};
long group_size();

void passenger(long whoami);  // passenger trajectory
vector<string> people = {"incoming", "outgoing", "interfloor"}
void elevator();
void loop_around_airport(long & seats_used);

void load_elevator(long whereami, long & on_board);

qtable elevator_occ("elevator occupancy");  // time average of how full is the elevator

extern "C" void sim(int argc, char *argv[])      // main process
{

      create("sim");
      elevator_occ.add_histogram(NUM_SEATS+1,0,NUM_SEATS);
      
      for(int i = 0; i < floors.size(); i++){
        make_passengers(i);
      }

      elevator();              // create a single elevator
      hold (1440);             // wait for a whole day (in minutes) to pass
      report();
      status_facilities();
  
}

// Model segment 1: generate groups of new passengers at specified location

void make_passengers(long whereami)
{
  const char* myName=floors[whereami].c_str(); // hack because CSIM wants a char*
  create(myName);

  while(clock < 1440.)          // run for one day (in minutes)
  {
    hold(expntl(10)); 
    long group = group_size();
    for (long i=0;i<group;i++) 
      passenger(whereami); 
  }
}

// Model segment 2: activities followed by an individual passenger

void passenger(long whoami)
{
  const char* myName=people[whoami].c_str(); // hack because CSIM wants a char*
  create(myName);

  // Give random destination floor

  int dest_floor = -1;

  while(dest_floor != whoami){
     dest_floor = rand() % 8;
  }

  //(*buttons) [whoami].reserve();     // join the queue at my starting location
  Wakeup.set();  // head of queue, so call shuttle
  
  //change array
  if(dest_floor > whoami){
    want_up[whoami]++;
  }
  else if(dest_floor < whoami){
    want_dn[whoami]++;
  }

  (*hop_on) [whoami].queue();        // wait for shuttle and invitation to board
  (*elevator_called) [whoami].clear();// cancel my call; next in line will push 
  
  if(dest_floor > whoami){
    want_up[whoami];
  }
  else if(dest_floor < whoami){
    want_dn[whoami];
  }
  //SET WANT OFF HERE


  hold(uniform(0.5,1.0));        // takes time to get seated
  boarded.set();                 // tell driver you are in your seat
  (*buttons) [whoami].release();     // let next person (if any) access button
  get_off_now->wait_any();            // everybody off when shuttle reaches next stop
}

// Elevator Process

void elevator() {
  create ("elevator");
  while(1) {  // loop forever
    Wakeup.wait();
    
    int dn_visit = -1;
    int up_visit = 9;
	  
    //Check array of requests, check for highest person wanting to go down and lowest person wanting to go up
    for(int = 0; i < 8; i++){
	     if(want_up[i] > 0 && want_up[i] < up_visit) up_visit = i;
	     if(want_dn[i] > 0 && want_dn[i] > dn_visit) dn_visit = i;
    }
    
    //TODO: CHECK IF OTHER ELEVATOR IS ON, GOING IN THAT DIRECTION ALREADY. IF NOTPICK WHICHEVER IS CLOSEST
    if(up_visit != 9 && dn_visit != -1){
       
    }
    
    
    
    
    //long who_pushed = elevator_called->wait_any();
    //(*elevator_called) [who_pushed].set(); // loop exit needs to see event

    long seats_used = 0;              // shuttle is initially empty
    elevator_occ.note_value(seats_used);

    hold(5);  // 5 minutes to reach car lot stop

    for(unsigned int i = 0; i < (*elevator_called).num_events(); i++){ //loop through every terminal + car lot
      if((*elevator_called) [i].state()==OCC){
        loop_around_airport(seats_used);
      }
    }
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

// HERE IMPLEMENT PASSENGER TERMINAL SETUP
void loop_around_airport(long & seats_used) { // one trip around the airport
  // Start by picking up departing passengers at car lot
  load_elevator(LOBBY, seats_used);
  elevator_occ.note_value(seats_used);

  for(unsigned int i = 0; i < floors.size(); i++){

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
  }
  // Back to starting point. Bus is empty. Maybe I can rest...
}

void load_elevator(long whereami, long & on_board)  // manage passenger loading
{
  // invite passengers to enter, one at a time, until all seats are full
  while((on_board < NUM_SEATS) &&
    ((*buttons) [whereami].num_busy() + (*buttons) [whereami].qlength() > 0))
  {
    (*hop_on) [whereami].set();// invite one person to board
    boarded.wait();  // pause until that person is seated
    on_board++;
    hold(TINY);  // let next passenger (if any) reset the button
  }
}
