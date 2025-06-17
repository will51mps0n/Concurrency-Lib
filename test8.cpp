#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"
#include <memory>
using std::cout;
using std::endl;




/*
* I modified the Spec test again to check for more general cv and mutex stuff
*/


int count = 0; 


mutex mutex1; // Sorry manos :(
cv cv1;
bool conditional = false;




void loop(uintptr_t arg) {
   auto id = reinterpret_cast<const char*>(arg);


   mutex1.lock();
   cout << id << " started." << endl;


   for (int i = 0; i < 5; i++) {
       cout << id << " iteration " << i << " count=" << count++ << endl;
       mutex1.unlock();
       thread::yield();
       mutex1.lock();
   }


   cout << id << " Finished first spec exec." << endl;
   mutex1.unlock();
}


void cv_waiter(uintptr_t arg) {
   auto id = reinterpret_cast<const char*>(arg);


   mutex1.lock();
   while (!conditional) { 
       cout << id << " waiting on CV." << endl;
       cv1.wait(mutex1);
   }
   cout << id << " Doing after the cv signal, count is currently =" << count << endl;
   mutex1.unlock();
}


void cv_testHarder(uintptr_t arg) {
   auto id = reinterpret_cast<const char*>(arg);


   thread::yield(); 


   mutex1.lock();
   conditional = true;
   // not sure on signal order may need to check the invariant
   cout << id << " Checking the signaling " << endl;
   cv1.signal();
   mutex1.unlock();
}


void cv_broadcaster(uintptr_t arg) {
   auto id = reinterpret_cast<const char*>(arg);


   thread::yield(); 


   mutex1.lock();
   conditional = true;
   cout << id << "Trying broadcast " << endl;
   cv1.broadcast();
   mutex1.unlock();
}


void join_on_self(uintptr_t arg) {
   thread t8(cv_broadcaster, reinterpret_cast<uintptr_t>("Self Thread2"));
   std::cout << "Trying to join on self" << std::endl;


   t8.join();
   std::cout << "Joined on self" << std::endl << std::endl;


}

void parent(uintptr_t arg) {
   thread t1(loop, reinterpret_cast<uintptr_t>("Thread 1"));
   thread t2(loop, reinterpret_cast<uintptr_t>("Thread 2"));
   thread t3(cv_waiter, reinterpret_cast<uintptr_t>("Thread 3"));
   thread t4(cv_waiter, reinterpret_cast<uintptr_t>("Thread 4 "));
   thread t5(cv_testHarder, reinterpret_cast<uintptr_t>("Thread 5 "));


   // Dont think we have called this in the parent / firt function yet. Not sure if it does error check
   thread::yield(); 


   thread t6(cv_broadcaster, reinterpret_cast<uintptr_t>("Epic Thread"));


   thread::yield(); 


   thread t7(join_on_self, reinterpret_cast<uintptr_t>("Self Thread"));
}


int main() {
   cpu::boot(1, parent, 0, false, false, 0);
}
