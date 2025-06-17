#include <iostream>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"
#include <vector>
#include <queue>
/**** CURENTTLY this test doesnt work. But its probably because of how i wrote it  */
/*
 * This is an exhaustive test, inspired by the lab on deadlocks. This will use a "Order Mutex" that acts as a master mutex, to prevent dead locks rather than doing lock ordering
 * In this test, there are a certain amount of bankers, who will handle transferring money for people. People can also request money and the banker can collect money.
 * Some people will be broke, some people will have money.... etc.
 * Bankers can only help one person at a time, 
 */

 // This first is the mutex to prevent deadlock?? I think itll work
mutex orderMutex;
cv requestCV;
cv bankerCV;
mutex customerMutex[5]; 

struct Request {
    unsigned int fromID;
    unsigned int toID;
    int amount;
    bool fromBank;
};

struct Customer {
    unsigned int accountNumber;
    int balance;
    int amountOfRequests;
    int credit;
    std::queue<Request> requestQueue;

    Customer(unsigned int acc, int bal, int req, int cred) 
    : accountNumber(acc), balance(bal), amountOfRequests(req), credit(cred) {}
};


struct Banker {
    int id;
    bool available;
};

std::vector<Customer> allCustomers;
std::queue<Request> customerRequests;
std::vector<Banker> allBankers;

void banker_thread(uintptr_t arg) {
    auto id = static_cast<int>(arg);
    Banker& banker = allBankers[id];

    while (true) {

        orderMutex.lock();
        std::cout << "Banker " << id << " is ready to complete a request " << std::endl;
        while (customerRequests.empty()) {
            bankerCV.wait(orderMutex);
        }
        std::cout << "Banker " << id << " has accepted a request and is now not available " << std::endl;
        banker.available = false;
        
        Request req = customerRequests.front();
        customerRequests.pop();
        orderMutex.unlock();
        
        if (req.fromBank) {
            customerMutex[req.toID].lock();

            allCustomers[req.toID].balance += req.amount;
            std::cout << "banker:  " << id << " finished customers request  " << req.toID << " ($" << req.amount << ")\n";
            customerMutex[req.toID].unlock();

        } else {
            // should be okay if order mutex is locked. i think
            customerMutex[req.fromID].lock();
            customerMutex[req.toID].lock();
            if (allCustomers[req.fromID].balance >= req.amount) {
                allCustomers[req.fromID].balance -= req.amount;
                allCustomers[req.toID].balance += req.amount;

                std::cout << "Banker " << id << " transferred $" << req.amount << " from Customer " << req.fromID << " to the reciever:  " << req.toID << "\n";
            } else {
                std::cout << "Banker " << id << " Customer " << req.fromID << " is a broke ahh and got request denied " << " ($" << req.amount << ")\n";
            }
            customerMutex[req.toID].unlock();
            customerMutex[req.fromID].unlock();
        }
        
        orderMutex.lock();
        banker.available = true;
        requestCV.signal();
        orderMutex.unlock();
    }
}

void customer_thread(uintptr_t arg) {
    auto accountNumber = static_cast<unsigned int>(arg);
    int completedRequests = 0;
    Customer& customer = allCustomers[accountNumber];
    
    while (completedRequests < customer.amountOfRequests) {
        orderMutex.lock();
        std::cout << "customer " << accountNumber << " is sending a new request " << std::endl;

        if (!customer.requestQueue.empty()) {
            customerRequests.push(customer.requestQueue.front());
            customer.requestQueue.pop();
            bankerCV.signal();
        }
        std::cout << "customer " << accountNumber << " has completed request, their completed request count is:  " << completedRequests << std::endl;
        orderMutex.unlock();
        completedRequests++;
    }
}

void parent(uintptr_t arg) {
    int numBankers = 2;
    int numCustomers = 5;
    
    // this is custoemrs values and requests we can add or change these
    allCustomers.push_back(Customer(0, 500, 2, 0));
    allCustomers.push_back(Customer(1, 300, 2, 100));
    allCustomers.push_back(Customer(2, 100, 1, 50));
    allCustomers.push_back(Customer(3, 0, 3, 200));
    allCustomers.push_back(Customer(4, 700, 2, 0));
    
    std::vector<Request> initialRequests = {
        {0, 1, 50, false},
        {1, 2, 150, false},
        {2, 3, 50, false},
        {3, 4, 200, false},
        {4, 0, 300, false},
        {3, 3, 100, true},
        {2, 2, 200, true}
    };
    
    for (auto& req : initialRequests) {
        allCustomers[req.fromID].requestQueue.push(req);
    }
    
    for (int i = 0; i < numBankers; i++) {
        allBankers.push_back({i, true});
        thread(banker_thread, static_cast<uintptr_t>(i));
    }
    for (int i = 0; i < numCustomers; i++) {
        thread(customer_thread, static_cast<uintptr_t>(i));
    }
}

int main() {
    cpu::boot(1, parent, 0, false, false, 0);
}

