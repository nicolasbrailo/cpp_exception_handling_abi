
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
#include <typeinfo>

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
                 std::type_info *tinfo,
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
typedef _uleb128_t LSDA_line;

const unsigned char *
read_uleb128 (const unsigned char *p, _uleb128_t *val)
{
  unsigned int shift = 0;
  unsigned char byte;
  _uleb128_t result;
  result = 0;
  do
    {
      byte = *p++;
      result |= ((_uleb128_t)byte & 0x7f) << shift;
      shift += 7;
    }
  while (byte & 0x80);
  *val = result;
  return p;
}

// This function recieves a pointer to a ttype entry and return the corrisponding type_info
// It's an implementation of three different functions from unwind-pe.h specific to our case and cannot be generalized for different encoding and size
const std::type_info *
get_ttype_entry (uint8_t* entry)
{
    const int32_t *u = (const int32_t *) entry;
    unsigned long result;

    result = *u + (unsigned long)u;
	entry += 4;
    result = *(unsigned long *)result;
    
    const std::type_info *tinfo = reinterpret_cast<const std::type_info *>(result);
    return tinfo;
}

struct LSDA_Header {
    /**
     * Read the LSDA table into a struct; advances the lsda pointer
     * as many bytes as read
     */
    LSDA_Header(LSDA_ptr *lsda) {
        // LSDA_ptr read_ptr = *lsda;

        // // Copy the LSDA fields
        // start_encoding = read_ptr[0];
        // type_encoding = read_ptr[1];
        // type_table_offset = read_ptr[2];

        // // Advance the lsda pointer
        // *lsda = read_ptr + sizeof(LSDA_Header);

        // Modified version of read for compatability with decodoing function
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        start_encoding = read_ptr[0];
        type_encoding = read_ptr[1];
        read_ptr += 2;
        read_ptr = read_uleb128(read_ptr, &type_table_offset);
        *lsda = (LSDA_ptr)read_ptr;
    }

    uint8_t start_encoding;
    uint8_t type_encoding;

    // This is the offset, from the end of the header, to the types table
    LSDA_line type_table_offset;
};

struct LSDA_CS_Header {
    // Same as other LSDA constructors
    LSDA_CS_Header(LSDA_ptr *lsda) {
        // LSDA_ptr read_ptr = *lsda;
        // encoding = read_ptr[0];
        // length = read_ptr[1];
        // *lsda = read_ptr + sizeof(LSDA_CS_Header);

        // Modified version of read for compatability with decodoing function
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        encoding = read_ptr[0];
        read_ptr += 1;
        read_ptr = read_uleb128(read_ptr, &length);
        *lsda = (LSDA_ptr)read_ptr;
    }

    uint8_t encoding;
    LSDA_line length;
};

struct LSDA_CS {
    // Same as other LSDA constructors
    LSDA_CS(LSDA_ptr *lsda) {
        // LSDA_ptr read_ptr = *lsda;
        // start = read_ptr[0];
        // len = read_ptr[1];
        // lp = read_ptr[2];
        // action = read_ptr[3];
        // *lsda = read_ptr + sizeof(LSDA_CS);

        // Modified version of read for compatability with decodoing function
        const unsigned char *read_ptr = (const unsigned char *)*lsda;
        read_ptr = read_uleb128(read_ptr, &start);
        read_ptr = read_uleb128(read_ptr, &len);
        read_ptr = read_uleb128(read_ptr, &lp);
        read_ptr = read_uleb128(read_ptr, &action);
        *lsda = (LSDA_ptr)read_ptr;
    }

    LSDA_CS() { }

    // Note start, len and lp would be void*'s, but they are actually relative
    // addresses: start and lp are relative to the start of the function, len
    // is relative to start
 
    // Offset into function from which we could handle a throw
    LSDA_line start;
    // Length of the block that might throw
    LSDA_line len;
    // Landing pad
    LSDA_line lp;
    // Offset into action table + 1 (0 means no action)
    // Used to run destructors
    LSDA_line action;
};

/**
 * A class to read the language specific data for a function
 */
struct LSDA
{
    LSDA_Header header;

    // The types_table_start holds all the types this stack frame
    // could handle (this table will hold pointers to struct
    // type_info so this is actually a pointer to a list of ptrs
    const void** types_table_start;

    // With the call site header we can calculate the lenght of the
    // call site table
    LSDA_CS_Header cs_header;

    // A pointer to the start of the call site table
    const LSDA_ptr cs_table_start;

    // A pointer to the end of the call site table
    const LSDA_ptr cs_table_end;

    // A pointer to the start of the action table, where an action is
    // defined for each call site
    const LSDA_ptr action_tbl_start;

    LSDA(LSDA_ptr raw_lsda) :
        // Read LSDA header for the LSDA, advance the ptr
        header(&raw_lsda),

        // Get the start of the types table (it's actually the end of the
        // table, but since the action index will hold a negative index
        // for this table we can say it's the beginning
        // modified the pointer size for pointer arthemitics
        types_table_start( (const void**)((uint8_t*)raw_lsda + header.type_table_offset) ),

