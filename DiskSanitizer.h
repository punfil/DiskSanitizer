#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "DiskDevice.h"

typedef struct program_variables{
    /*Protocols*/
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* devicePathToTextProtocol;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputExProtocol;

    /*Variables*/
    disk_device diskDevices[8];
    UINTN diskDevicesCount;
    UINTN chosen_disk;
    UINTN exitProgram;

} program_variables;

typedef enum menu_options{
    SHOW_CURRENTLY_CHOSEN_DISK,
    CHOOSE_DISK,
    ERASE_DISK,
    EXIT,
    NOT_EXIT,

} menu_options;

CHAR16 EFIAPI ReadKey(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputExProtocol);

EFI_STATUS InitializeProgramVariables(program_variables* programVariables);
EFI_STATUS RunTheProgram(EFI_BOOT_SERVICES* gBS, EFI_HANDLE imgHandle);
EFI_STATUS DeinitializeProgramVariables(program_variables* programVariables);

void PrintAllDrives(disk_device* diskDevices, UINTN numHandles);
void PrintWelcomeMessage();
void PrintMenu();
