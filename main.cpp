/*
* Author: #946 (Isaac Sikma)
* Course: ECE 46810
* Due Date: 02/13/23
* Description: This program takes an input file containing processes, input.txt, and simulates
* the operation of a CPU scheduler.
*/

#include <iostream>
#include <string>
#include <queue>
#include <sstream>
#include <vector>
using namespace std;

//////////////////////////////////////////////////
//Structures
//////////////////////////////////////////////////

//Resource request structure
struct resourceRequest{
    string name;
    int    time;

    //Default resourceRequest constructor
    resourceRequest(string name_, int time_)
    {
        name = name_;
        time = time_;
    }
};

//Processes have PID, Class, Start Time, Deadline, Current Status, and Resources
struct process{
    int           pid;
    string        processClass;
    int           startTime;
    int           deadline;
    string        status;
    queue<string> resources;

    //Default process constructor
    process(int pid_, string processClass_, int startTime_, int deadline_, string status_, queue<string> resources_)
    {
        pid = pid_;
        processClass = processClass_;
        startTime    = startTime_;
        deadline     = deadline_;
        status       = status_;
        resources    = resources_;
    }
};

//////////////////////////////////////////////////
//Defines
//////////////////////////////////////////////////
queue<process> interactiveReadyQ, realtimeReadyQ;
queue<process> cpuQ, diskQ, terminalQ;

vector<queue<string>> v1; // A vector holding queues of resource requests
vector<process> v2; // A vector holding every process

float totalTime           = 0; // Internal counter to know when processes start and stop
float cpuTotal            = 0; // Time CPU is being utilized
float diskTotal           = 0; // Time disk is being utilized
int ttyTotal              = 0; // Time terminal is being utilized
int realtimeFinished      = 0; // Number of real time proccesses finished
int interactiveFinished   = 0; // Number of interactive processes finished
int deadlineMissed        = 0; // Number of real time processes where the deadline was missed
float diskAccesses        = 0; // Number of disk accesses

//////////////////////////////////////////////////
//Functions
//////////////////////////////////////////////////

//Read redirected input from stdin
void readInput()
{
    string line, tmpString;
    //Flags for telling what process type we are working with
    int interactiveFlag, realtimeFlag;
    int counter, previousCounter;
    interactiveFlag = 0;
    realtimeFlag    = 0;
    counter         = 0;
    previousCounter = 0;

    while(getline(cin, line))
    {
        istringstream iss(line);
        string field;
        while(getline(iss, field, ' '))
        {
            tmpString = field;
            //Check interactive or real-time
            if(tmpString == "INTERACTIVE")
            {
                realtimeFlag = 0;
                interactiveFlag = 1;
                v1.emplace_back();
                counter++;
            } 
            else if(tmpString == "REAL-TIME")
            {
                interactiveFlag = 0;
                realtimeFlag = 1;
                v1.emplace_back();
                counter++;
            }
            //Check flags
            if(interactiveFlag == 1)
            {
                v1[counter - 1].emplace(tmpString);
            }
            else if(realtimeFlag == 1)
            {
                v1[counter - 1].emplace(tmpString);
            }
        }
    }
}

//Print Queue for debugging
void printQueue(queue<string> q)
{
    queue<string> temp = q;
    while(!temp.empty()) {
        cout << temp.front() << endl;
        temp.pop();
    }
}

//Print Process for debugging
void printProcess(process p)
{
    cout << p.pid << " ";
    cout << p.processClass << " ";
    cout << p.startTime << " ";
    cout << p.deadline << " ";
    cout << p.status << endl;
    printQueue(p.resources);
}

// Create processes from the vector of resource requests
void createProcess()
{
    int numProcesses; // Number of processes in the vector v1
    int processStartTime; // Time at which the process is scheduled to begin
    int processDeadline; // Deadline of the process if it is real-time
    numProcesses = v1.size();
    processStartTime = 0;
    processDeadline = 0;

    // Iterate over every list and create a process
    for (int i = 0; i < numProcesses; i++)
    {
        //cout << v1[i].front() << endl;
        // Interactive or real-time
        if(v1[i].front() == "INTERACTIVE")
        {
            v1[i].pop(); // Remove process class entry
            processStartTime = stoi(v1[i].front()); // Record process start time
            v1[i].pop(); // Remove process start time entry
            // Create an interactive process and add it the process table and interactive queue
            v2.emplace_back(i, "INTERACTIVE", processStartTime, NULL, "WAITING", v1[i]);
            interactiveReadyQ.push(v2[i]);
        }
        else if(v1[i].front() == "REAL-TIME")
        {
            v1[i].pop(); // Remove process class
            processStartTime = stoi(v1[i].front());
            v1[i].pop(); // Remove start time
            v1[i].pop(); // Remove DEADLINE text
            processDeadline = stoi(v1[i].front());
            v1[i].pop(); // Remove deadline time
            // Create a real-time process 
            v2.emplace_back(i, "REAL-TIME", processStartTime, processDeadline, "WAITING", v1[i]);
            realtimeReadyQ.push(v2[i]);
        }
    }
}