        // Read the LSDA CS header
        cs_header(&raw_lsda),

        // The call site table starts immediatelly after the CS header
        cs_table_start(raw_lsda),

        // Calculate where the end of the LSDA CS table is
        // Pointer Arth. has been changed to accomodate the lareger pointer type
        cs_table_end((const LSDA_ptr)((uint8_t*)(raw_lsda) + cs_header.length)),

        // Get the start of action tables
        action_tbl_start( cs_table_end )
    {
    }
   

    LSDA_CS next_cs_entry;
    LSDA_ptr next_cs_entry_ptr;

    const LSDA_CS* next_call_site_entry(bool start=false)
    {
        if (start) next_cs_entry_ptr = cs_table_start;

        // If we went over the end of the table return NULL
        if (next_cs_entry_ptr >= cs_table_end)
            return NULL;

        // Copy the call site table and advance the cursor by sizeof(LSDA_CS).
        // We need to copy the struct here because there might be alignment
        // issues otherwise
        next_cs_entry = LSDA_CS(&next_cs_entry_ptr);

        return &next_cs_entry;
    }
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

        // Calculate what the instruction pointer was just before the
        // exception was thrown for this stack frame
        uintptr_t throw_ip = _Unwind_GetIP(context) - 1;

        // Get a pointer to the raw memory address of the LSDA
        LSDA_ptr raw_lsda = (LSDA_ptr) _Unwind_GetLanguageSpecificData(context);

        // Create an object to hide some part of the LSDA processing
        LSDA lsda(raw_lsda);

        // Go through each call site in this stack frame to check whether
        // the current exception can be handled here
        /* 
         * This loop produces a segmentation fault error due to accesing a wrong memory location
         * This happens probably because the LSDA entries are read without proper decoding so until we fix it, it will remain preceded by these nice //
         */
        for(const LSDA_CS *cs = lsda.next_call_site_entry(true);
                cs != NULL;
                cs = lsda.next_call_site_entry())
        {
            // If there's no landing pad we can't handle this exception
            if (not cs->lp) continue;

            uintptr_t func_start = _Unwind_GetRegionStart(context);

            // Calculate the range of the instruction pointer valid for this
            // landing pad; if this LP can handle the current exception then
            // the IP for this stack frame must be in this range
            uintptr_t try_start = func_start + cs->start;
            uintptr_t try_end = func_start + cs->start + cs->len;

            // Check if this is the correct LP for the current try block
            if (throw_ip < try_start) continue;
            if (throw_ip > try_end) continue;

            // Get the offset into the action table for this LP
            if (cs->action > 0)
            {
                // cs->action is the offset + 1; that way cs->action == 0
                // means there is no associated entry in the action table
                const size_t action_offset = cs->action - 1;
                const LSDA_ptr action = lsda.action_tbl_start + action_offset;

                // For a landing pad with a catch the action table will
                // hold an index to a list of types
                // the 4 is the size of the .long data that stores types in ttable
                int type_index = 4 * action[0];

                uint8_t* catch_type_info = (uint8_t*)lsda.types_table_start;
                catch_type_info -= type_index;
                // void** catch_type_info_void = (void**)catch_type_info_byte;
                // const void* catch_type_info = (const void*)(catch_type_info_void[0]);
                const std::type_info *catch_ti = get_ttype_entry(catch_type_info);
                printf("%s\n", catch_ti->name());
            }

            // We found a landing pad for this exception; resume execution
            int r0 = __builtin_eh_return_data_regno(0);
            int r1 = __builtin_eh_return_data_regno(1);

            _Unwind_SetGR(context, r0, (uintptr_t)(unwind_exception));

            // Note the following code hardcodes the exception type;
            // we'll fix that later on
            _Unwind_SetGR(context, r1, (uintptr_t)(1));

            _Unwind_SetIP(context, func_start + cs->lp);
            break;
        }

        // // This is used to print the values inside the headers
        // printf("LSDA Header:\n");
        // printf("\tstart_encoding: %i\n", lsda.header.start_encoding);
        // printf("\ttype_encoding: %i\n", lsda.header.type_encoding);
        // printf("\ttype_table_offset: %i\n", lsda.header.type_table_offset);

        // printf("LSDA Call Site Header:\n");
        // printf("\tencoding: %i\n", lsda.cs_header.encoding);
        // printf("\tlength: %i\n", lsda.cs_header.length);

        // //Now this one is a cute little for loop for accessing the Call Site table entries for debugging
        // int i = 0;
        // for (const LSDA_CS *cs = lsda.next_call_site_entry(true);
        //     cs != NULL;
        //     cs = lsda.next_call_site_entry())
        // {
        //     printf("Found a CS #%i:\n", i);
        //     printf("\tcs_start: %i\n", cs->start);
        //     printf("\tcs_len: %i\n", cs->len);
        //     printf("\tcs_lp: %i\n", cs->lp);
        //     printf("\tcs_action: %i\n", cs->action);
        //     i++;
        // }

        return _URC_INSTALL_CONTEXT;
    } else {
        printf("Personality function, error\n");
        return _URC_FATAL_PHASE1_ERROR;
    }
}

}
