/*
 * mutex.h -- interface to the mutex class
 *
 * You may add new variables and functions to this class.
 *
 * Do not modify any of the given function declarations.
 */

#pragma once

#include <queue>
#include <memory>
#include "thread.h"
#include "cpu.h"
#include <ucontext.h>
#include "global.h"

class mutex {
public:
    bool locked;
    int ownerThreadID;
    mutex();
    //~mutex();

    void lock();
    void unlock();

    // TODO: had to comment this out for some reason to do make for the test file
    // we havent implemented lock yet so shoudlnt matter
    //mutex.lock();

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    /*
     * Move constructor and move assignment operator.  Implementing these is
     * optional in Project 2.
     */
    mutex(mutex&&);
    mutex& operator=(mutex&&);

    std::queue<std::unique_ptr<ThreadContext>> waitingThreads;
};