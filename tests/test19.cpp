#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

int count = 0;
mutex countMutex;
mutex printMutex;
cv countCv;

// I made our prints into a fucntion to simplify the cout mutex. Didnt do this for old tests but did for pizza so thought it may get more bugs or something
void print(const std::string& msg) {
    printMutex.lock();
    cout << msg << endl;
    printMutex.unlock();
}

void incrementer(uintptr_t arg) {
    int id = (int)arg;
    std::string name = "Thread " + std::to_string(id);
    
    print(name + ": begining its execution");
    
    // Incr the shared global counter 3 times
    for (int i = 0; i < 3; i++) {
        print(name + ": Attempting to lock countMutex");
        countMutex.lock();
        print(name + ": Got countMutex lock");
        
        int temp = count;
        thread::yield();  
        temp++;
        count = temp;
        
        print(name + ": Incremented count to " + std::to_string(count));
        
        // Signal waiting
        countCv.signal();
        print(name + ": Signaled cv");
        
        countMutex.unlock();
        print(name + ": Released countMutex");
        
        // yield and others can run
        thread::yield();
        print(name + ": returned from yield");
    }
    
    print(name + ": Done with increments");
}

void watcher(uintptr_t arg) {
    int target = (int)arg;
    std::string name = "watch";
    
    print(name + ": Starting execution, waiting for the count to reach specified target = " + std::to_string(target));
    
    countMutex.lock();
    print(name + ": got the countMutex lock!!!");
    
    // Wait until counter reaches target
    while (count < target) {
        print(name + ": Count is " + std::to_string(count) + 
              " < " + std::to_string(target) + ", must wait on cv and relinquish countMutex");
        
        countCv.wait(countMutex);
        print(name + ": Woke up from wait, count is now " + std::to_string(count));
    }
    
    print(name + ": Target reached and now da count = " + std::to_string(count));
    countMutex.unlock();
    print(name + ": released countMutex");
}

void joinTester(uintptr_t arg) {
    std::string name = "JoinTestr";
    print(name + ": creating threads to join");
    
    // create threads and then join them
    thread t1(incrementer, 10);
    thread t2(incrementer, 20);
    
    print(name + ": Created threads, now yielding");
    thread::yield();
    
    print(name + ": Back from yield, joining thread 1");
    t1.join();
    print(name + ": Joined thread 1, now joining thread 2");
    t2.join();
    print(name + ": Joined thread 2, join test complete");
}

void threadCreator(uintptr_t arg) {
    std::string name = "ThreadCreator";
    print(name + ": Starting execution");
    
    // Give birth to the other threads awe babies
    print(name + ": Creating worker threads");
    thread inc1(incrementer, 1);
    thread inc2(incrementer, 2);
    thread inc3(incrementer, 3);
    
    
    print(name + ": Creating watcher thread");
    thread watch(watcher, 8);
    
    print(name + ": Creating join tester thread");
    thread joiner(joinTester, 0);
    
    print(name + ": Yielding to let other threads run");
    thread::yield();
    
    print(name + ": Joining work threads");
    inc1.join();
    inc2.join();
    inc3.join();
    
    print(name + ": Joining the watch thread");
    watch.join();
    
    print(name + ": Joining join test thread");
    joiner.join();
    
    print(name + ": all threads joined, test complete!");
}

void parent(uintptr_t arg) {
    cout << "Parent: Starting the execution of all threads" << endl;
    
    thread t1(threadCreator, 0);
    
    cout << "Parent created threadCreator thread, which will have lots of kids. Now we attempt to join it" << endl;
    t1.join();
    
    cout << "Parent joined creator thread, all tests complete" << endl;
    cout << "Final count value: " << count << endl;
}

int main() {
    cout << "In main, Booting CPU" << endl;
    cpu::boot(1, parent, 0, false, false, 0);
    return 0;
}