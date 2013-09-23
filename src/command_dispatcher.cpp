#include "command_dispatcher.h"
#include <SerialCommand.h>
#include "baremetal.h"

SerialCommand SCmd;

// This gets set as the default handler, and gets called when no other command matches. 
void unrecognized(){
  Serial.println("What?");
}

void initialize_handlers(){//Needs to have serial started beforehand
	SCmd.addCommand("RESET", reset);
	SCmd.addDefaultHandler(unrecognized);  
}

void handle_data(){//dispatch to handlers once data is recieved
	SCmd.readSerial();
}



