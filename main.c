#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleTextInEx.h>

#include "DiskDevice.h"

#define WARN(x) Print(L"WARNING: %s\n", x);
#define ERR(x) Print(L"ERROR: %s\n", x);
#define LOG(x) Print(L"LOG: %a\n", x);
#define ASSERT_PTR_NOT_NULL(x) (bool)x ? 1:0

UINTN EFIAPI PrintAllDrives(){
    
    return 0;
}

char EFIAPI ReadKey(EFI_BOOT_SERVICES* gBS){
    EFI_KEY_DATA key = {0};
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputEx = NULL;
    UINTN index;
    EFI_STATUS status;
    status = gBS->LocateProtocol(&gEfiSimpleTextInputExProtocolGuid, NULL, (void**)&inputEx);
    gBS->WaitForEvent(1, &(inputEx->WaitForKeyEx), &index);
    status = inputEx->ReadKeyStrokeEx(inputEx, &key);
    if (status!=EFI_SUCCESS){
        ERR("Reading key failed");
    }
    return key.Key.UnicodeChar;
}


EFI_STATUS EFIAPI UefiEntry(IN EFI_HANDLE imgHandle, IN EFI_SYSTEM_TABLE* sysTable){
    gST = sysTable;
    gBS = sysTable->BootServices;
    gImageHandle = imgHandle;
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    Print(L"Welcome to DiskSanitizer! Please select the disk you would like to erase:\n");
    return EFI_SUCCESS;
}
