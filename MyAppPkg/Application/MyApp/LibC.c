#include "LibC.h"

VOID MemCopy(CHAR8 *from, CHAR8 *to, UINT32 size)
{
   if(to < from){
        __asm__ __volatile__ (
            "cld\n\t"                      // Clear the direction flag to ensure forward copying
            "rep movsd\n\t"                // Copy 32-bit chunks
            "mov %3, %%ecx\n\t"            // Remaining bytes count
            "rep movsb"                    // Copy remaining bytes
            : /* No output operands */
            : "D" (to), "S" (from), "c" (size / 4), "r" (size % 4)
            : "memory"
        );
   }else if(to > from){
        __asm__ __volatile__ (
            "std\n\t"                      // Set the direction flag to ensure backward copying
            "rep movsb\n\t"                    // Copy remaining bytes
            "cld"
            : /* No output operands */
            : "D" (to + size - 1), "S" (from + size - 1), "c" (size)
            : "memory"
        );
   }
}

CHAR8 *MemSet(CHAR8 *dest, CHAR8 val, UINT32 count) {
    asm volatile (
        "cld\n\t"            // Clear the direction flag to ensure forward direction
        "rep stosb"          // Repeat the `stosb`(store string byte) instruction `count` times
        :                    // No output operands
        : "a" (val), "D" (dest), "c" (count)  // Input operands: AL (value), EDI (destination), and ECX (count)
        : "memory"           // Clobber list: memory
    );
    return dest;
}