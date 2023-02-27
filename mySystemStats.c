#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

/*
	Author: Matthew Iannantuono
	Date: Feb 5, 2023
	Title: CSCB09 A1: System Monitoring Tool
*/


int getArgNum(char *str, int position){
	/*
		@description: This function returns the number in a given string as an Integer.
		@parameters: char *str: This parameter provides the string to extract the number from.
					 int position: This parameter provides the position of the first digit of the number in the string.
	*/

	int numSize = strlen(str) - position;
	char num[numSize];
	for(int i = 0; i < numSize; i++){
		num[i] = str[i + position];	
	}
	return atoi(num);
}

void printSysInfo(){
	/*
		@description: This function prints all system information needed to be proivided in the 
					  "### System Information ###" block of output using the utsname.h library.
					  It prints System name, Machine name, Version, Release, and Architecture.
	*/

	printf("--------------------------------------------------\n");	
	printf("### System Information ###\n");
	struct utsname *buf = malloc(sizeof(struct utsname));
	uname(buf);
	printf(" System Name = %s\n", buf->sysname);	
	printf(" Machine Name = %s\n", buf->nodename);
	printf(" Version = %s\n", buf->version);
	printf(" Release = %s\n",buf->release);
	printf(" Architecture = %s\n", buf->machine);
	free(buf);
	printf("--------------------------------------------------\n");	
}

void printUserInfo(){
	/*
		@description: This function prints all user information needed to be proivided in the 
					  "### Sessions/Users ###" block of output using the utmp.h library.It prints
					  the Username, Device name, and Hostname of users currently on the system.
	*/

	printf("--------------------------------------------------\n");
    printf("### Sessions/Users ###\n");
	struct utmp *users;
	setutent();
	users = getutent();
	while(users != NULL){
		if(users->ut_type == USER_PROCESS){
			printf(" %s %15s (%s)\n",users->ut_user, users->ut_line, users->ut_host);
		}
		users = getutent();
	}
	printf("--------------------------------------------------\n");	
}

int getProcStatInfo(long *sum, long *idle){
	/*
		@description: This function reads the file "/proc/stat" and gets the sum of the values on the
					  first line of the file, representing the sum of cpu usage across all cpu's in
					  the system at the current momment in time. It also gets the total idle time from
					  the first line of the file representing the total idle time across all cpu in the system.
		@parameters: long *sum: a pointer to the variable outside of this function where the calculated sum time in this
						        function is meant to be stored.
					 long *idle: a pointer to the variable outside of this function where the calculated idle time in this
					            function is meant to be stored.
	*/

	FILE *cpuInfo = fopen("/proc/stat","r");
	if(cpuInfo == NULL){
		fprintf(stderr, "Error opening /proc/stat\n");
		return 1;
	}
	int i = 0;
	char line[150];
	fgets(line, 150, cpuInfo);
	char *num = strtok(line, " ");
	num = strtok(NULL," ");
	while(num != NULL){
        *sum += strtol(num, NULL, 10);
		if(i == 3){
			*idle = strtol(num, NULL, 10);
		}
		num = strtok(NULL, " ");
        i++;
	}
	if(fclose(cpuInfo) != 0){
		fprintf(stderr, "fclose failed\n");
		return 1;
	}
	return 0;
}


