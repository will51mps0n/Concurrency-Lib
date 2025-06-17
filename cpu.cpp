#include "cpu.h"

#include <iostream>
#include <memory>
#include <queue>

#include <ucontext.h>

#include "cv.h"
#include "global.h"
#include "mutex.h"
#include "thread.h"


/*
 * Allocate + initialize a pointer to the first thread context.
 * Put the pointer into ThreadContext object + push to readyThreads
 */
void cpu::startContext(thread_startfunc_t func, uintptr_t arg) {
    // Create context
    auto context = std::make_unique<ucontext_t>();
    char* ptr = new char[STACK_SIZE];

    context->uc_stack.ss_sp = ptr;
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = cpuContext.get();

    stackPointers.push_back(ptr);

    // Throw context into readyThreads
    makecontext(context.get(), reinterpret_cast<void (*)()>(threadStarter), 2, reinterpret_cast<uintptr_t>(func), arg);

    auto threadContext = std::make_unique<ThreadContext>(std::move(context));
    readyThreads.push(std::move(threadContext));
}

/*
 * Placeholder function for unneeded ipi interrupt (4 credit version)
 */
void cpu::ipiInterrupt() {
    return;
}

/*
 * Main driver for thread library.
 */
cpu::cpu(thread_startfunc_t func, uintptr_t arg) {
    std::unique_ptr<ucontext_t> context;

    // Initialize cpu context
    cpuContext = std::make_unique<ucontext_t>();
    char* ptr = new char[STACK_SIZE];

    cpuContext->uc_stack.ss_sp = ptr;
    cpuContext->uc_stack.ss_size = STACK_SIZE;
    cpuContext->uc_stack.ss_flags = 0;
    cpuContext->uc_link = nullptr;
    // TODO: do I put emplace back
    stackPointers.push_back(ptr);

    // Timer interrupt just yields, ipi does nothing
    interrupt_vector_table[TIMER] = thread::yield;
    interrupt_vector_table[IPI] = ipiInterrupt;

    // Boot first context
    if (func != nullptr) {
        startContext(func, arg);
    }
    // Execute threads until none are ready
    while (!readyThreads.empty()) {
        // Choose next available thread
        currentContext = std::move(readyThreads.front());
        readyThreads.pop();
        // std::cout << "here in cpu\n";
        swapcontext(cpuContext.get(), currentContext->context.get());
        // If thread isn't finished or blocked, put it back in line
        if (currentContext && !currentContext->completed && !currentContext->waitingOnLock) {
            readyThreads.push(std::move(currentContext));
        }
        // Clean up context stack pointers
        if (currentContext && currentContext->completed) {
            delete stackPointers[currentContext->id + 1];
        }
    }

    // Suspend CPU once all threads execute
    interrupt_enable_suspend();
}

bool cpu_interrupts_disabled() {
    return !cpu::self()->interrupts_enabled;
}