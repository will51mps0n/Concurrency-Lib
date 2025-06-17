#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

/*
 * This test tests a join call on a thread that is currently waiting on a lock
 */

using std::cout;
using std::endl;

mutex mutex1;

void other(uintptr_t arg) {
    auto id = reinterpret_cast<char*>(arg);
    cout << id << "is attempting to grab mutex 1. If lock not confirmed, thread is waiting. " << endl;
    mutex1.lock();
    cout << "mutex 1 has been locked by " << id << endl;

    cout << id << "is yielding so other threads can take over " << endl;
    thread::yield();
    cout << id << "unlocking mutex 1 " << endl;
    mutex1.unlock();
    cout << id << "Is done " << endl;

}

void test_join(uintptr_t arg)
{
    thread t2 (other, reinterpret_cast<uintptr_t>("thread 2"));
    thread t3 (other, reinterpret_cast<uintptr_t>("thread 3"));
    std::cout << "Yielding thread 1 so thread 2 can take over execution" << std::endl;
    thread::yield();

    std::cout << "Thread 1 now attempting to join thread 3, which should be waiting on lock" << std::endl;
    t3.join();

    std::cout << "Thread 1 has now moved on after join, so 3 should be done. Test done" << std::endl;


}

void parent(uintptr_t arg)
{
    thread t1 (test_join, reinterpret_cast<uintptr_t>("thread 1"));
}

int main()
{
    cpu::boot(1, parent, 0, false, false, 0);
}