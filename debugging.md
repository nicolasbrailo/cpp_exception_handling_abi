### ULEB128 encoding
As mentioned in the blog (prior knowledge of the source code from the blog until v08 is required), the personality function iterates through the LSDA call site table (LSDACS) entries in search for a proper handler. the entries are encoded in ULEB128 formate (for god knows why) and one of the assumptions that sadly wasn't true is that the values will be small enough so that their encoded value will be the same as the original (check [ULEB128 encoding](https://en.wikipedia.org/wiki/LEB128) for more info about the encoding process). The values stored represent relative addresses for possible handlers and while the assumption holds true for this case on a 32 bit machine, the longer machine code in a 64 bit resulted in a higher value for the relative addresses ultimately exceeding 127 (which is the maximum value that has the same encoding as its original) and as result the value read directly from the call site table wasn't the ones we needed and we had to proberly decode them. To solve this issue, a simple decoding function 'read_uleb128' was implemented and called each time a uleb128 entry was accessed. The result is, we can now access all the entries (with the correct values) in the lsda table, but it's never too easy. We still had the segmentation fault error, and the reason this time was another architicture dependant issue.
### Void* size
After finding the correct handler from the call site table, the next step is accessing its related entry in the action table (and we can do this without a problem). The problem happens when we try jumping to the needed entry in the type table (basic understanding of the exception handling flow is required). Entries in type table are of type long and hold addresses to the needed data types (of type 'type_info') to check the thrown exception type. '.long' data type in x86 assembly (mostly anywhere actually) have a size of 4 bytes. The original source code iterated the table with the help of a simple void pointer and some pointer arthematics. ++1-ing a void pointer simply adds its size ('sizeof(void*)' which is 4 byte for 32 bit architicture) to the address contained in the pointer and this is how we access subsequent entries in the table, so far so good. Now when we go to a 64 bit machine, the size of the void* increases to 8 bytes but the entries are still spaced at 4 byte (because the .long size doesn't change with the architicture) and what happens is a troublsome out of bounds case. by modifying the mechanism of iteration (modifying the pointer size) we were able to access the correct address value. Now you would think that by jumping to this address we can have our required 'type_info', little did we know it wasn't that simple.
### Table type entries encoding
In one of the earlier version when we first started reading the lsda, we came across a '.byte' entry that we ignored assuming that it holds unnecessary information about the encoding of the type table. Well, this seamingly innocent single byte value should be masked with 3 different values to get the appropriate size, encoding, and relative address base. Now the size and encoding were not a problem (well we already solved the size issue and the size directives made it clear that this is a 4 byte value). The relative address base on the other hand was wrongfully ignored. It was assumed in the blog that the adress was an absolute address to a location in the assembly code (it was actually if you try it in the same environment). It turns out it's a pc-relative address (relative to the location of the current entry to be exact) after tracing the eh-personality of the libsupc++ library on an 64 bit machine. In order to get the correct address, just add the value found in the type table to the address of the value in the table (this was implemented in 'get_ttype_entry'). And there you have it, v08 now finally works on an x86_64 machine. We were hoping that propagating the changes to v12 would solve everything, but '__cxa_throw' function had another say in the matter.
### __cxa_throw 'thrown_exception' parameter
If you jump all the way up to cxa_throw you will notice that there is a weird -1 (in the old version at least). This -1 moves the '\*header' up by the size of '__cxa_exception'. The only logical reason you would need to do this is if the pointer '\*thrown_exception' that is passed to the function was pointing to the end of the struct and not the begining, but who decides where the pointer is? Well, it's '__cxa_allocate' but in our version it's normally pointing to the begining of the buffer that would be casted later on to a '__cxa_exception' type. So, all that we need is to remove the -1 and all will be good, right? Well actually this indeed solved the problem this time, but (there is always a but) the allocate method in gcc/libsupc++ responsible for exception handling sends the pointer at the end for some reason, so it's a good idea to follow it just in case some unwind function expects the '\*thrown_exception' to point at the end. Get the -1 back and make allocate point to the end of the buffer. And there you have it, your minimal working exception handling ABI.
____
### TTBase_encoding break down
The info stored in @TTBase_encoding can be obtained by applying different masks  

To get the size of the encoded value mask it with 0x07  
|Mask Result  |Size           |
|-------------|---------------|
|0x00         |sizeof(void*)  |
|0x02         |2 bytes        |
|0x03         |4 bytes        |
|0x04         |8 bytes        |

To get the encoding and data type mask it with 0x0f  
|Mask Result  |Type               |
|-------------|-------------------|
|0x00         |void*              |
|0x01         |uleb128            |
|0x09         |sleb128            |
|0x02         |unsigned 2 bytes   |
|0x03         |unsigned 4 bytes   |
|0x04         |unsigned 8 bytes   |
|0x0a         |signed 2 bytes     |
|0x0b         |signed 4 bytes     |
|0x0c         |signed 8 bytes     |

There is a special treatment if the value of @TTBase_encoding is 0x50 (if it's of reference aligned)  

To get the base of the relative address mask it with 0x70
|Mask Result  |Reference          |
|-------------|-------------------|
|0x00         |absolute ptr       |
|0x10         |pc relative        |
|0x50         |aligned*           |
|0x20         |text relative      |
|0x30         |data relative      |
|0x40         |function relative  |

*aligned is still unknown and for know is deemed unnecessary
