# Thread Library

A custom user-level thread library implemented in C++ for an operating system. The library provides fundamental threading primitives such as threads, mutexes, and condition variables, all built to operate on top of a custom CPU simulation environment.

## Features

* Cooperative and preemptive threading via a simulated CPU context
* Implementation of `thread_create`, `thread_yield`, and `thread_join`
* Mutex locking and unlocking with `mutex_lock`, `mutex_unlock`
* Condition variable wait/signal mechanisms with `cv_wait`, `cv_signal`, and `cv_broadcast`
* Tested using over 20 automated test cases and real-world multi-threaded simulations

## Highlight: `pizza.cpp`

An advanced simulation showcasing real-world multithreading involving customer and driver coordination using condition variables and mutexes.

**Scenario:**

* Multiple customers issue delivery requests.
* Drivers wait for assignments.
* Customers are matched with the closest available driver.
* Synchronization is handled entirely via the custom threading primitives implemented in this project.

## Project Layout

```
.
├── cpu.*           # Simulated CPU interface
├── thread.*        # Core thread implementation
├── mutex.*         # Mutex abstraction
├── cv.*            # Condition variables
├── Makefile        # Build system
├── pizza.cpp       # End-to-end multithreaded simulation
├── test*.cpp       # Individual test cases (1 through 23)
├── correct/        # Reference outputs
├── output/         # Test run outputs
├── test.sh         # Batch test runner
```

## Build & Run

```bash
make clean
make all
./test.sh   # runs all tests and checks output
```

To run `pizza.cpp` manually:

```bash
g++ pizza.cpp libthread.o libcpu_macos.o -o pizza -pthread
./pizza
```

> Note: This is designed for Unix-based systems (tested on macOS). Adjust `Makefile` if using Linux.
