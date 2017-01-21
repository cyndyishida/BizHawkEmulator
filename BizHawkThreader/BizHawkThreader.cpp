// BizHawkThreader.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <iostream>



using namespace std;

/// path of emulator file 
string PATH_OF_EMULATOR = "/mnt/c/Users/Cyndy/Projects/SideProjects/SpartaHacks17-master/SpartaHacks17";

int main()
{
		string current_command =PATH_OF_EMULATOR;
		const char* val = current_command.c_str();
		int temp = chdir(val);

		current_command = "./EmuHawkMulti.exe ";
		
		//  + " --thread-count= 2"
		val = current_command.c_str();
		system(val);
	

	//}



    return 0;
}