void printHeader(int N, int T){
	/*
		@description: This function prints the header info displaying information about number
					  of samples to be taken for every number of seconds, as well as the memory
					  usage taken up by this current file. This information is gathered from the 
					  sys/resource.h library, and from the user input.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/
    
	struct rusage *use = malloc(sizeof(struct rusage));;
	getrusage(RUSAGE_SELF, use);
	printf("Nbr of samples: %d -- every %d secs\n ", N, T);
    printf(" Memory Usage %ld kb\n", use -> ru_maxrss );
	free(use);
	printf("--------------------------------------------------\n");	
}

void printMemLine(){
	/*
		@description: This function prints a single line of the memory information of the system
					  at the current moment in time. It prints the memory usage in the form of

							Physical Used GB / Physical Total GB --- Virtual Used GB / Virtual Total GB

					  All memory information is accessed using the sys/sysinfo.h library and each value is
					  calculated by,

							Physical total GB = total ram / 10^9
							Physical used GB = Phyical total - (free ram / 10^9)
							Virtual total GB = Physical total - (total swap memory / 10^9)
							Virtual Used GB = Virtual total - (free ram + free swap) / 10^9
	*/
	
	struct sysinfo *info = malloc(sizeof(struct sysinfo));
	sysinfo(info);
	float pTotal = info -> totalram /  1000000000.0;
	float pUsed = pTotal - (info -> freeram / 1000000000.0);
	float vTotal = pTotal + (info -> totalswap / 1000000000.0);
	float vUsed = vTotal - ((info -> freeram + info -> freeswap) / 1000000000.0);
	printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", pUsed, pTotal, vUsed, vTotal);
	free(info);
}

float calcCpuUsage(long sumPrev, long idlePrev, long sumCur, long idleCur){
	/*
		@description: This function calculates the difference in cpu usage in percentage given two 
					  samples of cpu usage calculated at two different points in time.
		@parameters: long sumPrev: This variable is the previous sum value of total cpu usage from the /proc/stat file 
					 long idlePrev: This variable is the previous idle time value of cpu usage from the /proc/stat file 
					 long sumCur: This variable is the current sum of total cpu usage from the /proc/stat file 
					 long idleCur: This variable is the current idle time value of cpu usage from the /proc/stat file 
	*/

	float total2 = (float)(sumCur - idleCur) * 100;
	float total1 = (float)(sumPrev - idlePrev) * 100;
	return (abs(total2 - total1) / total1) * 100;
}

void printAllStats(int N, int T){
	/*
		@description: This file prints all stats to be displayed if no command line flags are given.
					  It prints N samples every T seconds of the system memory usage (from printMemLine()),
					  current user information (from printUserInfo()), current cpu usage information 
					  (from calcCpuUsage()), the systems information (from printSysInfo()), and the 
					  values of N and T (from printHeader()).
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken specified by the user.
	*/

	long sumPrev = 0;
	long idlePrev = 0;
	long sumCur = 0;
	long idleCur = 0;
	float usage = 0;

	system("clear");
	printHeader(N, T);
	printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
	for(int i = 0; i < N; i++){
		printMemLine();
		for(int j = i + 1; j < N; j++){
			printf("\n");	
		}
		printUserInfo();
		printf("Number of cores: %d\n", get_nprocs());
		if( i > 0){
			getProcStatInfo(&sumCur, &idleCur);
			usage = calcCpuUsage(sumPrev, idlePrev, sumCur, idleCur);
		}
		printf(" total cpu use = %.2f%%\n", usage);
		getProcStatInfo(&sumPrev, &idlePrev);
		sleep(T);
		if(i != N-1){
			printf("\x1b[%d;0H", (i+4)+2);
			printf("\x1b[0J");
		}
	}
	printSysInfo();
}

