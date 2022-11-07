#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/DiskIo.h> 
#include <Protocol/BlockIo.h> 
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo2.h>
#include <Protocol/DevicePathToText.h>
#include <Library/BaseLib.h>

#include "Defines.h"

typedef struct disk_device{
    EFI_HANDLE handle;
    EFI_DISK_IO_PROTOCOL* diskIoProtocol;
    EFI_DISK_IO2_PROTOCOL* diskIo2Protocol;
    EFI_BLOCK_IO_PROTOCOL* blockIoProtocol;
    EFI_BLOCK_IO2_PROTOCOL* blockIo2Protocol;
    EFI_DEVICE_PATH_PROTOCOL* diskPathProtocol;
    CHAR16* textDiskPath;

}disk_device;


UINTN LocateAllDiskDevices(disk_device* diskDevices, EFI_HANDLE imgHandle, EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* devicePathToTextProtocol);
EFI_STATUS GetDataFromController(EFI_HANDLE imgHandle, disk_device* diskDevice);

EFI_STATUS CloseAllProtocolsForAllDrives(disk_device* diskDevices, EFI_HANDLE imgHandle, UINTN numHandles);
EFI_STATUS CloseAllProtocolsForDrive(EFI_HANDLE imgHandle, disk_device* diskDevice);
