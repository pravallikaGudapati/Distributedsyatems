# Distributedsyatems
The program simulates the scheduling of processes to memory. Each process has a randomly generated amount of memory that it requires. 
a.The program Use pthreads instead of C++ version 11 threads.
b. It uses arrays to keep track of the different threads.
c. The Scheduler class routines getMemory and returnMemory may be executed by multiple threads at the same time. All shared variables in these routines are updated in a mutually exclusive manner.
d. The program does not take into account how much memory there actually is, nor does it take into account the maximum number of process that can be running at one time.
These limits are given by TOTALMEM and MAXDEGREEMP, respectively. If either one of these limits would be exceeded, that then the process thread will wait until these conditions no longer exist. 
The output is a message that identifies which thread is waiting.
