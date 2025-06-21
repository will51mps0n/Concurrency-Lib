#include "cpu.h"
#include "thread.h"
#include "mutex.h"
#include "cv.h"
#include <iostream>
#include <queue>
#include <vector>
#include <map>
#include <fstream>
#include <memory>
#include <limits>
#include <assert.h>

mutex cout_mutex;

struct location_t {
    unsigned int x;
    unsigned int y;
};

std::vector<std::queue<location_t>> customerRequests;

std::queue<location_t> get_requests(unsigned int customerID) {
    return customerRequests[customerID];
}

void driver_ready(unsigned int driver, location_t location) {
    cout_mutex.lock();
    std::cout<<"Driver: " <<  driver << " ready at location (" << location.x << ", " << location.y << ")" << "\n";
    cout_mutex.unlock();
}
void drive(unsigned int driver, location_t start, location_t end) {

}

void customer_ready(unsigned int customer, location_t location) {
    cout_mutex.lock();
    std::cout<<"Customer: " <<  customer << " ready at location (" << location.x << ", " << location.y << ")" << "\n";
    cout_mutex.unlock();
}
void pay(unsigned int customer, unsigned int driver) {
    cout_mutex.lock();
    std::cout<<"Customer: " <<  customer << " payed driver: " << driver << "\n";
    cout_mutex.unlock();
}

void match(unsigned int customer, unsigned int driver) {
    cout_mutex.lock();
    std::cout<<"Customer: " <<  customer << " matched with driver: " << driver << "\n";
    cout_mutex.unlock();
}


struct Order {
    bool driverArrived = false;
    bool customerPayed = false;
    unsigned int customerId;
    unsigned int driverId;
    location_t customerLocation;

    Order(unsigned int customer, unsigned int driver, location_t requestLoc) : customerId(customer),  driverId(driver), customerLocation(requestLoc) {}
};

std::vector<std::shared_ptr<Order>> orders;

struct Customer {
    unsigned int id;
    location_t location;
    std::queue<location_t> requests;

    std::shared_ptr<Order> currentOrder = nullptr;

    Customer() : id(0), location({0, 0}) {}

    Customer(unsigned int customerId, std::queue<location_t> customerRequests) : id(customerId), requests(customerRequests) {location = customerRequests.front();}
};

struct Driver {
    unsigned int id;
    location_t location;
    std::shared_ptr<Order> currentOrder = nullptr;

    Driver(unsigned int i) : id(i), location({0,0}) {}
};

std::deque<Driver*> readyDrivers;
std::vector<std::string> customerFiles;
std::vector<Driver> allDrivers;

unsigned int readyCustomerCount = 0;
unsigned int numCustomers = 0;
unsigned int numDrivers = 0;

mutex m;
cv c;

unsigned int calculate_distance(const location_t& loc1, const location_t& loc2) {
    return std::abs(static_cast<int>(loc1.x) - static_cast<int>(loc2.x)) +
           std::abs(static_cast<int>(loc1.y) - static_cast<int>(loc2.y));
}

void driver_thread(uintptr_t arg) {
    unsigned int driverID = static_cast<unsigned int>(arg);
    Driver& driver = allDrivers[driverID];

    
    while(true) {
        m.lock();

        assert(driver.currentOrder == nullptr);
        driver_ready(driverID, driver.location);
        readyDrivers.push_back(&driver);

        c.broadcast();

        // Wait for customer to match
        while(driver.currentOrder == nullptr) {
            c.wait(m);
        }
        // we haave matched with a customer
        assert(driver.currentOrder);
        assert(driver.currentOrder->driverId == driver.id);


        location_t startLocation = driver.location;
        location_t endLocation = driver.currentOrder->customerLocation;
        // Drive to the customer
        m.unlock();
        drive(driverID, startLocation, endLocation);
        m.lock();
        driver.location = endLocation;
        driver.currentOrder->driverArrived = true;
        
        c.broadcast();
        while(!driver.currentOrder->customerPayed) {
            c.wait(m);
        }
        // driver is all set for next iteration, so just clear the current order so it dies
        driver.currentOrder = nullptr;
        m.unlock();
    }
}

