/*
The model for real tiem systems with
the non-preemptive global fixed priority scheduler,
the preemptive global fixed priority scheduler, 
the non-preemptive earliest deadline priority scheduler, and 
the preemptive earliest deadline priority scheduler (P-EDF). 
@author Natalia Garanina natta.garanina@gmail.com https://www.researchgate.net/profile/Natalia-Garanina
@conference PSSV-2024
@license GNU GPL
*/

#include "gen.pml"

byte busy = 0

bool release[NumTask];	// list of released  
bool go[NumTask];		// list of active  

int C_i[NumTask];
int D_i[NumTask];
int C_cur[NumTask];
int D_cur[NumTask];
int Util = 0;


chan task_shed = [0] of { bool };

bool BADD = false;

byte j = 0;
byte old = 0;
byte tmp = 0;
bool ins = false;
bool plan = false;

init { 

atomic {  
  setup();
  run tasks(); 
  run schedulerNPGPF(); // non-preemptive global fixed priority
}
}




inline task_plan(me, C, D){
	if 
	::  C_cur[me] == C && D_cur[me] == D && !release[me] -> // release
		if 
		:: release[me] = true; 
		   plan = true;
		:: skip; 
		fi
		
	:: C_cur[me] == 0 -> // finish 
			busy--;
			C_cur[me] = C;
			go[me] = false;
			if 
			:: D_cur[me] == 0 -> 
				D_cur[me] = D;
				if 
				:: release[me] = false; 
				:: skip; 
				fi			
			:: else -> release[me] = false; 
			fi
			plan = true;

	:: C_cur[me] > D_cur[me] && release[me] -> // fail deadline
			BADD = true; // point 6, 7
			tmp = me;
	:: else -> skip;
	fi
}

inline task_step(me, C, D){
	if 
	:: C_cur[me] > 0 && D_cur[me] > 0 && C_cur[me] <= D_cur[me] && release[me] ->  
			if 
			::  go[me] -> 
					C_cur[me]--; D_cur[me]--;  	// executing job
			:: else -> D_cur[me]--; 			// waiting execution
			fi		
		
	::  C_cur[me] == C && D_cur[me] > 0 && D_cur[me] < D && !release[me] -> // time till new release
			D_cur[me]--; 
			if 
			:: D_cur[me] == 0 -> 
				D_cur[me] = D;
			:: else -> skip;
			fi

	:: else -> skip;
	fi
}




proctype tasks () { 

byte i;
/*
atomic{
	for (i : 0 .. NumTask-1){ 
		C_cur[i] = i+1;
		D_cur[i] = 5*(i+2); 
		Util = Util + (100*(i+1))/(5*(i+2)); 
		que[i] = 255;
	}
Util = Util/NumProc;
}

#S(TASKSET
    :M 2
    :N 3
    :U 0.16298701
    :UC 0.2
    :TASKS (#S(TASK :C 1 :D 4 :T 4) 
			#S(TASK :C 4 :D 80 :T 80)
            #S(TASK :C 4 :D 154 :T 154)))
			*/

atomic{
	
	for (i : 0 .. NumTask-1){
		C_cur[i] = C_i[i];
		D_cur[i] = D_i[i]; 
		Util = Util + (100*C_i[i])/D_i[i]; 
		}
	Util = Util/NumProc;
}

do 
  :: atomic{
		for (i : 0 .. NumTask-1){ task_plan(i, C_i[i], D_i[i]); }
		if
		:: BADD -> break;
		:: else ->
			if 
			:: plan ->
				task_shed ! true;
				task_shed ? false;
				plan = false;
			:: else -> skip;
			fi
			for (i : 0 .. NumTask-1){ task_step(i, C_i[i], D_i[i]); }
		fi
	}
od
}

proctype schedulerNPGPF(){ 

byte free = 0;
byte i = 0;

 do 
 :: task_shed ? true ->
	atomic{
		free = NumProc - busy;
		for (i : 0 .. NumTask - 1){ 
			if 
			:: free == 0 -> break;
			:: else -> 
				if 
				:: release[i] && !go[i] && free != 0 -> 				
					go[i] = true; // go!
					busy++;
					free--;
				:: else -> skip;
				fi							
			fi	
		}
		task_shed ! false;
	}
 ::	BADD -> break;
 od;
}



 ltl p1 {[]!BADD } 


 
