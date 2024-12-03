// scheduler.cc
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would
//	end up calling FindNextToRun(), and that would put us in an
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "scheduler.h"

#include "copyright.h"
#include "debug.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler() {
    readyList = new List<Thread *>;

    // MP3
    L1 = new List<Thread *>;  // Preemptive SJF
    L2 = new List<Thread *>;  // Non-preemptive Priority
    L3 = new List<Thread *>;  // Round Robin

    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler() {
    delete readyList;

    // MP3
    delete L1;
    delete L2;
    delete L3;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread) {
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    // cout << "Putting thread on ready list: " << thread->getName() << endl ;

    thread->UpdateRemainBurstTime();
    thread->setStatus(READY);
    //cout << "Thread Priority: " << thread->getPriority() << endl;
    //readyList->Append(thread);

    // MP3
    // Stop accumulating T when the thread becomes ready state (2-1 (f))

    if (thread->getPriority() >= 100) {
        addToQueue(thread, L1, 1);
    } else if (thread->getPriority() >= 50 && thread->getPriority() <= 99) {
        addToQueue(thread, L2, 2);
    } else {
        addToQueue(thread, L3, 3);
    }
    thread->setStartAgingTick(kernel->stats->totalTicks);
    


}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun() {
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    // if (readyList->IsEmpty()) {
    //     return NULL;
    // } else {
    //     return readyList->RemoveFront();
    // }

    // MP3
    Thread *pickedThread = NULL;
    ListIterator<Thread *> *it;
    if (!L1->IsEmpty()) {   // Preemptive SJF
        pickedThread = L1->Front();
        it = new ListIterator<Thread *>(L1);
        while(!it->IsDone()) {
            Thread *t = it->Item();
            if (t->getRemainBurstTime() == pickedThread->getRemainBurstTime()) {
                if (t->getID() < pickedThread->getID()) {
                    pickedThread = t;
                }
            } else if (t->getRemainBurstTime() < pickedThread->getRemainBurstTime()) {
                //cout << "Thread" << t->getID() << " : t_i - T = " << t->getRemainBurstTime() << ", ";
                //cout << "Thread" << pickedThread->getID()  << " : t_i - T = " << pickedThread->getRemainBurstTime() << endl;
                pickedThread = t;
            }
            it->Next();
        }
        return removeFromQueue(pickedThread, L1, 1);
    }
    else if (!L2->IsEmpty()) {  // Non-preemptive Priority
        pickedThread = L2->Front();
        it = new ListIterator<Thread *>(L2);
        while(!it->IsDone()) {
            Thread *t = it->Item();
            if (t->getPriority() == pickedThread->getPriority()) {
                if (t->getID() < pickedThread->getID()) {
                    pickedThread = t;
                }
            } else if (t->getPriority() > pickedThread->getPriority()) {
                pickedThread = t;
            }
            it->Next();
        }
        return removeFromQueue(pickedThread, L2, 2);
    }
    else if (!L3->IsEmpty()) {  // Round Robin
        pickedThread = L3->Front();
        return removeFromQueue(pickedThread, L3, 3);
    }
    else {
        //cout << "No thread in ready queue" << endl;
        return NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread, bool finishing) {
    Thread *oldThread = kernel->currentThread;

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {  // mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }

    if (oldThread->space != NULL) {  // if this thread is a user program,
        oldThread->SaveUserState();  // save the user's CPU registers
        oldThread->space->SaveState();
    }

    oldThread->CheckOverflow();  // check if the old thread
                                 // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running

    // MP3
    // Resume accumulating T when the thread moves back to the running state. (2-1 (f))
    nextThread->UpdateInitRunningTick();
    DEBUG(dbgScheduler, "[E] Tick [" << kernel->stats->totalTicks << "]: Thread [" << nextThread->getID() << "] is now selected for execution, thread [" << oldThread->getID() << "] is replaced, and it has executed [" << kernel->stats->totalTicks - oldThread->getInitRunningTick() << "] ticks");

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());

    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
    oldThread->UpdateInitRunningTick();

    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();  // check if thread we were running
                           // before this one has finished
                           // and needs to be cleaned up

    if (oldThread->space != NULL) {     // if there is an address space
        oldThread->RestoreUserState();  // to restore, do it.
        oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void Scheduler::CheckToBeDestroyed() {
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print() {
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}

// MP3
void Scheduler::addToQueue(Thread *thread, List<Thread *> *queue, int queueLevel) {
    queue->Append(thread);
    //thread->setStartAgingTick(kernel->stats->totalTicks); // 設定thread進入ready queue的tick
    thread->setQueueLevel(queueLevel);
    DEBUG('z', "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[" << queueLevel << "]" );
}

Thread *Scheduler::removeFromQueue(Thread* thread, List<Thread *> *queue, int queueLevel) {
    queue->Remove(thread);
    DEBUG('z', "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[" << queueLevel << "]" );
    return thread;
}

void Scheduler::UpdateThreadAging() {
    UpdateAgeInQueue(L1, 1);
    UpdateAgeInQueue(L2, 2);
    UpdateAgeInQueue(L3, 3);
}

void Scheduler::UpdateAgeInQueue(List<Thread*>* queue, int queueLevel){
    ListIterator<Thread*> iter(queue);
    while(!iter.IsDone()){
        Thread* currentThread = iter.Item();
        //currentThread->UpdateAgingTime();
        //currentThread->setStartAgingTick(kernel->stats->totalTicks);
        currentThread->UpdatePriority();
        iter.Next();

        int updatePriority = currentThread->getPriority();
        if(queueLevel == 2 && updatePriority >= 100){
            removeFromQueue(currentThread, L2, 2);
            addToQueue(currentThread, L1, 1);
        }
        else if(queueLevel == 3 && updatePriority >= 50){
            removeFromQueue(currentThread, L3, 3);
            addToQueue(currentThread, L2, 2);
        }
    }
}