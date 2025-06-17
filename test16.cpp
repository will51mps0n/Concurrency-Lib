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


mutex turn_mutex;
int turn = 0;
int count = 0;
int stopper = 0;
cv c;


void ping(uintptr_t arg) {
    turn_mutex.lock();
    while (count <= 9) { 
        turn = 0;
        std::cout << "Pass" << std::endl;
        c.signal();
        while (turn == 0) {
            c.wait(turn_mutex);
        }
    }
    turn = 0;
    c.broadcast();
    turn_mutex.unlock();
}

void pong(uintptr_t arg) {
    turn_mutex.lock();
    while (count <= 9) {
        turn = 1;
        std::cout << "Dribble R" << std::endl;
        count++;
        c.signal();
        while (turn == 1) {
            c.wait(turn_mutex);
        } 
    }
    turn = 1;
    c.broadcast();
    turn_mutex.unlock();
}
void pong2(uintptr_t arg) {
    turn_mutex.lock();
    while (count <= 9) { 
        turn = 2;
        std::cout << "Dribble L" << std::endl;
        count++;
        c.signal();
        while (turn == 2) {
            c.wait(turn_mutex);
        } 
    }
    turn_mutex.unlock();
}

void trouble(uintptr_t arg) {
    std::cout << "here comes trouble\n";
    turn_mutex.lock();
    std::cout << "in trouble\n";
    thread::yield();
    turn_mutex.unlock();
    std::cout << "out of throuble\n";
}


void boot(uintptr_t) {
    thread troub(trouble, 0);
   thread pinger(ping, 0);
   thread ponger(pong, 1);
   thread ponger2(pong2, 1);
   ponger2.join();
   std::cout << "SHOOT\n";
}

int main() {
    cpu::boot(1, boot, 0, false, false, 0);
 }