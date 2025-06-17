/*
 * thread.h -- interface to the thread library
 *
 * This file should be included by the thread library and by application
 * programs that use the thread library.
 * 
 * You may add new variables and functions to this class.
 *
 * Do not modify any of the given function declarations.
 */

 #pragma once

 #include <cstdint>
 #include <memory>
 #include <ucontext.h>
 #include "global.h"
 #include <queue>
 
 struct ThreadContext;
 
 #if !defined(__cplusplus) || __cplusplus < 201700L
 #error Please configure your compiler to use C++17 or C++20
 #endif
 
 #if !defined(__clang__) && defined(__GNUC__) && __GNUC__ < 11
 #error Please use g++ version 11 or higher
 #endif
 
 static constexpr unsigned int STACK_SIZE=262144;// size of each thread's stack in bytes
 
 using thread_startfunc_t = void (*)(uintptr_t);
 
 class thread {
 public:
     thread(thread_startfunc_t func, uintptr_t arg); // create a new thread
     ~thread();
 
     void join();                                // wait for this thread to finish
 
     static void yield();                        // yield the CPU
 
     /*
      * Disable the copy constructor and copy assignment operator.
      */
     thread(const thread&) = delete;
     thread& operator=(const thread&) = delete;
 
     /*
      * Move constructor and move assignment operator.  Implementing these is
      * optional in Project 2.
      */
    thread(thread&&);
    thread& operator=(thread&&);
    std::unique_ptr<ucontext_t> context;
    std::shared_ptr<bool> isAlive;
    std::shared_ptr<std::queue<std::unique_ptr<ThreadContext>>> threadJoinQueue;
 
 };