#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS
CreateIdentityPaging4GB (
    OUT EFI_PHYSICAL_ADDRESS  *Pml4Physical
    );

