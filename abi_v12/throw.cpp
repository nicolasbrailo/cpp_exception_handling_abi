#include <stdio.h>
#include "throw.h"

struct RAII {
    int i;
    RAII(int i) : i(i) { printf("RAII object %i has been built\n", i); }
    ~RAII() { printf("RAII object %i has been destroyed\n", i); }
};

struct Fake_Exception {};

void raise() {
    throw Exception();
}

void try_but_dont_catch() {
    RAII x(1);

    try {
        raise();
    } catch(Fake_Exception&) {
        printf("Caught a Fake_Exception!\n");
    }

    printf("try_but_dont_catch handled the exception\n");
}

void catchit() {
    try {
        try_but_dont_catch();
    } catch(Fake_Exception&) {
        printf("Caught a Fake_Exception!\n");
    } catch(Exception&) {
        printf("Caught an Exception!\n");
    }

    printf("catchit handled the exception\n");
}

extern "C" {
    void seppuku() {
        catchit();
    }
}

