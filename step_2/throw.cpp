#include "throw.h"
#include <stdio.h>

// Notice we're adding a second exception type
struct Fake_Exception {};

void raise() {
    throw Exception();
}

// We will analyze what happens if a try block doesn't catch an exception
void try_but_dont_catch() {
    try {
        raise();
    } catch(Fake_Exception&) {
        printf("Running try_but_dont_catch::catch(Fake_Exception)\n");
    }

    printf("try_but_dont_catch handled an exception and resumed execution");
}

// And also what happens when it does
void catchit() {
    try {
        try_but_dont_catch();
    } catch(Exception&) {
        printf("Running try_but_dont_catch::catch(Exception)\n");
    } catch(Fake_Exception&) {
        printf("Running try_but_dont_catch::catch(Fake_Exception)\n");
    }

    printf("catchit handled an exception and resumed execution");
}

extern "C" {
    void seppuku() {
        catchit();
    }
}

