#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"
#include <string>
/*
 * This exclusevly tests mutex, lock and unlock - multiple locks. Not using cvs, we can just busy wait
 */

using std::cout;
using std::endl;

/* It has 2 locks, the first is acquired in order of thread creation */
/* The next will be acquired by the last person to get lock 1 */
int complete = 0;
mutex mutex1;
mutex mutex2;

const char* cat_copypasta = 
"\n /\\_/\\  \n"
"( o.o ) \n"
" > ^ <  \n";


void brawl_stars(uintptr_t arg) {
    std::cout << "IN BRAWL STARS FUNCTION" << std::endl;
    auto name = reinterpret_cast<char*>(arg);
    std::cout << name << " trying to acquire lock 1..." << std::endl;
    mutex1.lock();
    std::cout << name << " acquired lock! awesome" << std::endl;
    std::cout << "Now that " << name << " has lock, he opens the door. He pets the cat " << cat_copypasta << std:: endl;
    complete++;

    std::cout << name << " now returns the lock, complete count = " << complete << std::endl;
    mutex1.unlock();

    if (complete != 3) {
        thread::yield();
    }

    std::cout << name << " trying to acquire lock 2..." << std::endl;
    mutex2.lock();
    std::cout << name << " acquired lock 2! - theyre now going to yield, but not give it up" << std::endl;
    thread::yield();
    std::cout << name << " unlocks the door, but the the room is empty, so they give up the lock" << std::endl;
    mutex2.unlock();
    complete--;
    std::cout << name << " they now call yield, and dont move on till the last person is done " << complete << std::endl;
    if (complete != 0) {
        thread::yield();
    }
    std::cout << name << "Done with their turn, done with thread" << std::endl;
}

void boot_the_threads(uintptr_t arg)
{
    /* I made the names alphabetical. I could do ints instead but this is more epic */
    thread t1 (brawl_stars, reinterpret_cast<uintptr_t>("Adam"));
    thread t2 (brawl_stars, reinterpret_cast<uintptr_t>("Ethan"));
    thread t3 (brawl_stars, reinterpret_cast<uintptr_t>("Sergey"));
}

int main()
{
    cpu::boot(1, boot_the_threads, 0, false, false, 0);
}