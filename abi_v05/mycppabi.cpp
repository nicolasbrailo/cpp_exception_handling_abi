#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

typedef void (*unexpected_handler)(void);
typedef void (*terminate_handler)(void);

struct __cxa_exception { 
	std::type_info *	exceptionType;
	void (*exceptionDestructor) (void *); 
	unexpected_handler	unexpectedHandler;
	terminate_handler	terminateHandler;
	__cxa_exception *	nextException;

	int			handlerCount;
	int			handlerSwitchValue;
	const char *		actionRecord;
	const char *		languageSpecificData;
	void *			catchTemp;
	void *			adjustedPtr;

	_Unwind_Exception	unwindHeader;
};

void __cxa_throw(void* thrown_exception, struct type_info *tinfo, void (*dest)(void*))
{
    printf("__cxa_throw called\n");

    __cxa_exception *header = ((__cxa_exception *) thrown_exception - 1);
    _Unwind_RaiseException(&header->unwindHeader);

    // __cxa_throw never returns
    printf("no one handled __cxa_throw, terminate!\n");
    exit(0);
}


void __cxa_begin_catch()
{
    printf("begin FTW\n");
}

void __cxa_end_catch()
{
    printf("end FTW\n");
}




struct LSDA_Header {
    uint8_t lsda_start_encoding;
    uint8_t lsda_type_encoding;
    uint8_t lsda_call_site_table_length;
};

struct LSDA_Call_Site_Header {
    uint8_t encoding;
    uint8_t length;
};

struct LSDA_Call_Site {
    
    LSDA_Call_Site(const uint8_t *ptr) {
        cs_start = ptr[0];
        cs_len = ptr[1];
        cs_lp = ptr[2];
        cs_action = ptr[3];
    }

    uint8_t cs_start;
    uint8_t cs_len;
    uint8_t cs_lp;
    uint8_t cs_action;
};

_Unwind_Reason_Code __gxx_personality_v0 (
                     int version, _Unwind_Action actions, uint64_t exceptionClass,
                     _Unwind_Exception* unwind_exception, _Unwind_Context* context)
{
    if (actions & _UA_SEARCH_PHASE)
    {
        printf("Personality function, lookup phase\n");
        return _URC_HANDLER_FOUND;
    } else if (actions & _UA_CLEANUP_PHASE) {
        printf("Personality function, cleanup\n");

        const uint8_t* lsda = (const uint8_t*)
                                    _Unwind_GetLanguageSpecificData(context);

        LSDA_Header *header = (LSDA_Header*)(lsda);
        LSDA_Call_Site_Header *cs_header = (LSDA_Call_Site_Header*)
                                                (lsda + sizeof(LSDA_Header));

        size_t cs_in_table = cs_header->length / sizeof(LSDA_Call_Site);

        // We must declare cs_table_base as uint8, otherwise we risk an
        // unaligned access
        const uint8_t *cs_table_base = lsda + sizeof(LSDA_Header)
                                            + sizeof(LSDA_Call_Site_Header);

        // Go through every entry on the call site table
        for (size_t i=0; i < cs_in_table; ++i)
        {
            const uint8_t *offset = &cs_table_base[i * sizeof(LSDA_Call_Site)];
            LSDA_Call_Site cs(offset);
            printf("Found a CS:\n");
            printf("\tcs_start: %i\n", cs.cs_start);
            printf("\tcs_len: %i\n", cs.cs_len);
            printf("\tcs_lp: %i\n", cs.cs_lp);
            printf("\tcs_action: %i\n", cs.cs_action);
        }

        uintptr_t ip = _Unwind_GetIP(context);
        uintptr_t funcStart = _Unwind_GetRegionStart(context);
        uintptr_t ipOffset = ip - funcStart;


        return _URC_INSTALL_CONTEXT;
    } else {
        printf("Personality function, error\n");
        return _URC_FATAL_PHASE1_ERROR;
    }
}



}

