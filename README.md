# CS177Proj
Advanced Elevator Technology

ELEVATORS
-: Things we still need to do
+: Things we've already done

Elevator Call Buttons:
   - Change shuttle_called to logical arrays:
       - want_up
       - want_dn
       - 3D Array: want_off[e][f] (e = elevator carrying passenger, f = floor passenger wants to get off on)

Elevator Process:
   - When is an Elevator Asleep?:
       - No passengers on board
       - No passengers wishing to get picked up
       - Another elevator is already traveling to every floor with passengers waiting to go in that direction
   - Create event: Wakeup
       - We will only be working with two elevators so this is a lot simpler:
            - Before wakeup, one elevator must be on the top floor one must be on the bottom floor
            - After exactly one wake up, the other elevator still asleep must go to middle floor (i.e. Floor 8)

Entry and Exit from Elevator:
   - Replace single events with event sets
       - on_temnl replace with going_up
       - on_carlot replace with going_down
       - get_off_now replace with here_is_floor
   - Time to enter/leave elevator
       - Let's assume people aren't impatient assholes
            - First calculate time for everyone to leave elevator
            - Then calculate time for everyone to enter elevator

Passengers:
   - Three classes:
       -Incoming: called from ground floor going to uniformly chosen upper-level floor
       -Outgoing: called from uniformly chosen upper-level floor going to ground floor
       -Interfloor: called from uniformly chosen upper-level floor going to another uniformly chosen upper-level floor that ISN'T THE ONE THEY CAME FROM

Global Facility(?):
   - Instead of seperate facilities controlling head of line, use a single global facility update_workload.

Keeping track of how many people are in elevator:
   - Mentioned as step that would be addressed later but I'm assuming we do it now
   - Elevator size Array which keeps track of how many people are in elevator
