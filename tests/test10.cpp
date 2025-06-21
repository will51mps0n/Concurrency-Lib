
#include "thread.h"
#include "cpu.h"
#include <iostream>
#include <vector>
#include <cassert>

int counter = 0;
std::vector<int> execution_order;


void increment_counter(uintptr_t thread_num) {
   std::cout << "Thread " << thread_num << " starting... epic\n";
   execution_order.push_back(thread_num);
  
   for(int i = 0; i < 3; i++) {
       thread::yield(); 
       counter++;
       std::cout << "Thread " << thread_num << " incremented counter to " << counter << "\n";
   }
  
   std::cout << "Thread " << thread_num << " finishing\n";
}


void join_test(uintptr_t arg) {
   std::cout << "Starting the join test\n";
  
   thread* t1 = new thread(increment_counter, 1);
   thread* t2 = new thread(increment_counter, 2);
   thread* t3 = new thread(increment_counter, 3);
  
   std::cout << "Here means we created three threads\n";
  
   std::cout << "Calling join on t1 before creating the 4th thread\n";
   t1->join();
   std::cout << "Thread 1 done\n";
  
   thread* t4 = new thread(increment_counter, 4);
   std::cout << "SPAWNED thread 4\n";
  
   std::cout << "Waiting for completion of thread 2 - calling join on t2\n";
   t2->join();
   std::cout << "Thread 2 done\n";
  
   std::cout << "Waiting for thread 3 to complete - calling join on 3\n";
   t3->join();
   std::cout << "Thread 3 finished\n";
  
   std::cout << "Waiting for thread 4 to finish...\n";
   t4->join();
   std::cout << "Thread 4 finished\n";
  
   delete t1;
   delete t2;
   delete t3;
   delete t4;
  
   std::cout << "Ensure counter is 12, as each thread should incrememnt 3 times " << counter << "\n";
   assert(counter == 12);
  
   std::cout << "Join test completed successfully!\n";
}


int main() {
   cpu::boot(1, join_test, 0, false, false, 0);
   return 0;
}

