#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

namespace __cxxabiv1 {
    struct __class_type_info {
        virtual void foo() {}
    } ti;
}

#define EXCEPTION_BUFF_SIZE 255
char exception_buff[EXCEPTION_BUFF_SIZE];

extern "C" {

void* __cxa_allocate_exception(size_t thrown_size)
{
    printf("alloc ex %i\n", thrown_size);
    if (thrown_size > EXCEPTION_BUFF_SIZE) printf("Exception too big");
    return &exception_buff;
}

void __cxa_free_exception(void *thrown_exception);

#include <unwind.h>
void __cxa_throw(
          void* thrown_exception,
          struct type_info *tinfo,
          void (*dest)(void*))
{
    printf("throw\n");
    // __cxa_throw never returns
    exit(0);
}

} // extern "C"
