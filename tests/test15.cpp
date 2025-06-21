#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <cassert>

#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"

/*
 * This test tests the switching of threads and ordering. Fair ordering, mutex, cv, and global variable switching functionality
 */

using std::cout;
using std::endl;

mutex turn_mutex;
int turn = 0;
int count = 0;
cv c;

void ping(uintptr_t arg) {
    //int id = static_cast<int>(arg);
    while (count <= 9) { 
        turn_mutex.lock();
        std::cout << "Ping" << std::endl;
        count++; 
        thread::yield();
        turn_mutex.unlock();
    }
}

void net(uintptr_t arg) {
    //int id = static_cast<int>(arg);
    while (count <= 9) {
        turn_mutex.lock();
        if(count % 4 == 0) std::cout << "Hit net\n";
        thread::yield();
        turn_mutex.unlock();
    }
}

void pong(uintptr_t arg) {
    //int id = static_cast<int>(arg);
    while (count <= 9) { 
        turn_mutex.lock();
        std::cout << "Pong" << std::endl;
        count++; 
        thread::yield();
        turn_mutex.unlock();
    }
}


void boot(uintptr_t) {
   thread pinger(ping, 0);
   thread ponger(pong, 1);
   thread netter(net, 2);
}

int main() {
   cpu::boot(1, boot, 0, false, false, 0);
}