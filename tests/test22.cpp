#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

mutex m;
cv c;

// This thread tries to lock mutex it doesnt own
void waitingThread(uintptr_t arg) {
    auto id = reinterpret_cast<char*>(arg);
    
    cout << id << ": Starting execution" << endl;
    cout << id << ": About to unlock m when we dont own it" << endl;
    try {
        m.unlock();
        cout << "\n\nin try statement it waited on m : WE DONT WANT THIS TO PRINT!\n\n";
    }
    catch(std::runtime_error &e){
        cout << "try was caught\n";
    }
    
    cout << id << ": After wait attempt with unlocked mutex" << endl;
    cout << id << ": Waiting thread finishing" << endl;
}

void normalThread(uintptr_t arg) {
    auto id = reinterpret_cast<char*>(arg);
    
    cout << id << ": Starting execution - waking up c waiting threads" << endl;
    c.broadcast();
    cout << id << ": Normal thread finishing" << endl;
}

void parent(uintptr_t arg) {
    cout << "Parent: Starting execution" << endl;
       
    cout << "Creating normal thread" << endl;
    thread normal(normalThread, reinterpret_cast<uintptr_t>("NormalThread"));
    
    thread::yield();


    cout << "Locking mutex" << endl;
    m.lock();
    cout << "Creating waiting thread, with m locked. waitng thread will try to call wait on m" << endl;
    thread waiting(waitingThread, reinterpret_cast<uintptr_t>("WaitingThread"));
    
    thread::yield();
    cout << "Waiting for threads to complete" << endl;
    
    cout << "Test completed" << endl;
}

int main() {
    cout << "Main" << endl;
    cpu::boot(1, parent, 0, false, false, 0);
    return 0;
}