#include "throw.h"

extern "C" {
    void seppuku() {
        throw Exception();
    }
}


