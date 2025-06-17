#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include "cpu.h"
#include "mutex.h"
#include "cv.h"
#include "thread.h"
#include <memory>
#include <vector>
#include <queue>

mutex x, y, c;
mutex mu;
mutex mLock;
int threadsExited = 0;
int numThreadsForBool = 3;
int threadsEntered = 0;
cv canEnterBarrier;
cv canExitBarrier;

void func_B(uintptr_t arg);
void func_C(uintptr_t arg);

struct Request {
    int fromAccount;
    int amount;
    Request(int f, int a) : fromAccount(f), amount(a) {}
};

std::vector<std::unique_ptr<mutex>> allLocks;
std::vector<std::queue<Request>> allRequests;

cv transferCV;
mutex transferMutex;
bool transferInProgress = false;

struct Account {
    int ID;
    int money;

    Account(int i, int m) : ID(i), money(m) {}
};

std::vector<Account> allAccounts;

void child1(uintptr_t arg)
{
    for (int i=0; i<1000; i++) {
        if (!(i%100)) {
	    c.lock();
	    std::cout << "child1: " << i << std::endl;
	    c.unlock();
	}
	x.lock();
	y.lock();
	y.unlock();
	x.unlock();
    }
}

void child2(uintptr_t arg)
{
    for (int i=0; i<1000; i++) {
        if (!(i%100)) {
	    c.lock();
	    std::cout << "child2: " << i << std::endl;
	    c.unlock();
	}
	y.lock();
	x.lock();
	x.unlock();
	y.unlock();
    }
}

void transfer_money(int acct1, int acct2, int amount) {
    std::cout << "Account: " << acct1 << " attempting to request " << amount << " from account: " << acct2 << std::endl;
    allLocks[std::min(acct1, acct2)]->lock();
    allLocks[std::max(acct1, acct2)]->lock();
    if(amount <= allAccounts[acct2].money) {
        std::cout << "Completed request of account: " << acct1 << "requesting " << amount << " from account: " << acct2 << std::endl;
        allAccounts[acct1].money += amount;
        allAccounts[acct2].money -= amount;
    } else {
        std::cout << "Request of account: " << acct1 << "requesting " << amount << " from account: " << acct2 << "was denied bc insufficient funds" << std::endl;
    }
    allLocks[acct2]->unlock();
    allLocks[acct1]->unlock();
}

void account_thread(uintptr_t arg) {
    int id = static_cast<int>(arg);
    while(!allRequests[id].empty()) {
        Request &currentRequest = allRequests[id].front();
        allRequests[id].pop();
        transferMutex.lock();
        while(transferInProgress) {
            transferCV.wait(transferMutex);
        }
        transferInProgress = true;
        transfer_money(id, currentRequest.fromAccount, currentRequest.amount);
        transferInProgress = false;
        transferCV.broadcast();
        transferMutex.unlock();
    }
}



void transfers_thread(uintptr_t arg) {
    allLocks.push_back(std::make_unique<mutex>());
    allLocks.push_back(std::make_unique<mutex>());
    allLocks.push_back(std::make_unique<mutex>());
    allLocks.push_back(std::make_unique<mutex>());
    
    allAccounts.push_back(Account(0, 500));
    std::queue<Request> requests;
    requests.push(Request(2,10));
    requests.push(Request(0,100));
    requests.push(Request(3,10));
    allRequests.push_back(requests);

    allAccounts.push_back(Account(1, 10000));
    std::queue<Request> requests1;
    requests.push(Request(3,10));
    requests.push(Request(0,1000));
    allRequests.push_back(requests1);

    allAccounts.push_back(Account(2, 10));
    std::queue<Request> requests2;
    requests.push(Request(1,1000));
    requests.push(Request(0,1));
    allRequests.push_back(requests2);

    allAccounts.push_back(Account(3, 300));
    std::queue<Request> requests3;
    requests.push(Request(1,1000));
    requests.push(Request(3,1));
    allRequests.push_back(requests3);

    thread t1(account_thread, static_cast<uintptr_t>(0));
    thread t2(account_thread, static_cast<uintptr_t>(1));
    thread t3(account_thread, static_cast<uintptr_t>(2));
    thread t4(account_thread, static_cast<uintptr_t>(3));
}

void func_A(uintptr_t arg) {
    thread t1(func_B, 0);
    thread t2(func_C, 0);
    mu.lock();
    std::cout << "A has lock" << std::endl;
    thread::yield();
    mu.unlock();
    std::cout << "A done" << std::endl;
}

void func_B(uintptr_t arg) {
    std::cout << "B starts" << std::endl;
    mu.lock();
    std::cout << "B has lock" << std::endl;
    mu.unlock();
}

void func_C(uintptr_t arg) {
    std::cout << "C starts" << std::endl;
    thread::yield();
    std::cout << "C is back" << std::endl;
    mu.lock();
    std::cout << "C has lock" << std::endl;
    mu.unlock();
}

void test_deadlock(uintptr_t arg) {
    static mutex m1, m2;
    if (arg == 0) {
        m1.lock();
        std::cout << "Thread 0 locked m1, waiting for m2" << std::endl;
        thread::yield();
        m2.lock();
        m2.unlock();
        m1.unlock();
    } else {
        m2.lock();
        std::cout << "Thread 1 locked m2, waiting for m1" << std::endl;
        thread::yield();
        m1.lock();
        m1.unlock();
        m2.unlock();
    }
}

void waitBool(uintptr_t arg) {
    mLock.lock();

    while (threadsExited < numThreadsForBool) {
      canEnterBarrier.wait(mLock);
    }

    threadsEntered++;
    bool last = threadsEntered == numThreadsForBool; 

    while (threadsEntered < numThreadsForBool) {
        canExitBarrier.wait(mLock);
    }

    if (last) {
      threadsExited = 0;
      canExitBarrier.broadcast();
    }
    threadsExited++;
    if (threadsExited == numThreadsForBool) {
       threadsEntered = 0;
       canEnterBarrier.broadcast();
    }

    mLock.unlock();
    std::cout << "Wait bool function done, thread: " << arg << "finished." << std::endl;
  }

void parent(uintptr_t arg) {
    thread t1(func_A, static_cast<uintptr_t>(0));
    thread t2(func_B, static_cast<uintptr_t>(0));
    thread t3(func_C, static_cast<uintptr_t>(0));
    // thread t4(waitBool, static_cast<uintptr_t>(arg));
    // for (int i = 1; i < 10; i++) {
    //     thread t4(waitBool, static_cast<uintptr_t>(arg));
    // }
    std::cout << "Parent thread is joining children" << std::endl;
    t1.join();
    std::cout << "Parent thread is yielding to children" << std::endl;
    thread::yield();
    t2.join();
    std::cout << "Parent thread is done joining children" << std::endl;
    thread::yield();
    thread t5(child1, static_cast<uintptr_t>(0));
    thread t6(child2, static_cast<uintptr_t>(1));
    thread t7 (test_deadlock, static_cast<uintptr_t>(0));
    thread t13 (test_deadlock, static_cast<uintptr_t>(1));
    thread t8(child2, static_cast<uintptr_t>(1));
    thread::yield();
    thread t9(waitBool, static_cast<uintptr_t>(0));
    thread t10(waitBool, static_cast<uintptr_t>(1));
    thread t11(waitBool, static_cast<uintptr_t>(2));
    thread t12(transfers_thread, 0);
}
  
int main() {
    cpu::boot(1, parent, 0, false, false, 0);
}