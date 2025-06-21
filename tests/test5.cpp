#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <cassert>
#include <queue>
#include <vector>
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

void player(uintptr_t arg) {
    int id = static_cast<int>(arg);
    turn_mutex.lock();
    while (count <= 9) { 
        // turn_mutex.lock();
        
        while (turn != id && count <= 9) {
            c.wait(turn_mutex);
        }
        if (count > 9) {
            break;
        }
        if (id == 0) {
            std::cout << "Ping" << std::endl;
        } else {
            std::cout << "Pong" << std::endl;
        }
        count++;
        turn = 1 - turn;  
        c.signal();
        // turn_mutex.unlock();
    }
    turn_mutex.unlock();
}



void booter(uintptr_t) {
   thread pinger(player, 0);
   thread ponger(player, 1);
   ponger.join();
   std::cout << "all done\n";
}

int main() {
   cpu::boot(1, booter, 0, false, false, 0);
}

