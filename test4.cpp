#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

/*
 * This test tests mutex and cv basic functionality. Tests functionality of parents and locks
 */

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;

void loop(uintptr_t arg)
{
    auto id = reinterpret_cast<char*>(arg);

    mutex1.lock();
    cout << "loop called by " << id << endl;

    int i;
    for (i=0; i<5; i++, g++) {
        cout << id << ":	" << i << "	" << g << endl;
        mutex1.unlock();
        thread::yield();
        mutex1.lock();
    }
    cout << id << ":	" << i << "	" << g << endl;
    mutex1.unlock();
}

void parent(uintptr_t arg)
{
    thread t1 (loop, reinterpret_cast<uintptr_t>("child thread"));
    loop(reinterpret_cast<uintptr_t>("parent thread"));
}

int main()
{
    cpu::boot(1, parent, 0, false, false, 0);
}