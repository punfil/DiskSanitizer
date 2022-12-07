#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "DiskSanitizer.h"


EFI_STATUS EFIAPI UefiEntry(IN EFI_HANDLE imgHandle, IN EFI_SYSTEM_TABLE* sysTable){
    gST = sysTable;
    gBS = sysTable->BootServices;
    gImageHandle = imgHandle;
    EFI_STATUS status;

    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    status = RunTheProgram(gBS, imgHandle);
    return status;
}
