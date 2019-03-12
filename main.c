// Programmer: Andrew McCormick 
// Function:   main.c
// Class:      CS-4760 PA 1
// Date:       2/7/2019

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static const char *optString = "ho:i:n:s:";

int main(int argc, char * argv[])
{
	opt = getopt(argc, argv, optString);

	while(opt != -1)
	{
		switch(opt)
		{
			// for option -o outputfile
			case 'o':
				files.outputFileName = optarg;
				break;

			// for option -i inputfile
			case 'i':
				files.inputFileName = optarg;
				break;
					
			// for option -n number of child processes
			case 'n':
				files.numChild = optarg;
				break;
			
			// for option -s number of childern running at a time
			case 's':
				files.childAtTime = optarg;
				break;

			case 'h':
				print("-i: name_of_inputfile,\n-o name_of_outputfile,\n-n: Number of child processes,\n-s: Number of child running at a time");
				return EXIT_SUCCESS;
		}
		
		opt = getopt(argc, argv, optString);
	}

}
