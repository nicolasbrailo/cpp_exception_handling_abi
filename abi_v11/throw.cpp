#include <stdio.h>
#include "throw.h"

struct Fake_Exception {};

void raise() {
    throw Exception();
}

void try_but_dont_catch() {
    try {
        printf("Harmless try\n");
    } catch(...) {
        printf("A harmless try has thrown!\n");
    }

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

