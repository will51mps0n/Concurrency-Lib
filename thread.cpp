#include "thread.h"

#include <iostream>
#include <memory>
#include <queue>

#include <ucontext.h>
#include <unistd.h>

#include "cpu.h"
#include "cv.h"
#include "global.h"
#include "mutex.h"

/*
 * Thread constructor: creates its context, sets
 */
thread::thread(thread_startfunc_t func, uintptr_t arg) {
    // From user code = disable interrupts
    cpu::interrupt_guard guard;

    context = std::make_unique<ucontext_t>();
    char* ptr = new char[STACK_SIZE];

    context->uc_stack.ss_sp = ptr;
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = cpuContext.get();

    stackPointers.push_back(ptr);

    // Call user function with thread
    makecontext(context.get(), reinterpret_cast<void (*)()>(threadStarter), 2, reinterpret_cast<uintptr_t>(func), arg);

    // Push context to ready threads
    std::unique_ptr<ThreadContext> threadCtx;

    threadCtx = std::make_unique<ThreadContext>(std::move(context));

    isAlive = threadCtx->threadAlive;
    threadJoinQueue = threadCtx->threadJoinQueue;
    readyThreads.push(std::move(threadCtx));
}

/*
 * Waits on the calling thread to finsh before proceeding
 */
void thread::join() {
    cpu::interrupt_guard guard;

    // If the calling thread is already done, skip + keep going with execution
    if (!(*isAlive)) {
        return;
    }

    // Add currentContext to waiting queue for this thread
    // It will sit here until this thread finishes
    threadJoinQueue->push(std::move(currentContext));

    if (!readyThreads.empty()) {
        currentContext = std::move(readyThreads.front());
        readyThreads.pop();
        swapcontext(threadJoinQueue->back()->context.get(), currentContext->context.get());
    } else {
        currentContext->waitingOnLock = true;
        swapcontext(currentContext->context.get(), cpuContext.get());
    }
}


/*
 * Surrender control to CPU, which will run next ready thread
 */
void thread::yield() {
    // Just go to next thread
    cpu::interrupt_guard guard;
    go_to_next_thread();
}

/*
 * Placeholder:
 * All allocated memory is deleted in cpu::cpu
 */
thread::~thread() {
    return;
}