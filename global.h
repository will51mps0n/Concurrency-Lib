#pragma once
#include "thread.h"
#include "cpu.h"
#include <memory>
#include <queue>
#include <set>
#include <ucontext.h>
#include <iostream>

struct ThreadContext {
    static int nextID; 
    int id;
    std::unique_ptr<ucontext_t> context;
    std::shared_ptr<std::queue<std::unique_ptr<ThreadContext>>> threadJoinQueue;
    bool completed;
    bool waitingOnLock;

    // THis is for join: tracks if thread is still tunning
    std::shared_ptr<bool> threadAlive;

    ThreadContext(std::unique_ptr<ucontext_t> ctx)
        : id(nextID++),          
          context(std::move(ctx)),
          completed(false),
          waitingOnLock(false) {

        threadAlive = std::make_shared<bool>(true);
        threadJoinQueue = std::make_shared<std::queue<std::unique_ptr<ThreadContext>>>();
    
    }
};

void threadStarter(thread_startfunc_t func, uintptr_t arg);
void go_to_next_thread();
// void initContext(std::unique_ptr<ucontext_t>& context);

extern std::queue<std::unique_ptr<ThreadContext>> readyThreads;
extern std::unique_ptr<ucontext_t> cpuContext;
extern std::unique_ptr<ThreadContext> currentContext;

extern std::vector<char*> stackPointers;


