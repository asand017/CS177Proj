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

//MBOX buttons;
//buttons = mailbox ("buttons");
//facility idle ("idle");

//event_set going_up[2];
//event_set going_dn[2];
event_set get_off_now("get off now", 2); 
//event_set hop_on("hop on", 2);

event_set boarded("boarded", 2); 

//Wakes up elevators
event_set Wakeup("Wakeup", 2);

event_set Going_up("up", 9);
event_set Going_dn("down", 9);

int want_up[9];
int want_dn[9];
//int want_off[2][8];

vector<int> arr_elv = {0,0,0,0,0,0,0,0,0}; // 0 - no elevator coming, 1 - elevator 1 coming, 2 - elevator 2 coming
// 3 - both elevators coming 

struct Elevator{
	int current_floor; // current floor of the elevator
	vector<int> want_off[9]; // floors where passengers want to go
	int direction; // 0 - idle, 1 - up, 2 - down
	int next_stop; // next floor to go to

	void update(int curr, int next, int Direction){
		current_floor = curr;
		next_stop = next;
		direction = Direction;
	}

	void print(){
	    cout << "current floor: " << current_floor << endl;
		cout << "direction: " << direction << endl;
		cout << "next elevator stop: " << next_stop << endl;
	}
};	

void make_passengers(long whereami);

