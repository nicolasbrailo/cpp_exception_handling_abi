
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

void __cxa_throw(void* thrown_exception,
                 struct type_info *tinfo,
                 void (*dest)(void*))
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




/**********************************************/

/**
 * The LSDA is a read only place in memory; we'll create a typedef for
 * this to avoid a const mess later on; LSDA_ptr refers to readonly and
 * &LSDA_ptr will be a non-const pointer to a const place in memory
 */
typedef const uint8_t* LSDA_ptr;

struct LSDA_Header {
    /**
     * Read the LSDA table into a struct; advances the lsda pointer
     * as many bytes as read
     */
    LSDA_Header(LSDA_ptr *lsda) {
        LSDA_ptr read_ptr = *lsda;

        // Copy the LSDA fields
        start_encoding = read_ptr[0];
        type_encoding = read_ptr[1];
        ttype = read_ptr[2];

        // Advance the lsda pointer
        *lsda = read_ptr + sizeof(LSDA_Header);
    }

    uint8_t start_encoding;
    uint8_t type_encoding;
    uint8_t ttype;
};

struct LSDA_CS_Header {
    // Same as other LSDA constructors
    LSDA_CS_Header(LSDA_ptr *lsda) {
        LSDA_ptr read_ptr = *lsda;
        encoding = read_ptr[0];
        length = read_ptr[1];
        *lsda = read_ptr + sizeof(LSDA_CS_Header);
    }

    uint8_t encoding;
    uint8_t length;
};

struct LSDA_CS {
    // Same as other LSDA constructors
    LSDA_CS(LSDA_ptr *lsda) {
        LSDA_ptr read_ptr = *lsda;
        start = read_ptr[0];
        len = read_ptr[1];
        lp = read_ptr[2];
        action = read_ptr[3];
        *lsda = read_ptr + sizeof(LSDA_CS);
    }

    // Note start, len and lp would be void*'s, but they are actually relative
    // addresses: start and lp are relative to the start of the function, len
    // is relative to start
 
    // Offset into function from which we could handle a throw
    uint8_t start;
    // Length of the block that might throw
    uint8_t len;
    // Landing pad
    uint8_t lp;
    // Offset into action table + 1 (0 means no action)
    // Used to run destructors
    uint8_t action;
};

/**********************************************/


_Unwind_Reason_Code __gxx_personality_v0 (
                             int version,
                             _Unwind_Action actions,
                             uint64_t exceptionClass,
                             _Unwind_Exception* unwind_exception,
                             _Unwind_Context* context)
{
    if (actions & _UA_SEARCH_PHASE)
    {
        printf("Personality function, lookup phase\n");
        return _URC_HANDLER_FOUND;
    } else if (actions & _UA_CLEANUP_PHASE) {
        printf("Personality function, cleanup\n");

        // Pointer to the beginning of the raw LSDA
        LSDA_ptr lsda = (uint8_t*)_Unwind_GetLanguageSpecificData(context);

        // Read LSDA headerfor the LSDA
        LSDA_Header header(&lsda);

        // Read the LSDA CS header
        LSDA_CS_Header cs_header(&lsda);

        // Calculate where the end of the LSDA CS table is
        const LSDA_ptr lsda_cs_table_end = lsda + cs_header.length;

        // Loop through each entry in the CS table
        while (lsda < lsda_cs_table_end)
        {
            LSDA_CS cs(&lsda);

            if (cs.lp)
            {
                int r0 = __builtin_eh_return_data_regno(0);
                int r1 = __builtin_eh_return_data_regno(1);

                _Unwind_SetGR(context, r0, (uintptr_t)(unwind_exception));
                // Note the following code hardcodes the exception type;
                // we'll fix that later on
                _Unwind_SetGR(context, r1, (uintptr_t)(1));

                uintptr_t func_start = _Unwind_GetRegionStart(context);
                _Unwind_SetIP(context, func_start + cs.lp);
                break;
            }
        }

        return _URC_INSTALL_CONTEXT;
    } else {
        printf("Personality function, error\n");
        return _URC_FATAL_PHASE1_ERROR;
    }
}


}
