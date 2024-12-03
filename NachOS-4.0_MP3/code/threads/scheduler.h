// scheduler.h
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction --
// the data structures and operations needed to keep track of which
// thread is running, and which threads are ready but not running.

class Scheduler {
   public:
    Scheduler();   // Initialize list of ready threads
    ~Scheduler();  // De-allocate ready list

    void ReadyToRun(Thread* thread);
    // Thread can be dispatched.
    Thread* FindNextToRun();  // Dequeue first thread on the ready
                              // list, if any, and return thread.
    void Run(Thread* nextThread, bool finishing);
    // Cause nextThread to start running
    void CheckToBeDestroyed();  // Check if thread that had been
                                // running needs to be deleted
    void Print();               // Print contents of ready list

    // MP3

    void addToQueue(Thread* thread, List<Thread*>* queue, int queueLevel);
    Thread* removeFromQueue(Thread* thread, List<Thread*>* queue, int queueLevel);
    
    void UpdateThreadAging(); // 更新所有thread的aging
    bool IsL1NeedToPreempt(); // 判斷L1 queue是否需要preempt

    List<Thread*>* L1;
    List<Thread*>* L2;
    List<Thread*>* L3;

    // SelfTest for scheduler is implemented in class Thread

   private:
    List<Thread*>* readyList;  // queue of threads that are ready to run,
                               // but not running

    Thread* toBeDestroyed;     // finishing thread to be destroyed
                               // by the next thread that runs
      // MP3
    void UpdateAgeInQueue(List<Thread*>* queue, int queueLevel);   // 更新所有thread在queue中的aging
};

#endif  // SCHEDULER_H
