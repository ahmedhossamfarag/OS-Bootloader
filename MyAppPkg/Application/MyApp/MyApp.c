#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Protocol/SimpleFileSystem.h>
#include <Library/UefiBootServicesTableLib.h>

#include "Elf32.h"
#include "Info.h"

#define FILE_NPAGES 64
#define LOADER_GUID 0x12345678

EFI_STATUS KernelLoad(IN EFI_SYSTEM_TABLE * ST, OUT EFI_PHYSICAL_ADDRESS* KernelEntry){
    EFI_STATUS Status;

    CHAR16* FileName = L"kernel.o";

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
    EFI_FILE_PROTOCOL* Root;
    EFI_FILE_PROTOCOL* File;
    UINT32 FileSize = FILE_NPAGES * 1024;
    CHAR8* Buffer;
    Elf32_Map map;
    EFI_PHYSICAL_ADDRESS Kernel;
    BOOLEAN Load_Success = FALSE;

    Status = ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderCode, FILE_NPAGES,(EFI_PHYSICAL_ADDRESS*) &Buffer);
    if(EFI_ERROR(Status))
      return Status;

    Status = ST->BootServices->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (VOID**)&FileSystem);

    if (!EFI_ERROR(Status)){
      Status = FileSystem->OpenVolume(FileSystem, &Root);
      if(!EFI_ERROR(Status)){
        Status = Root->Open(Root, &File, FileName, EFI_FILE_MODE_READ, 0);
        if(!EFI_ERROR(Status)){
          Status = File->Read(File, &FileSize, Buffer);
          if(!EFI_ERROR(Status)){
            if(Elf32GetMap(&map, (CHAR8*)Buffer)){
              if(Efl32CheckSupported(map.ehdr) && Elf32CheckExecutabel(map.ehdr)){
                if(map.nphdr){
                  Kernel = map.phdr[0].p_vaddr;
                  Status = ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, FILE_NPAGES, &Kernel);
                  if(!EFI_ERROR(Status)){
                    Elf32LoadFile(&map, 0);
                    *KernelEntry = map.ehdr->e_entry;
                    Load_Success = TRUE;
                  }
                }
              }
            }
          }
          File->Close(File);
        }
        Root->Close(Root);
      }
    }

    if(!Load_Success){
      ST->BootServices->FreePages((UINT32)Buffer, FILE_NPAGES);
    }

    return Load_Success? EFI_SUCCESS : EFI_UNSUPPORTED;
}

VOID Handoff(IN EFI_SYSTEM_TABLE* ST, IN EFI_PHYSICAL_ADDRESS KernelEntry){
      EFI_STATUS Status;
      GraphicsInfo* GI;
      MemoryInfo* MI;

      Status = GetGraphicsInfo(ST, &GI);
      if(EFI_ERROR(Status)){
        GI = 0;
      }

      Status = GetMemoryInfo(ST, &MI);
      if(EFI_ERROR(Status)){
        MI = 0;
      }

      asm("cli\n\t"
          "mov $0x1, %%ebx\n\t"
          "mov %%ebx, %%cr0\n\t"
          "xor %%ebx, %%ebx\n\t"
          "mov %%ebx, %%cr4\n\t"
          "mov %%ebx, %%cr3\n\t"
          "call %0"
          ::   "m"(KernelEntry), "a"(LOADER_GUID), "c"(MI), "d"(GI)
      );
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    EFI_SYSTEM_TABLE* ST = SystemTable;

    ST->ConOut->SetAttribute(ST->ConOut, EFI_BACKGROUND_CYAN);

    ST->ConOut->ClearScreen(ST->ConOut);

    ST->ConOut->EnableCursor(ST->ConOut, TRUE);

    EFI_PHYSICAL_ADDRESS KernelEntry;
    Status = KernelLoad(ST, &KernelEntry);

    if(EFI_ERROR(Status)){
      Print(L"Failed To Load Kernel");
    }else{
      Print(L"Kerenel Loaded Successfully\n");
      Handoff(ST, KernelEntry);      
    }

    Status = ST->ConIn->Reset(ST->ConIn, FALSE);
    if (EFI_ERROR(Status))
        return Status;

    while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ;

    return Status;
}
