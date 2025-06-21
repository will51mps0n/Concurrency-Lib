#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

using std::cout;
using std::endl;

mutex mutex1;
cv cv1;

void other(uintptr_t arg) {
    auto id = reinterpret_cast<char*>(arg);
    cout << id << "is attempting to grab mutex 1. If lock not confirmed, thread is waiting. " << endl;
    mutex1.lock();
    cout << "mutex 1 has been locked by " << id << endl;
    cout << id << "is yielding so other threads can take over " << endl;
    thread::yield();
    cout << id << "CALLING SIGNAL CV 1 " << endl;
    mutex1.unlock();
    cv1.signal();
    cout << id << "Called signal on lock, now will wait " << endl;
    thread::yield();
    cout << id << "is attempting to grab mutex 1. If lock not confirmed, thread is waiting. " << endl;
    mutex1.lock();
}

void parent(uintptr_t arg)
{
    thread t1 (other, reinterpret_cast<uintptr_t>("thread 1"));
    cout << "parent created t1\n";
    mutex1.lock();
    cout << "parent got lock. Waiting on mutex\n";
    cv1.wait(mutex1);
    cout << "Parent has been woken up. Needs to happen after thread 1 announces signal\n";
    mutex1.unlock();
    thread::yield();
    cout << "Parent returned\n";

    cout << "parent trying to get lock\n";
    mutex1.lock();
}

int main()
{
    cpu::boot(1, parent, 0, false, false, 0);
}