void checkArgs(int *userCheck, int *sysCheck, int *seqCheck, int *N, int *T, int argc, char **argv){
	/*
		@description: This function traverses the command line arguments given and fills in the values
					  specified by the user for number of samples and time, as well as specifying if
					  a specific flag is in the given set of command line arguments.
		@parameters: int *userCheck: This pointer changes the value of a variable outside of this function,
									giving it a value of 1 if the flag "--user" is called and 0 otherwise.
					 int *sysCheck: This pointer changes the value of a variable outside of this function,
									giving it a value of 1 if the flag "--system" is called and 0 otherwise.
					 int *seqCheck: This pointer changes the value of a variable outside of this function,
									giving it a value of 1 if the flag "--sequential" is called and 0 otherwise.
					 int *N: This pointer changes the value of a variable outside of this function, giving it a
					 		 a value N specified by the flag "--samples=N" or the first number in the positional 
							 arguments "N T" representing the number of samples to be taken
					 int *T: This pointer changes the value of a variable outside of this function, giving it a
					 		 a value T specified by the flag "--tdelay=T" or the second number in the positional 
							 arguments "N T" representing the time delay in seconds of each sample.
					 int argc: The number of command line arguments given at execution.
					 char **argv: The array of strings holding the command line arguments given at execution. 
	*/
	
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "--sequential") == 0){
			*seqCheck = 1;
		} else if (strncmp(argv[i], "--samples=", 10) == 0){
			*N = getArgNum(argv[i], 10);
		} else if (strncmp(argv[i], "--tdelay=", 9) == 0){
			*T = getArgNum(argv[i], 9);
		} else if(strcmp(argv[i], "--user") == 0){
		 	*userCheck = 1;      
		} else if(strcmp(argv[i], "--system") == 0){
			*sysCheck = 1;
		} else if(isdigit(argv[i][0])){
			*N = atoi(argv[i]);
			*T = atoi(argv[i+1]);
			i++;
		} else {
			fprintf(stderr,"%s flag not found\n", argv[i]);
			exit(0);
		}
	}
}

void normalSeq(int N, int T){
	/*
		@description: This function prints a normal sequential output displaying all system information
					  for N iterations/samples every T seconds. It provides information about memory usage,
					  cpu usage, header information, user information, and system information.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/

	long sumPrev = 0;
	long idlePrev = 0;
	long sumCur = 0;
	long idleCur = 0;
	float usage = 0;
	
	system("clear");
	for(int i = 0; i < N; i++){
		printf(">>> iteration %d\n", i);
		struct rusage *use = malloc(sizeof(struct rusage));
		getrusage(RUSAGE_SELF, use);
		printf(" Memory Usage %ld kb\n", use -> ru_maxrss);
		free(use);	
		printf("--------------------------------------------------\n");	
		printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
		for(int j = 0; j < N; j++){
			if(j == i){
				printMemLine();
			} else {
				printf("\n");
			}
		}
		printUserInfo();
		printf("Number of cores: %d\n", get_nprocs());
		if( i > 0){
			getProcStatInfo(&sumCur, &idleCur);
			usage = calcCpuUsage(sumPrev, idlePrev, sumCur, idleCur);
		}
		printf(" total cpu use = %.2f%%\n", usage);
		getProcStatInfo(&sumPrev, &idlePrev);
		sleep(T);
	}	
	printSysInfo();
}

void printUserArg(int N, int T){
	/*
		@description: This function prints current users information for the flag "--user", using the
					  printUserInfo() function. It displays current information about the users every T seconds
					  for N samples.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/

	system("clear");
	printHeader(N, T);
	for(int i = 0; i < N; i++){
		printUserInfo();
		sleep(T);
		if(i != N - 1){
			printf("\x1b[4;0H");
			printf("\x1b[0J");
		}
	}
	printSysInfo();
}

void printUserSeq(int N, int T){
	/*
		@description: This function prints current users information for the called flags "--user" and
					  "--sequential", with N iterations/samples every T seconds in sequential order.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/

	system("clear");
	for(int i = 0; i < N; i++){
		printf(">>> iteration %d\n", i);
		struct rusage *use = malloc(sizeof(struct rusage));
		getrusage(RUSAGE_SELF, use);
		printf("Memory Usage %ld kb\n", use -> ru_maxrss);
		free(use);
		printUserInfo();
		sleep(T);
	}
	printSysInfo();
}

void printSysArg(int N, int T){
	/*
		@description: This function prints current system information for the flag "--system", using the
					  printMemLine() and the calcCpuUsage() functions.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/

	long sumPrev = 0;
	long idlePrev = 0;
	long sumCur = 0;
	long idleCur = 0;
	float usage = 0;

	system("clear");
	printHeader(N, T);
	printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
	for(int i = 0; i < N; i++){
		printMemLine();
		for(int j = i + 1; j < N; j++){
			printf("\n");
		}
		printf("Number of cores: %d\n", get_nprocs());
		if( i > 0){
			getProcStatInfo(&sumCur, &idleCur);
			usage = calcCpuUsage(sumPrev, idlePrev, sumCur, idleCur);
		}
		printf(" total cpu use = %.2f%%\n", usage);
		getProcStatInfo(&sumPrev, &idlePrev);
		sleep(T);
		if(i != N - 1){
			printf("\x1b[%d;0H", (i+4)+2);
			printf("\x1b[0J");
		}
	}	
	printSysInfo();
}

void printSysSeq(int N, int T){
	/*
		@description: This function prints current system information for the called flags "--system" and
					  "--sequential", with N iterations/samples every T seconds.
		@parameters: int N: This variable is the number of samples to be taken specified by the user
					 int T: This variable is the number of seconds every sample should be taken 
					        specified by the user.
	*/ 

	long sumPrev = 0;
	long idlePrev = 0;
	long sumCur = 0;
	long idleCur = 0;
	float usage = 0;
	
	for(int i = 0; i < N; i++){
		printf(">>> iteration %d\n", i);
		struct rusage *use = malloc(sizeof(struct rusage));
		getrusage(RUSAGE_SELF, use);
		printf("Memory Usage %ld kb\n", use -> ru_maxrss);
		free(use);
		printf("--------------------------------------------------\n");	
		printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
		for(int j = 0; j < N; j++){
			if(j == i){
				printMemLine();
			} else {
				printf("\n");
			}
		}
		printf("Number of cores: %d\n", get_nprocs());
		if( i > 0){
			getProcStatInfo(&sumCur, &idleCur);
			usage = calcCpuUsage(sumPrev, idlePrev, sumCur, idleCur);
		}
		printf(" total cpu use = %.2f%%\n", usage);
		getProcStatInfo(&sumPrev, &idlePrev);
		sleep(T);
	}
	printSysInfo();
}

