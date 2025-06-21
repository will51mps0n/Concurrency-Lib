#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

/*
 * This test tests a join call on a thread that is currently waiting on a CV
 */

using std::cout;
using std::endl;

int waitingOnParent = 0;

mutex mutex1;
cv cv1;

void other(uintptr_t arg) {
    auto id = reinterpret_cast<char*>(arg);
    cout << id << " is going to wait on a cv until t1 has attempted to join " << endl;
    mutex1.lock();
    while (waitingOnParent != 1) {
        cv1.wait(mutex1);
    }

    cout << "Thread " << id << " Has been awoken by thread 1" << endl;
    mutex1.unlock();
    cout << id << " is done " << endl;

}

void test_join(uintptr_t arg)
{
    thread t2 (other, reinterpret_cast<uintptr_t>("thread 2"));
    std::cout << "Yielding thread 1 so thread 2 can take over execution" << std::endl;
    thread::yield();

    std::cout << "Thread 1 now going to attempt to join t2. It will grab the lock, then signal, then release the lock. t2 should wait on mutex because hasnt incrememnted wait on parent count yet" << std::endl;
    waitingOnParent++;
    mutex1.lock();
    cv1.signal();
    cv1.signal();
    cv1.broadcast();
    mutex1.unlock();
    
    t2.join();

    std::cout << "Thread 1 has now moved on after join, so 2 should be done. Test done" << std::endl;


}

void parent(uintptr_t arg)
{
    thread t1 (test_join, reinterpret_cast<uintptr_t>("thread 1"));
}

int main()
{
    cpu::boot(1, parent, 0, false, false, 0);
}