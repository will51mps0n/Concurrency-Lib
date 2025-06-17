#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <cassert>
#include "cv.h"
#include<map>
#include <memory>
#include "cpu.h"
#include "thread.h"
#include "mutex.h"


using std::cout;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::unique_ptr;

/*
 * This test deals with having multiple CVS, a parent thread represented as confuctor, locks, unlocks, and signal
 */

/* given helper functions and variables */

enum class Note : uintptr_t { Na = 0, Do = 1, Re = 2, Mi = 3, Fa = 4, So = 5, La = 6, Ti = 7 };
std::vector<std::string> notes_str{"empty", "do", "re", "mi", "fa", "so", "la", "ti"};

std::map<Note, unique_ptr<cv>> pianoCvs;

// Use ptrs because the copy assignment didnt work in last submit >:(
void initPianoCvs() {
    pianoCvs[Note::Do] = std::make_unique<cv>();
    pianoCvs[Note::Re] = std::make_unique<cv>();
    pianoCvs[Note::Mi] = std::make_unique<cv>();
    pianoCvs[Note::Fa] = std::make_unique<cv>();
    pianoCvs[Note::So] = std::make_unique<cv>();
    pianoCvs[Note::La] = std::make_unique<cv>();
    pianoCvs[Note::Ti] = std::make_unique<cv>();
    pianoCvs[Note::Na] = std::make_unique<cv>();
}

mutex noteMutex;
cv conductorCv;

size_t noteIndex = 0;
string noteSequence = "12345671";
Note currentNote = Note::Na;

void play(Note note) {
    assert(note != Note::Na);
    cout << notes_str[static_cast<size_t>(note)] << endl;
}

void conductor(uintptr_t arg) {
    while (noteIndex < noteSequence.length()) {
        std::cout << "conductor in loop\n";
        Note tmpNote = static_cast<Note>(noteSequence[noteIndex++] - '0');
        noteMutex.lock();
        while (currentNote != Note::Na) {
            std::cout << "conductor in wait\n";
            conductorCv.wait(noteMutex);
        }
        std::cout << "conductor signaling\n";
        currentNote = tmpNote;
        pianoCvs[currentNote]->signal(); 
        noteMutex.unlock();
    }
}

void pianoKey(uintptr_t i) {
    Note myNote = static_cast<Note>(i);
    while (true) {
        std::cout << i << " in loop\n";
        noteMutex.lock();
        while (currentNote != myNote) {
            std::cout << i << " in wait\n";
            pianoCvs[myNote]->wait(noteMutex);
        }
        std::cout << i << "\n";
        play(myNote);
        currentNote = Note::Na;
        conductorCv.signal();
        noteMutex.unlock();
    }
}


void manageThreads(uintptr_t arg) {
    initPianoCvs();
    std::cout << "here22\n";
    for (uintptr_t i = 1; i <= 7; ++i) {
        thread pianoKeyThread(pianoKey, i);
    }
    uintptr_t i = 0;
    thread conductorThread(conductor, i);
    std::cout << "here222\n";
}


int main() {
    cpu::boot(1, manageThreads,0, false, false, 0);
}