// BizHawkThreader.cpp : Defines the entry point for the console application.
//


#include <stdio.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <iostream>
#include <thread>


using namespace std;



/**
  *
  *
  */
void *RunEmulator(string command, int threadCount)
{
	 command += to_string(threadCount);
	 const char* thread_execution = command.c_str();
	 system(thread_execution);


}




/// path of emulator file 
string PATH_OF_EMULATOR = "/mnt/c/Users/Cyndy/Projects/SideProjects/SpartaHacks17-master/SpartaHacks17";

int main()
{
		// change of directory, will make more dynamic once ready to merge
		string current_command = PATH_OF_EMULATOR;
		const char* val = current_command.c_str();
		int temp = chdir(val);



		current_command = "./EmuHawkMulti.exe --thread-count=";
		for (int i = 0; i<2; ++i)
		{
		// thread call
			thread currentThread(RunEmulator, current_command, i+1);
		//if(currentThread.joinable()) 
			currentThread.join();

			cout << i << endl;
		}




    return 0;
}



/*
#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS     5

void *PrintHello(void *threadid)
{
   long tid;
   tid = (long)threadid;
   printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   pthread_t threads[NUM_THREADS];
   int rc;
   long t;
   for(t=0; t<NUM_THREADS; t++){
      printf("In main: creating thread %ld\n", t);
      rc = pthread_create(&threads[t], NULL, PrintHello, (void *)t);
      if (rc){
         printf("ERROR; return code from pthread_create() is %d\n", rc);
         exit(-1);
      }
   }
   pthread_exit(NULL);
}



*/