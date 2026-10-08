#include "Paging.h"
#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#define PAGE_SIZE_4K       0x1000ULL
#define PAGE_SIZE_2M       0x200000ULL
#define FOUR_GB            0x100000000ULL

#define PAGE_PRESENT       (1ULL << 0)
#define PAGE_RW            (1ULL << 1)
#define PAGE_PS            (1ULL << 7)

/*
 * PML4[0] -> PDPT
 * PDPT[0..3] -> PD[0..3]
 * Each PD contains 512 x 2MiB pages.
 *
 * 4 PDs x 512 x 2MiB = 4 GiB
 */

typedef UINT64 PAGE_ENTRY;

EFI_STATUS
CreateIdentityPaging4GB (
    OUT EFI_PHYSICAL_ADDRESS  *Pml4Physical
    )
{
    EFI_STATUS Status;

    EFI_PHYSICAL_ADDRESS Pml4Address;
    EFI_PHYSICAL_ADDRESS PdptAddress;
    EFI_PHYSICAL_ADDRESS PdAddress[4];

    PAGE_ENTRY *Pml4;
    PAGE_ENTRY *Pdpt;
    PAGE_ENTRY *Pd[4];

    UINTN I;
    UINTN J;

    //
    // Allocate PML4.
    //
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiLoaderData,
                    1,
                    &Pml4Address
                    );
    if (EFI_ERROR (Status)) {
        return Status;
    }

    //
    // Allocate PDPT.
    //
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiLoaderData,
                    1,
                    &PdptAddress
                    );
    if (EFI_ERROR (Status)) {
        gBS->FreePages (Pml4Address, 1);
        return Status;
    }

    //
    // Allocate four Page Directories.
    //
    for (I = 0; I < 4; I++) {
        Status = gBS->AllocatePages (
                        AllocateAnyPages,
                        EfiLoaderData,
                        1,
                        &PdAddress[I]
                        );

        if (EFI_ERROR (Status)) {
            while (I > 0) {
                I--;
                gBS->FreePages (PdAddress[I], 1);
            }

            gBS->FreePages (PdptAddress, 1);
            gBS->FreePages (Pml4Address, 1);

            return Status;
        }
    }

    //
    // The pages returned by AllocatePages() are physical addresses.
    //
    Pml4 = (PAGE_ENTRY *)(UINTN)Pml4Address;
    Pdpt = (PAGE_ENTRY *)(UINTN)PdptAddress;

    for (I = 0; I < 4; I++) {
        Pd[I] = (PAGE_ENTRY *)(UINTN)PdAddress[I];
    }

    //
    // Clear all paging structures.
    //
    ZeroMem (Pml4, PAGE_SIZE_4K);
    ZeroMem (Pdpt, PAGE_SIZE_4K);

    for (I = 0; I < 4; I++) {
        ZeroMem (Pd[I], PAGE_SIZE_4K);
    }

    //
    // PML4[0] -> PDPT
    //
    Pml4[0] =
        PdptAddress |
        PAGE_PRESENT |
        PAGE_RW;

    //
    // PDPT[0..3] -> four page directories.
    //
    for (I = 0; I < 4; I++) {

        Pdpt[I] =
            PdAddress[I] |
            PAGE_PRESENT |
            PAGE_RW;
    }

    //
    // Each Page Directory contains:
    //
    //     512 x 2 MiB = 1 GiB
    //
    // Four PDs therefore cover:
    //
    //     4 x 1 GiB = 4 GiB
    //
    for (I = 0; I < 4; I++) {

        for (J = 0; J < 512; J++) {

            EFI_PHYSICAL_ADDRESS PhysicalAddress;

            PhysicalAddress =
                ((EFI_PHYSICAL_ADDRESS)I * 0x40000000ULL) +
                ((EFI_PHYSICAL_ADDRESS)J * PAGE_SIZE_2M);

            Pd[I][J] =
                PhysicalAddress |
                PAGE_PRESENT |
                PAGE_RW |
                PAGE_PS;
        }
    }

    //
    // Return physical address of PML4.
    //
    *Pml4Physical = Pml4Address;

    return EFI_SUCCESS;
}