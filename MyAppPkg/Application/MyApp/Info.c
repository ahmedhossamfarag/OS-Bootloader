#include "Info.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS GetGraphicsInfo(IN EFI_SYSTEM_TABLE* ST, OUT GraphicsInfo** GI){
    EFI_STATUS Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN SizeOfInfo;
    UINT32 ModeNumber;

    Status = ST->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&GraphicsOutput);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    ModeNumber = GraphicsOutput->Mode->Mode;
    Status = GraphicsOutput->QueryMode(GraphicsOutput, ModeNumber, &SizeOfInfo, &Info);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    GraphicsInfo* GInfo;
    Status = ST->BootServices->AllocatePool(EfiBootServicesData, sizeof(GraphicsInfo), (VOID**)&GInfo);
    if(EFI_ERROR(Status)){
        return Status;
    }

    GInfo->FameBufferBase = GraphicsOutput->Mode->FrameBufferBase;
    GInfo->FrameBufferSize = GraphicsOutput->Mode->FrameBufferSize;
    GInfo->Width = Info->HorizontalResolution;
    GInfo->Height = Info->VerticalResolution;
    GInfo->PixelFormat = Info->PixelFormat;
    GInfo->PixelsPerScanLine = Info->PixelsPerScanLine;
    GInfo->RedMask = Info->PixelInformation.RedMask;
    GInfo->GreenMask = Info->PixelInformation.GreenMask;
    GInfo->BlueMask = Info->PixelInformation.BlueMask;

    *GI = GInfo;

    return EFI_SUCCESS;
}

static UINT32 GetRSDP(IN EFI_SYSTEM_TABLE* ST){
    for (UINT32 i = 0; i < ST->NumberOfTableEntries; i++)
    {
        EFI_CONFIGURATION_TABLE CT = ST->ConfigurationTable[i];
        UINT64* Signature = (UINT64*) CT.VendorTable;
        if(*Signature == EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE){
            return (UINT32) CT.VendorTable;
        }
    }
    return 0;
}

EFI_STATUS GetMemoryInfo(IN EFI_SYSTEM_TABLE* ST, OUT MemoryInfo** MI){
    EFI_STATUS Status;
    EFI_MEMORY_DESCRIPTOR *MemoryMap;
    UINTN MemoryMapSize;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;
    UINTN Index;
    EFI_MEMORY_DESCRIPTOR *Descriptor;


    MemoryMapSize = 0;
    MemoryMap = NULL;

    Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, NULL, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        return Status;
    }

    ST->BootServices->AllocatePool(EfiLoaderCode, MemoryMapSize, (VOID**)&MemoryMap);
    if (MemoryMap == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = ST->BootServices->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
        ST->BootServices->FreePool(MemoryMap);
        return Status;
    }

    Descriptor = MemoryMap;
    UINT64 Size = 0;
    for (Index = 0; Index < MemoryMapSize / DescriptorSize; ++Index) {
        Size += Descriptor->NumberOfPages * EFI_PAGE_SIZE;
        Descriptor = (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)Descriptor + DescriptorSize);
    }
    ST->BootServices->FreePool(MemoryMap);

    MemoryInfo* MInfo;
    Status = ST->BootServices->AllocatePool(EfiBootServicesData, sizeof(MemoryInfo), (VOID**)&MInfo);
    if(EFI_ERROR(Status)){
        return Status;
    }

    MInfo->MomorySizeInMB = Size / 1024 / 1024;
    MInfo->RSDP = GetRSDP(ST);

    *MI = MInfo;

    return EFI_SUCCESS;
}