void customer_thread(uintptr_t arg) {
    unsigned int customerID = static_cast<unsigned int>(arg);
    std::queue<location_t> requests = get_requests(customerID);
    Customer customer(customerID, requests);

    while(!customer.requests.empty()) {
        m.lock();
        c.broadcast();
        while(readyCustomerCount > 0) {
            c.wait(m);
        }

        customer_ready(customerID, customer.location);
        readyCustomerCount++;
        c.broadcast();
        while(readyDrivers.empty()) {
            c.wait(m);
        }

        // Find closest match
        unsigned int closestDistance = std::numeric_limits<unsigned int>::max();

        assert(!readyDrivers.empty());
        Driver* closestDriver = nullptr;
        unsigned int initialReadySize = readyDrivers.size();
        for (unsigned int i = 0; i < initialReadySize; i++) {
            Driver* newDriver = readyDrivers.front();
            readyDrivers.pop_front();
            unsigned int distance = calculate_distance(customer.location, newDriver->location);
            if (distance < closestDistance) {
                closestDistance = distance;
                
                if (closestDriver) { 
                    readyDrivers.push_back(closestDriver);
                }
                closestDriver = newDriver;
            } else { 
                readyDrivers.push_back(newDriver);
            }
        }

        std::shared_ptr<Order> sharedOrder = std::make_shared<Order>(customer.id, closestDriver->id, customer.location);
        
        customer.currentOrder = sharedOrder;
        closestDriver->currentOrder = sharedOrder;

        readyCustomerCount --;
        match(customer.id, closestDriver->id);

        c.broadcast();

        while(!customer.currentOrder->driverArrived) {
            c.wait(m);
        }

        pay(customer.id, closestDriver->id);
        customer.currentOrder->customerPayed = true;
        customer.requests.pop();
        if(!customer.requests.empty()) {
            customer.location = customer.requests.front();
        }
        customer.currentOrder = nullptr;
        c.broadcast();
        m.unlock();
    }
    return;

}

void setup() {
    numCustomers = 10;
    numDrivers = 4;
    customerRequests.clear();
    customerRequests.resize(numCustomers);
    customerRequests[0].push({1, 1});
    customerRequests[0].push({1, 1});

    customerRequests[1].push({2, 3});
    customerRequests[1].push({3, 4});

    customerRequests[2].push({10, 10});
    customerRequests[2].push({11, 11});

    customerRequests[3].push({0, 0});
    customerRequests[3].push({20, 20});
    customerRequests[3].push({0, 0});

    customerRequests[4].push({5, 5});
    customerRequests[4].push({5, 5});
    customerRequests[4].push({5, 5});

    customerRequests[5].push({1, 2});

    customerRequests[6].push({2, 2});
    customerRequests[6].push({3, 3});
    customerRequests[6].push({4, 4});
    customerRequests[6].push({5, 5});

    customerRequests[7].push({3, 2});
    customerRequests[7].push({6, 1});
    customerRequests[7].push({3, 2});

    customerRequests[8].push({50, 50});

    customerRequests[9].push({0, 1});
    customerRequests[9].push({0, 1});
}


void begin_threads(uintptr_t arg) {
    for (unsigned int i = 0; i < numDrivers; i++) {
        thread(driver_thread, static_cast<uintptr_t>(i));
    }

    for (unsigned int i = 0; i < numCustomers; i++) {
        thread(customer_thread, static_cast<uintptr_t>(i));
    }
}

int main() {
    setup();

    for (unsigned int i = 0; i < numDrivers; i++) {
        allDrivers.emplace_back(i);
    }

    cpu::boot(1, begin_threads, 0, false, false, 0);
}