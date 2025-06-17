#include "mutex.h"

#include <iostream>

#include <assert.h>

#include "global.h"

mutex::mutex() {
    locked = false;
    ownerThreadID = -1;
}

/*
 * Ensure no other threads can access this resource
 */
void mutex::lock() {
    cpu::interrupt_guard guard;

    if (locked) {
        currentContext->waitingOnLock = true;
        waitingThreads.push(std::move(currentContext));

        ucontext_t* waitContext = waitingThreads.back()->context.get();
        if (!readyThreads.empty()) {
            currentContext = std::move(readyThreads.front());
            readyThreads.pop();
            swapcontext(waitContext, currentContext->context.get());
        } else {
            swapcontext(waitContext, cpuContext.get());
        }
    } else {
        locked = true;
        ownerThreadID = currentContext->id;
    }
}

/*
 * Allow other threads back into resource
 */
void mutex::unlock() {
    // ONLY LOCK IF NEEDED - fix so that cv wait can use m.lock()
    if (!cpu_interrupts_disabled()) {
        cpu::interrupt_guard guard;
    }

    // Ensure that currentContext exists
    //assert(currentContext && "Error currentContext is nullptr - In unlock function");

    // Ensure only the owner can unlock
    if (currentContext->id != ownerThreadID) {
        throw std::runtime_error("Error: Thread tried to unlock mutex it didnt own");
    }

    //assert(currentContext->id == ownerThreadID && "Error: Thread tried to unlock mutex it didnt own");

    // Reset owner
    locked = false;
    ownerThreadID = -1;

    // If someone is waiting, give them the lock
    if (!waitingThreads.empty()) {
        waitingThreads.front()->waitingOnLock = false;
        readyThreads.push(std::move(waitingThreads.front()));
        waitingThreads.pop();

        // Lock for next owner
        locked = true;
        ownerThreadID = readyThreads.back()->id;
    }
}