#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
 
EFI_STATUS EFIAPI UefiEntry(IN EFI_HANDLE imgHandle, IN EFI_SYSTEM_TABLE* sysTable)
{
    gST = sysTable;
    gBS = sysTable->BootServices;
    gImageHandle = imgHandle;
    // UEFI apps automatically exit after 5 minutes. Stop that here
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    Print(L"Hello, world!\r\n");
    // Allocate a string
    CHAR16* str = NULL;
    gBS->AllocatePool(EfiLoaderData, 48, (VOID**)&str);
    // Copy over a string
    CHAR16* str2 = L"Allocated stringg\r\n";
    gBS->CopyMem((VOID*)str, (VOID*)str2, 48);
    Print(str);
    gBS->FreePool(str);
    return EFI_SUCCESS;
}
