#include <Uefi.h>
#include <Library/UefiLib.h>

typedef struct
{
    UINT64 FameBufferBase;
    UINT64 FrameBufferSize;
    UINT64 Width;
    UINT64 Height;
    UINT64 PixelFormat;
    UINT64 PixelsPerScanLine;
    UINT64 RedMask;
    UINT64 GreenMask;
    UINT64 BlueMask;                  
} GraphicsInfo;

typedef struct
{
    UINT64 MomorySizeInMB;
    UINT64 RSDP;
} MemoryInfo;

EFI_STATUS GetGraphicsInfo(IN EFI_SYSTEM_TABLE* ST, OUT GraphicsInfo** GI);

EFI_STATUS GetMemoryInfo(IN EFI_SYSTEM_TABLE* ST, OUT MemoryInfo** MI);
