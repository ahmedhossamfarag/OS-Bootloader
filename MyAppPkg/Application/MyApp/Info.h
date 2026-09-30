#include <Uefi.h>
#include <Library/UefiLib.h>

typedef struct
{
    UINT64 FameBufferBase;
    UINT32 FrameBufferSize;
    UINT32 Width;
    UINT32 Height;
    UINT32 PixelFormat;
    UINT32 PixelsPerScanLine;
    UINT32 RedMask;
    UINT32 GreenMask;
    UINT32 BlueMask;                  
} GraphicsInfo;

typedef struct
{
    UINT64 MomorySizeInMB;
    UINT64 RSDP;
} MemoryInfo;

EFI_STATUS GetGraphicsInfo(IN EFI_SYSTEM_TABLE* ST, OUT GraphicsInfo** GI);

EFI_STATUS GetMemoryInfo(IN EFI_SYSTEM_TABLE* ST, OUT MemoryInfo** MI);