vector<string> floors = {"L", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8"};
long group_size();

void passenger(long whoami);  // passenger trajectory
vector<string> people = {"incoming", "outgoing", "interfloor"};

void Control();

void elevator(int elevatornum);
//void elevator2();
void loop_around_floors(int elevatornum, long & seats_used);

void load_elevator(int elevatornum, long whereami, long & on_board);

Elevator elevator_1;
Elevator elevator_2;

vector<Elevator> elevs = {elevator_1, elevator_2};

qtable elevator_occ("elevator occupancy");  // time average of how full is the elevator

extern "C" void sim(int argc, char *argv[])      // main process
{
	  
      create("sim");

      elevator_occ.add_histogram(NUM_SEATS+1,0,NUM_SEATS);
      
      for(int i = 0; i < floors.size(); i++){
        make_passengers(i);
      }

      passenger(0);
      passenger(3);

	  elevs[0].update(9, 9, 0); // elevator 1
	  elevs[1].update(0, 0, 0); // elevator 2

      elevator(1);             
      elevator(2);
	  hold (1440);             // wait for a whole day (in minutes) to pass
      report();
      //status_facilities();
  
}

// Model segment 1: generate groups of new passengers at specified location

void make_passengers(long whereami)
{
  
  const char* myName=floors[whereami].c_str();
  create(myName);

  

  while(clock < 1440)       
  {
    hold(expntl(10)); 
    long group = 10;//group_size();
    for (long i=0;i<group;i++) 
      passenger(whereami); 
  }

}

// Model segment 2: activities followed by an individual passenger

void passenger(long whoami)
{
  //whoami --> the current floor of the passenger
  
  
 
  // Give random destination floor

  int dest_floor = 0;
  
  //cout << want_up[0] << " " << want_dn[0] << endl;

  while(dest_floor == whoami){
     dest_floor = rand() % 9; // a random destination floor
  }
  
  string pass_type;
  if(whoami == 0) pass_type = people[0];
  else if(dest_floor == 0) pass_type = people[1];
  else pass_type = people[2];

  char floor_name = whoami + 48;
  string pass_name = pass_type + floor_name;

  const char* myName=pass_name.c_str();
  create(myName);
  

  //(*buttons) [whoami].reserve();     // join the queue at my starting location
  
  int pass_dir;

  //change array
  if(dest_floor > whoami){
    pass_dir = 1;
	want_up[whoami] += 1;
  }
  else if(dest_floor < whoami){
	pass_dir = 2;
    want_dn[whoami] += 1;
  }

  //send ( buttons, whoami ); // wake up an elevator

  //wait for announcemnt of which elevator is coming
  if(pass_dir ==1){
    Going_up[whoami].wait();
  }
  else if(pass_dir == 2){
    Going_dn[whoami].wait();
  }

  //elv_num is elevator that came to floor
  int elv_num = arr_elv[whoami];

  //while(elevs[elv_num].direction != pass_dir);
 
  if(dest_floor > whoami){
    want_up[whoami] -= 1;
  }
  else if(dest_floor < whoami){
    want_dn[whoami] -= 1;
  }
  

  hold(uniform(0.5,1.0));        // takes time to get seated
  //boarded.set();                 // tell driver you are in your seat
  //(*buttons) [whoami].release();     // let next person (if any) access button
  //get_off_now->wait_any();            // everybody off when shuttle reaches next stop*/
  
  
}

void Control(){
  	 //Find out first elevator to visit for maximum number of visits
     create("control");
  
    while(1){
      //long* whoami;
      //receive( buttons, whoami ); 
      int goto_floor; 
      bool up_true = false, dn_true = false;
      for(int i = 0; i < 9; i++){
         if(want_up[i] > 0 && arr_elv == 0) up_true = true;
         if(want_dn[i] > 0 && arr_elv == 0) dn_true = true;
      }
      
      
      int dn_visit = -1;
      int up_visit = 20; 
      
      if(up_true){
        for(int i = 0; i < 9; i++){
          if(i < up_visit) up_visit = i;
        }
        int min_up_visit = 100, min_elevator_dn = -1;
        for(int i = 0; i < elevs.size(); i++){
          int distance_up_visit = 101;
          if(elevs[i].direction == 0){
            distance_up_visit = abs(elevs[i].current_floor - up_visit);
            if(distance_up_visit < min_up_visit) {
             min_up_visit = distance_up_visit;
             min_elevator_up = i;
            }
          }
        }
      }
      
      else if(dn_true){
        for(int i = 0; i < 9; i++){
          if(i > dn_visit) dn_visit = i;
        }
        int min_dn_visit = 100, min_elevator_dn = -1;
        for(int i = 0; i < elevs.size(); i++){
          int distance_dn_visit = 101;
          
      }

      //Compare floor with maximum number of visits for up and down
      //Choose whichever is closest
      int min_dn_visit = 100, min_up_visit = 100, min_elevator_up = -1, min_elevator_dn = -1;
      for(int i = 0; i < elevs.size(); i++){
          int distance_up_visit = 101, distance_dn_visit = 101;
          if(elevs[i].direction == 0){
            if(up) distance_up_visit = abs(elevs[i].current_floor - up_visit);
            if(dn) distance_dn_visit = abs(elevs[i].current_floor - dn_visit);
            if(distance_up_visit < min_up_visit) {
             min_up_visit = distance_up_visit;
             min_elevator_up = i;
            }
            if(distance_dn_visit < min_dn_visit) {
              min_dn_visit = distance_dn_visit;
              min_elevator_dn = i;
            }
          }
       }
       
       
    }
}

// Elevator Process

void elevator(int elevatornum) {
  char num = elevatornum + 48;
  string elev_name = "elevator" + num;
  create (elev_name.c_str());

  while(1) {  // loop forever


    Wakeup[elevatornum].wait();
    
    //if both, make sure the same elevator isn't doing both up and down
    if(both)
    {
      if(min_elevator_up != elevs[elevatornum].current_floor) min_elevator_dn = elevatornum;
    }
    
    long seats_used = 0;
    
    if(min_elevator_up == elevatornum){  
      goto_floor = up_visit;
      elevs[elevatornum].next_stop = goto_floor;
      for(int i = goto_floor; i < 9; i++){
          if(want_up[i] > 0)
          {
             if(arr_elv[i] > 0) arr_elv[i] = 3;
             else arr_elv[i] = elevatornum;
          }
      }
      loop_around_floors(elevatornum, seats_used);
    }
    else if(min_elevator_dn == elevatornum){
      goto_floor = dn_visit;
      elevs[elevatornum].next_stop = goto_floor;
      for(int i =goto_floor; i >= 0; i--){
          if(want_dn[i] > 0)
          {
            if(arr_elv[i] > 0) arr_elv[i] = 3;
            else arr_elv[i] = elevatornum;
          }
      }
      loop_around_floors(elevatornum, seats_used);
    }

    else{
       
    }
    
    
    //long who_pushed = elevator_called->wait_any();
    //(*elevator_called) [who_pushed].set(); // loop exit needs to see event

    //long seats_used = 0;              // shuttle is initially empty
    //elevator_occ.note_value(seats_used);

    //hold(5);  // 5 minutes to reach car lot stop

    //for(unsigned int i = 0; i < (*elevator_called).num_events(); i++){ //loop through every terminal + car lot
    //  if((*elevator_called) [i].state()==OCC){
    //    loop_around_airport(seats_used);
    //  }
	//idle1.release();
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

void loop_around_floors (int elevatornum, long & seats_used) {

   
  // Start by picking up departing passengers at car lot
  //load_elevator(LOBBY, seats_used);
  //elevator_occ.note_value(seats_used);
  
  


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
  //while((on_board < NUM_SEATS) &&
    //((*buttons) [whereami].num_busy() + (*buttons) [whereami].qlength() > 0))
  //{
    //hop_on[whereami].set();// invite one person to board
    //boarded.wait();  // pause until that person is seated
    on_board++;
    hold(TINY);  // let next passenger (if any) reset the button
  //}
}
