// Ensures we exit with error when a thread unlocks a mutex it doesn't own
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

mutex turn_mutex;
int count = 0;

void oneStarter(uintptr_t arg) {
    //std::cout << count << "\n";
}

void boot(uintptr_t) {
    while (count < 1000) {
        thread one(oneStarter, 0);
        ++count;
        thread::yield();
    }
}

int main() {
    cpu::boot(1, boot, 0, false, false, 0);
}
