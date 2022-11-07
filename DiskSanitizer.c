#include "DiskSanitizer.h"

CHAR16 EFIAPI ReadKey(EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL* inputExProtocol){
    EFI_KEY_DATA key = {0};
    UINTN index;
    EFI_STATUS status;
    gBS->WaitForEvent(1, &(inputExProtocol->WaitForKeyEx), &index);
    status = inputExProtocol->ReadKeyStrokeEx(inputExProtocol, &key);
    if (status!=EFI_SUCCESS){
        ERR("Reading key failed");
    }
    return key.Key.UnicodeChar;
}

EFI_STATUS InitializeProgramVariables(program_variables* programVariables){
    EFI_STATUS status;

    programVariables->chosen_disk = 0;
    programVariables->exitProgram = NOT_EXIT;

    status = gBS->LocateProtocol(&gEfiDevicePathToTextProtocolGuid, NULL, (void**)&programVariables->devicePathToTextProtocol);
    if (status != EFI_SUCCESS){
        ERR("Error loading EFI_DEVICE_PATH_TO_TEXT_PROTOCOL!\n");
        return EFI_PROTOCOL_ERROR;
    }

    status = gBS->LocateProtocol(&gEfiSimpleTextInputExProtocolGuid, NULL, (void**)&programVariables->inputExProtocol);
    if (status != EFI_SUCCESS){
        ERR("Error loading EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL!\n");
        return EFI_PROTOCOL_ERROR;
    }
    return EFI_SUCCESS;
}

EFI_STATUS DeinitializeProgramVariables(program_variables* programVariables){
    return EFI_SUCCESS;
}

EFI_STATUS RunTheProgram(EFI_BOOT_SERVICES* gBS, EFI_HANDLE imgHandle){
    EFI_STATUS status;
    program_variables programVariables;
    UINTN menuOption = NOT_EXIT;

    status = InitializeProgramVariables(&programVariables);
    if (status != EFI_SUCCESS){
        ERR("Error initializing program variables!\n");
        return EFI_LOAD_ERROR;
    }

    PrintWelcomeMessage();
    programVariables.diskDevicesCount = LocateAllDiskDevices(programVariables.diskDevices, imgHandle, programVariables.devicePathToTextProtocol);
    if (programVariables.diskDevicesCount == GENERAL_ERR_VAL){
        return EFI_OUT_OF_RESOURCES;
    }

    while (programVariables.exitProgram == NOT_EXIT){
        PrintMenu();
        menuOption = ReadKey(programVariables.inputExProtocol) - ASCII_NUMBERS_BEGINNING;
        switch (menuOption){
            case SHOW_CURRENTLY_CHOSEN_DISK:
                Print(L"You chose drive number %d\n", programVariables.chosen_disk);
                break;
            case CHOOSE_DISK:
                PrintAllDrives(programVariables.diskDevices, programVariables.diskDevicesCount);
                programVariables.chosen_disk = ReadKey(programVariables.inputExProtocol) - ASCII_NUMBERS_BEGINNING;
                Print(L"You chose drive number %d\n", programVariables.chosen_disk);
                break;
            case ERASE_DISK:
                /*TODO*/
                break; 
            case EXIT:
                programVariables.exitProgram = EXIT;
                break;
        }

    }

    CloseAllProtocolsForAllDrives(programVariables.diskDevices, imgHandle, programVariables.diskDevicesCount);
    return EFI_SUCCESS;
}

void PrintAllDrives(disk_device* diskDevices, UINTN numHandles){
    Print(L"There are %d drives in the system. Please select one:\n", numHandles);
    for (UINTN i=0; i < numHandles; i++){
        Print(L"Device %d: %s\n", i, diskDevices[i].textDiskPath);
    }
}

void PrintWelcomeMessage(){
    Print(L"Welcome to DiskSanitizer!\nWe will make sure your drive will be clean and ready to be sold!\n");
}

void PrintMenu(){
    Print(L"Menu:\n%d. Show currently chosen disk.\n%d. Choose disk.\n%d. Erase the disk.\n%d. Exit.\n", SHOW_CURRENTLY_CHOSEN_DISK, CHOOSE_DISK, ERASE_DISK, EXIT); /*Make sure the order is aligned with menuOptions enum*/
}

/*
TODO:
a) dynamically alloc disk_device list, not define it's max size
b) add ifdef to file include
c) check if number that user chose is between 0 and numDiskDevices-1
*/