// The CPU receives a process, then keeps track of how long it was used
void cpu(int time_ms)
{
    int cpuTime = 0;
    while (cpuTime < time_ms)
    {
        cpuTime++;
    }
    cpuTotal += cpuTime;
    totalTime += cpuTime;
}

// The disk receives a process, then keeps track of how long it was used
void disk(int time_ms)
{
    int diskTime = 0;
    while (diskTime < time_ms)
    {
        diskTime++;
    }
    diskTotal += diskTime;
    totalTime += diskTime;
    diskAccesses++;
}

// The terminal receives a process, then keeps track of how long it was used
void tty(int time_ms)
{
    int ttyTime = 0;
    while (ttyTime < time_ms)
    {
        ttyTime++;
    }
    ttyTotal += ttyTime;
    totalTime += ttyTime;
}

// The scheduler function
void scheduler()
{
    /*
      The scheduler is responsible for allocating the
      CPU, disk, and terminal to processes who need it.
    */
   int interactiveStartTime = 0;
   int realtimeStartTime    = 0;

   // Continually check the interactive and real-time queues for processes
   while((!realtimeReadyQ.empty()))
   {
        // Check the start times of the processes at the front of the queues
        realtimeStartTime = realtimeReadyQ.front().startTime;
        if (totalTime >= realtimeStartTime)
        {
            cout << "Process #" << realtimeReadyQ.front().pid << " (REAL-TIME) started at time " << totalTime << "." << endl;        
            // Grab the next process and complete all of the requests inside
            while(!realtimeReadyQ.front().resources.empty())
            {
                // CPU request
                if(realtimeReadyQ.front().resources.front() == "CPU")
                {
                    realtimeReadyQ.front().resources.pop(); // Remove entry type
                    cpu(stoi(realtimeReadyQ.front().resources.front())); // Allocate the resource
                    realtimeReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
                // Disk request
                if(realtimeReadyQ.front().resources.front() == "DISK")
                {
                    realtimeReadyQ.front().resources.pop(); // Remove entry type
                    disk(stoi(realtimeReadyQ.front().resources.front())); // Allocate the resource
                    realtimeReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
                // Terminal request
                if(realtimeReadyQ.front().resources.front() == "TTY")
                {
                    realtimeReadyQ.front().resources.pop(); // Remove entry type
                    tty(stoi(realtimeReadyQ.front().resources.front())); // Allocate the resource
                    realtimeReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
            }
            cout << "Process #" << realtimeReadyQ.front().pid << " (REAL-TIME) terminated at time " << totalTime << "." << endl;
            realtimeReadyQ.pop(); // Remove the terminated process from the queue
            realtimeFinished++;
        }
        totalTime++;
   }
   while(!interactiveReadyQ.empty())
   {
        interactiveStartTime = interactiveReadyQ.front().startTime;
        if (totalTime >= interactiveStartTime)
        {
            cout << "Process #" << interactiveReadyQ.front().pid << " (INTERACTIVE) started at time " << totalTime << "." << endl;
            // Grab the next process and complete all of the requests inside
            while(!interactiveReadyQ.front().resources.empty())
            {
                // CPU request
                if(interactiveReadyQ.front().resources.front() == "CPU")
                {
                    interactiveReadyQ.front().resources.pop(); // Remove entry type
                    cpu(stoi(interactiveReadyQ.front().resources.front())); // Allocate the resource
                    interactiveReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
                // Disk request
                if(interactiveReadyQ.front().resources.front() == "DISK")
                {
                    interactiveReadyQ.front().resources.pop(); // Remove entry type
                    disk(stoi(interactiveReadyQ.front().resources.front())); // Allocate the resource
                    interactiveReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
                // Terminal request
                if(interactiveReadyQ.front().resources.front() == "TTY")
                {
                    interactiveReadyQ.front().resources.pop(); // Remove entry type
                    tty(stoi(interactiveReadyQ.front().resources.front())); // Allocate the resource
                    interactiveReadyQ.front().resources.pop(); // Remove the time to get ready for the next request
                }
            }
            cout << "Process #" << interactiveReadyQ.front().pid << " (INTERACTIVE) terminated at time " << totalTime << "." <<  endl;
            interactiveReadyQ.pop(); // Remove the terminated process from the queue
            interactiveFinished++;
        }
        totalTime++;
   }
   // Print the summary report
   cout << "--------------------SUMMARY REPORT--------------------" << endl;
   cout << realtimeFinished << " real-time processes completed." << endl;
   cout << deadlineMissed << " real-time processes missed their deadline." << endl;
   cout << interactiveFinished << " interactive processes completed." << endl;
   cout << "The disk was accessed " << diskAccesses << " times." << endl;
   cout << "The average duration of a disk access was " << diskTotal/diskAccesses << " milliseconds." << endl;
   cout << "Total time elapsed is " << totalTime << " milliseconds." << endl;
   cout << "CPU utilization: " << (cpuTotal/totalTime)*100 << "%." << endl;
   cout << "Disk utilization: " << (diskTotal/totalTime)*100 << "%." << endl;
}

// The main function
int main()
{
    readInput();
    createProcess();
    scheduler();
    return 0;
}