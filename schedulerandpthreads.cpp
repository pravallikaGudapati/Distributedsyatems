#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

const int TOTALMEM = 100;
const int MAXDEGREEMP = 4;
const int MAXPROCESSES = 8;

pthread_mutex_t Out;
pthread_mutex_t Mem;

// conditional variable to make threads wait if the
// corresponding limit has reached.

pthread_cond_t TotalMemLimit;
pthread_cond_t DegreeLimit;

void println (string s) {
    pthread_mutex_lock(&Out);
    cout << s << endl;
    pthread_mutex_unlock(&Out);
}

struct Memory {
    int id;
    int size;
};

class Scheduler {
private:
    int id;
    int totMem;
    int freeMem;
    Memory memory[MAXDEGREEMP+1];
    int next;

public:
    Scheduler ();
    Scheduler (int);
    void displayProcesses ();
    void getMemory (int id, int need);
    void returnMemory (int id, int need);
};

Scheduler::Scheduler () {
    totMem = 0;
    freeMem = 0;
    next = 0;
}

Scheduler::Scheduler (int totMem) {
    totMem= totMem;
    freeMem = totMem;
    for (int i = 0; i < MAXDEGREEMP+1; i++) {
        memory[i].id = 0; memory[i].size = 0;
    }
    next = 0;
}

void Scheduler::displayProcesses () {
	
    if (next == 0) return;
    string pl = "";
    for (int i = 0; i < next; i++)
        pl = pl + "Process " + to_string(memory[i].id) +
          ": memory size = " + to_string(memory[i].size) + "  ";
    println (pl);
}

void Scheduler::getMemory (int id, int need) {   
 pthread_mutex_lock(&Mem);
    // wait if insufficient free memory
    while (freeMem < need) {
        pthread_cond_wait(&TotalMemLimit, &Mem);
		println("the process waiting is :" +  to_string(id)); }

    freeMem = freeMem - need;

    // wait if the degree is too large
    while (next > MAXDEGREEMP) {
        pthread_cond_wait(&DegreeLimit, &Mem);
    }

    memory[next].id = id;
    memory[next].size = need;
    println ("getMemory:  assigned id " +  to_string(id) +
        " memory size " + to_string(need));
    next++;
    displayProcesses ();

    pthread_mutex_unlock(&Mem);
    return;
}

void Scheduler::returnMemory (int id, int need) {

    pthread_mutex_lock(&Mem);

    bool found = false;
    for (int i = 0; i < MAXDEGREEMP; i++) {
        if (id == memory[i].id) {
            found = true;
            freeMem = freeMem + memory[i].size;
            for (int j = i + 1; j < next; j++) {
                memory[j-1].id = memory[j].id;
                memory[j-1].size = memory[j].size;
            }
            next--;
            memory[next].id = 0;
            memory[next].size = 0;
            pthread_cond_signal(&TotalMemLimit);
            pthread_cond_signal(&DegreeLimit);
            break;
        }
    }
    if (!found)
        println("Error in program:" +  to_string(id) + " " +
            to_string(need));

    println ("returnMemory: returned id " + to_string(id) +
        " memory size " + to_string(need));
    displayProcesses ();

    pthread_mutex_unlock(&Mem);
    return;
}

class Process {
private:
    int id;
    unsigned int sid;        /* seed for rand */
    Scheduler* scheduler;
    int neededMemory;
    pthread_t thd;
    static void *pthread_run (void *args);
    void run();

public:
    void run (int, Scheduler*);
    void join();
};

void Process::run (int identifier, Scheduler* schd) {
    id = identifier;
    sid = id;
    scheduler = schd;
    neededMemory = 1 + rand_r(&sid) % TOTALMEM;
    pthread_create(&thd, NULL, pthread_run, this);
}

void *Process::pthread_run(void *args) {
    Process *proc = (Process *)args;
    proc->run();
    return NULL;
}

void Process::run() {
    scheduler->getMemory (id, neededMemory);
    sleep (1);
    scheduler->returnMemory (id, neededMemory);
    println ("process " + to_string(id) + ": finished");
}

void Process::join() {
    pthread_join(thd, NULL);
}

int main (int argc, const char * argv[]) {

pthread_mutex_init(&Out, NULL);
pthread_mutex_init(&Mem, NULL);
pthread_cond_init(&TotalMemLimit, NULL);
pthread_cond_init(&DegreeLimit, NULL);

Scheduler* schd = new Scheduler (TOTALMEM);
Process pthreads[MAXPROCESSES];
println ("All processes are active.");

for (int i = 0; i < MAXPROCESSES; i++) {
pthreads[i].run(i + 1, schd);
}

for (int i = 0; i < MAXPROCESSES; i++) {
pthreads[i].join();
}

println ("All processes have completed.");

pthread_mutex_destroy(&Out);
pthread_mutex_destroy(&Mem);
pthread_cond_destroy(&TotalMemLimit);
pthread_cond_destroy(&DegreeLimit);

return 0;
}

