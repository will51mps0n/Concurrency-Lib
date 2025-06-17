#include "global.h"

#include <iostream>

#include "cpu.h"
#include "thread.h"

/*
 * Executes user code, marks when the thread is done
 */
void threadStarter(thread_startfunc_t func, uintptr_t arg) {
    // Interrupts are already enabled from CPU swapcontext call
    cpu::interrupt_enable();
    // Execute user code when a thread is running
    func(arg);
    cpu::interrupt_disable();
    // Once func(arg) returns, we know this thread is done
    if (currentContext) {
        currentContext->completed = true;
        *(currentContext->threadAlive) = false;
    }

    // Wake up the threads waiting on this thread to finish
    while (!currentContext->threadJoinQueue->empty()) {
        readyThreads.push(std::move(currentContext->threadJoinQueue->front()));
        currentContext->threadJoinQueue->pop();
    }

    // Once thread is done, swap to the next thread
    swapcontext(currentContext->context.get(), cpuContext.get());
}

enum Level { MEDIUM, HIGH };

/*
 * Goes to next thread. If no readyThreads, swap to CPU
 */
void go_to_next_thread() {
    if (!readyThreads.empty()) {
        readyThreads.push(std::move(currentContext));
        currentContext = std::move(readyThreads.front());
        readyThreads.pop();
        swapcontext(readyThreads.back()->context.get(), currentContext->context.get());
    } else {
        swapcontext(currentContext->context.get(), cpuContext.get());
    }
}

int ThreadContext::nextID = 0;
std::queue<std::unique_ptr<ThreadContext>> readyThreads;
std::unique_ptr<ucontext_t> cpuContext;
std::unique_ptr<ThreadContext> currentContext;

std::vector<char*> stackPointers;