int main(int argc, char **argv){
    
    int N = 10;
    int T = 1;
    int seqCheck = 0;
    int sysCheck = 0;
    int userCheck = 0;

	/*
		Gather info about the called command line arguments to decide what information is 
		to be displayed and how it should be displayed.
	*/
    checkArgs(&userCheck, &sysCheck, &seqCheck, &N, &T, argc, argv);

    if((userCheck && sysCheck && seqCheck) || (!userCheck && !sysCheck && seqCheck)){
		/*
			This if statement is satisfied if the following command line arguments are called with
			the flags in no specific order: ./a.out --user --system --sequential OR ./a.out --sequential

			By assumption, --user --system --sequential and --sequential request the same inforamtion to be displayed
			in sequential order so both set of flags call the same function normalSeq().
		*/
		normalSeq(N, T);
    } else if((userCheck && sysCheck && !seqCheck) || (!userCheck && !sysCheck && !seqCheck) ){
		/*
			This else if statement is satisfied if the following command line arguments are called with
			the flags in no specific order: ./a.out --user --system OR ./a.out

			By assumption, ./a.out --user --system and ./a.out equest the same inforamtion to be displayed
			so both set of flags call the same function printAllStats().
		*/
		printAllStats(N, T);
    } else if(userCheck && !sysCheck && seqCheck){
		/*
			This else if statement is satisfied if the following command line arguments are called with
			the flags in no specific order: ./a.out --user --sequential
		*/
		printUserSeq(N, T);
    } else if(!userCheck && sysCheck && seqCheck){
		/*
			This else if statement is satisfied if the following command line arguments are called with
			the flags in no specific order: ./a.out --system --sequential
		*/
		printSysSeq(N, T);
    } else if(userCheck && !sysCheck && !seqCheck){
		/*
			This else if statement is satisfied if the following command line arguments are called: ./a.out --user --sequential
		*/
		printUserArg(N, T);
    } else if(!userCheck && sysCheck && !seqCheck){
		/*
			This else if statement is satisfied if the following command line arguments are called: ./a.out --system
		*/
		printSysArg(N, T);
    }

    return 0;
}
