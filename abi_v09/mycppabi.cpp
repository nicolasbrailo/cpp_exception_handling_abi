
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

    __cxa_exception *header = ((__cxa_exception *) thrown_exception);
    const char* nm = tinfo->name();
    printf("%s\n", nm);

    // We need to save the type info in the exception header _Unwind_ will
    // receive, otherwise we won't be able to know it when unwinding
    header->exceptionType = tinfo;

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

//This function receives a pointer the first byte to be decoded and the address of where to put the decoded value.
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
      // Shifting the byte to the left and propagating the new byte to it (if there's any)
      result |= ((_uleb128_t)byte & 0x7f) << shift;
      shift += 7;
    }
  // if the 8th bit is 1, this means that there still another byte to be concatinated to the current byte (if zero, then decoding is done)
  while (byte & 0x80);
  *val = result;
  return p;
}

// This function recieves a pointer to a ttype entry and return the corrisponding type_info
// It's an implementation of three different functions from unwind-pe.h specific to our case and cannot be generalized for different encoding and size
const std::type_info *
get_ttype_entry (uint8_t* entry)
{
    // Get a pointer to the value of the address inside entry
    const int32_t *u = (const int32_t *) entry;
    unsigned long result;

    // The final address will be the value stored plus the address of the value (pc relative addressing)
    result = *u + (unsigned long)u;
    result = *(unsigned long *)result;
    
    //  Type cast it into a valid type_info
    const std::type_info *tinfo = reinterpret_cast<const std::type_info *>(result);
    return tinfo;
}

struct LSDA_Header {
    /**
     * Read the LSDA table into a struct; advances the lsda pointer
     * as many bytes as read
     */
    LSDA_Header(LSDA_ptr *lsda) {
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
        types_table_start( (const void**)((uint8_t*)raw_lsda + header.type_table_offset) ),

        // Read the LSDA CS header
        cs_header(&raw_lsda),

        // The call site table starts immediatelly after the CS header
        cs_table_start(raw_lsda),

        // Calculate where the end of the LSDA CS table is
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
                int type_index = 4 * action[0];

                // Get the type of the exception we can handle
                uint8_t* catch_type_info = (uint8_t*)lsda.types_table_start;
                catch_type_info -= type_index;
                const std::type_info *catch_ti = get_ttype_entry(catch_type_info);

                // Get the type of the original exception being thrown
                __cxa_exception* exception_header = (__cxa_exception*)(unwind_exception+1) - 1;
                std::type_info *org_ex_type = exception_header->exceptionType;

                printf("%s thrown, catch handles %s\n",
                            org_ex_type->name(),
                            catch_ti->name());

                // Check if the exception being thrown is of the same type
                // than the exception we can handle
                if (org_ex_type->name() != catch_ti->name())
                    continue;
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

        return _URC_INSTALL_CONTEXT;
    } else {
        printf("Personality function, error\n");
        return _URC_FATAL_PHASE1_ERROR;
    }
}

}
