#include "command_dispatcher.h"
#include <SerialCommand.h>
#include "baremetal.h"

SerialCommand SCmd;

// This gets set as the default handler, and gets called when no other command matches. 
void unrecognized(){
  Serial.println("What?");
}

void echo(){
	char *arg;  
	Serial.print("ECHO "); 
	arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
	while (arg != NULL){     // As long as it exists, take it 
		Serial.print(arg); 
		arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
	} 
	Serial.print('\n'); //print newline to end this
}

void handle_data(){//dispatch to handlers once data is recieved
	SCmd.readSerial();
}

void initialize_handlers(){//Needs to have serial started beforehand
	SCmd.addCommand("RESET", reset);
	SCmd.addCommand("ECHO", echo);
	SCmd.addDefaultHandler(unrecognized);  
}