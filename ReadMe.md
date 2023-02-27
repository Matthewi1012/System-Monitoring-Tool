
# CSCB09 A1: System Monitoring Tool

### Date: February 5th, 2023 

### By: Matthew Iannantuono

<br></br>

## **How to use the program**

    1. gcc -Wall mySystemStats.c -o mySystemStats
    2. ./mySystemStats (with flags below)
            --user: output user usage (users currently on the system).
            --system: output system usage (Memory and CPU).
            --sequential: output information sequentially without refreshing.
            --samples=N: indicate how many times the statistics will be collected (default is 10)
            --tdelay=T: how frequently to sample in seconds (default is 1)
            N T: positional arguments representing the values specified above,
                (5 2 == --samples=5 --tdelay=2)
    3. The order the command line arguments are entered in have no affect on the output, 
        except for the positional arguments N T.
    4. Ensure the terminal screen is of reasonable size to output properly

<br></br>

## **How I Solved the Problem**

    1. Refreshing the Page: To refresh the page i used ESCape codes to move the cursor
        to one line below the previous outputted information and cleared all output at
        that line and below. This gave the affect of storing previous outputs while also
        refreshing information below in being printed after T seconds. This occured N times
        as specified by the user. Once it had done N samples it would not clear again and 
        keep the remaining output to be visible.

    2. CPU Usage in %: in order to get two samples at different points in time i took
        a sample on the first iteration, waited T seconds, then every iteration after that
        i took a second sample, calculated the % based of the two cpu samples then outputed
        it to the screen. The formula i used to calculate the percentage is

            abs(T2 - T1) / T1 * 100
            T2 = currentSum - currentIdle * 100    <-- * 100 because the values are 1/100 of a second
            T1 = previousSum - previousIdle * 100  <-- * 100 because the values are 1/100 of a second

            abs(T2 - T1) -> gives the difference between the two samples
            abs(T2 - T1) / T1 -> gives the decimal of the increase
            abs(T2 - T1) / T1 * 100 -> gives the final values in percentage

    3. Printing Sequentially: Prints all information requested without using ESCape codes to clear the
        screen or move the cursor and does this for N samples every T seconds.


<br></br>

## **Functions Overview**

### **int getArgNum(char \*str, int position);**

    Description: This function returns the number in a given string as an Integer. This is used to extract
    the number from flags "--samples=N" and "--tdelay=T".
    Parameters: 
        - char *str: This parameter provides the string to extract the number from.
		- int position: This parameter provides the position of the first digit of the number in the string.

<br></br>

### **void printSysInfo();**

    Description: This function prints all system information needed to be proivided in the 
	"### System Information ###" block of output using the utsname.h library. It prints 
    the System name, Machine name, Version, Release, and Architecture information.

<br></br>

### **void printUserInfo();**

    Description: This function prints all user information needed to be proivided in the 
	"### Sessions/Users ###" block of output using the utmp.h library. It prints the Username,
    Device name, and Hostname of users currently on the system.

<br></br>

### **int getProcStatInfo(long \*sum, long \*idle);**

    Description: This function reads the file "/proc/stat" and gets the sum of the values on the
	first line of the file, representing the sum of cpu usage across all cpu's in
	the system at the current momment in time. It also gets the total idle time from
	the first line of the file representing the total idle time across all cpu in the system.
	Parameters: 
        - long *sum: a pointer to the variable outside of this function where the calculated
            sum time in this function is meant to be stored.
		- long *idle: a pointer to the variable outside of this function where the calculated
            idle time in this function is meant to be stored.

<br></br>

### **void printHeader(int N, int T);**

    Description: This function prints the header info displaying information about number
	of samples to be taken for every number of seconds, as well as the memory usage taken
    up by the current file. This information is gathered from the sys/resource.h library,
    and from the user input.
    Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void peintMemLine();**

    Description: This function prints a single line of memory information of the system
	at the current moment in time. It prints the memory usage in the form of,

		Physical Used GB / Physical Total GB --- Virtual Used GB / Virtual Total GB

	All memory information is accessed using the sys/sysinfo.h library and each value is calculated by,

		Physical total GB = total ram / 10^9
		Physical used GB = Phyical total - (free ram / 10^9)
		Virtual total GB = Physical total - (total swap memory / 10^9)
		Virtual Used GB = Virtual total - (free ram + free swap) / 10^9

