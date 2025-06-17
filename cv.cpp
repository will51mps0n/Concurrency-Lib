#include "cv.h"

#include <ucontext.h>

cv::cv() {}

/*
 * Release mutex and go to sleep until broadcast or signal is called
 */
void cv::wait(mutex& m) {
    cpu::interrupt_guard guard;
    if (!m.locked) {
        go_to_next_thread();
        return;
    }

    if (m.ownerThreadID != currentContext->id) {
        throw std::runtime_error("Thread does not own mutex!!!!!!");
    }
    
    m.unlock();  
    cvWaitingThreads.push(std::move(currentContext));

    ucontext_t* savedContext = cvWaitingThreads.back()->context.get();

    if (!readyThreads.empty()) {
        currentContext = std::move(readyThreads.front());
        readyThreads.pop();
        swapcontext(savedContext, currentContext->context.get());
    } else {
        swapcontext(savedContext, cpuContext.get());
    }

    // After waking up
    if (m.locked) {
        currentContext->waitingOnLock = true;
        m.waitingThreads.push(std::move(currentContext));

        ucontext_t* waitingContext = m.waitingThreads.back()->context.get();
        if (!readyThreads.empty()) {
            currentContext = std::move(readyThreads.front());
            readyThreads.pop();
            swapcontext(waitingContext, currentContext->context.get());
        } else {
            swapcontext(waitingContext, cpuContext.get());
        }
    } else {
        currentContext->waitingOnLock = false;
        m.ownerThreadID = currentContext->id;
        m.locked = true;
    }
}

/*
 * Wake up first thread on queue
 */
void cv::signal() {
    cpu::interrupt_guard guard;
    if (!cvWaitingThreads.empty()) {
        std::unique_ptr<ThreadContext> threadCtx = std::move(cvWaitingThreads.front());
        readyThreads.push(std::move(threadCtx));
        cvWaitingThreads.pop();
    }
}

/*
 * Wake up all threads on the queue
 */
void cv::broadcast() {
    cpu::interrupt_guard guard;
    while (!cvWaitingThreads.empty()) {
        std::unique_ptr<ThreadContext> threadCtx = std::move(cvWaitingThreads.front());
        readyThreads.push(std::move(threadCtx));
        cvWaitingThreads.pop();
    }
}

/*
 * Move waiting threads in copy constructor
 */
cv::cv(cv&& other) {
    this->cvWaitingThreads = std::move(other.cvWaitingThreads);
}

/*
 * Move waiting threads in copy constructor
 */
cv& cv::operator=(cv&& other) {
    if (this != &other) {
        this->cvWaitingThreads = std::move(other.cvWaitingThreads);
    }
    return *this;
}