<br></br>

### **float calcCpuUsage(long sumPrev, long idlePrev, long sumCur, long idleCur);**

    Description: This function calculates the difference in cpu usage in percentage given two samples
    of cpu usage calculated at two different points in time.
	Parameters: 
        - long sumPrev: This variable is the previous sum value of total cpu usage from the /proc/stat file 
		- long idlePrev: This variable is the previous hidle time value of cpu usage from the /proc/stat file 
		- long sumCur: This variable is the current sum of total cpu usage from the /proc/stat file 
		- long idleCur: This variable is the current idle time value of cpu usage from the /proc/stat file 

<br></br>

### **void printAllStats(int N, int T);**

    Description: This file prints all stats to be displayed if no command line flags are given.
	It prints N samples every T seconds of the system memory usage (from printMemLine()), current
    user information (from printUserInfo()), current cpu usage information (from calcCpuUsage()),
    the systems information (from printSysInfo()), and the values of N and T (from printHeader()).
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void checkArgs(int \*userCheck, int \*sysCheck, int \*seqCheck, int \*N, int \*T, int argc, char \*\*argv);**

    Description: This function traverses the command line arguments given and fills in the values
    specified by the user for number of samples and time, as well as specifying if a specific flag
    is in the given set of command line arguments.
	Parameters: 
        - int *userCheck: This pointer changes the value of a variable outside of this function,
            giving it a value of 1 if the flag "--user" is called and 0 otherwise.
		- int *sysCheck: This pointer changes the value of a variable outside of this function,
			giving it a value of 1 if the flag "--system" is called and 0 otherwise.
		- int *seqCheck: This pointer changes the value of a variable outside of this function,
			giving it a value of 1 if the flag "--sequential" is called and 0 otherwise.
		- int *N: This pointer changes the value of a variable outside of this function, giving it a
			a value N specified by the flag "--samples=N" or the first number in the positional 
            arguments "N T" representing the number of samples to be taken.
		- int *T: This pointer changes the value of a variable outside of this function, giving it a
			a value T specified by the flag "--tdelay=T" or the second number in the positional 
			arguments "N T" representing the time delay in seconds of each sample.
		- int argc: The number of command line arguments given at execution.
		- char **argv: The array of strings storing the command line arguments given at execution. 

<br></br>

### **void normalSeq(int N, int T);**

    Description: This function prints sequential output displaying all system information
	for N iterations/samples every T seconds. It provides information about memory usage, cpu usage,
    header information, user information, and system information.
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void printUserArg(int N, int T);**

    Description: This function prints current users information for the flag "--user", using the
	printUserInfo() function. It displays current information about the users every T seconds
	for N samples.
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void printUserSeq(int N, int T);**

    Description: This function prints current users information for the called flags "--user" and
	"--sequential", with N iterations/samples every T seconds in sequential order.
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void printSysArg(int N, int T);**

    Description: This function prints current system information for the flag "--system", using the
	printMemLine() and the calcCpuUsage() functions.
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

### **void printSysSeq(int N, int T);**

    Description: This function prints current system information for the called flags "--system" and
	"--sequential", with N iterations/samples every T seconds of memory and cpu usage.
	Parameters: 
        - int N: This variable is the number of samples to be taken specified by the user.
		- int T: This variable is the number of seconds every sample should be taken specified by the user.

<br></br>

## **Assumptions Made**

    1. The memory usage values given by the sys/sysinfo.h library are in bytes.
    2. If the user requests --user --system --sequential then the information needed to 
        be displayed is the same as calling just --sequential. --user requests user info,
        --system requests memory and cpu info, and just --sequential already prints that
        information. Therefore, both sets of command line arguments result in the same output.
    3. If the user requests --user --system then the information needed to be displayed is the
        same as calling just the executable file without flags. Both sets of command line
        arguments result in the same output.
    4. The order of the command line arguments does not affect the output except in the case
        of the positional argument N T (5 2 != 2 5).
    5. The cpu usage values in /proc/stat are 1/100 of a second.
    6. The memory usage info printed at each memory line is a snapshot of the memory usage at that point
        in time.

<br></br>