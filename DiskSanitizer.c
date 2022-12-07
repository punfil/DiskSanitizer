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

    programVariables->chosenDisk = GENERAL_ERR_VAL;
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

EFI_STATUS EraseTheDrive(disk_device* diskDevice, UINT8 numberToWrite){
    UINT8* buffer;
    EFI_STATUS status;
    buffer = AllocatePool(diskDevice->blockIoProtocol->Media->BlockSize);
    for (UINTN i=0;i<diskDevice->blockIoProtocol->Media->BlockSize;i++){
        buffer[i] = numberToWrite;
    }
    for (EFI_LBA i=0;i<=diskDevice->blockIoProtocol->Media->LastBlock;i++){
        status = diskDevice->blockIoProtocol->WriteBlocks(diskDevice->blockIoProtocol, diskDevice->blockIoProtocol->Media->MediaId, i, diskDevice->blockIoProtocol->Media->BlockSize, (void*)buffer);
        if (status != EFI_SUCCESS){
            Print(L"Unable to write the disk block %d, return code %d\n", i, status);
        }
        else{
            Print(L"Erased block %d out of %d\n", i, diskDevice->blockIoProtocol->Media->LastBlock);
        }
    }
    FreePool(buffer);
    return EFI_SUCCESS;
}

EFI_STATUS ShowDiskContent(disk_device* diskDevice){
    UINT8* buffer;
    EFI_STATUS status;
    buffer = AllocateZeroPool(diskDevice->blockIoProtocol->Media->BlockSize);
    for (EFI_LBA i=0; i<=diskDevice->blockIoProtocol->Media->LastBlock;i++){
        status = diskDevice->blockIoProtocol->ReadBlocks(diskDevice->blockIoProtocol, diskDevice->blockIoProtocol->Media->MediaId, i, diskDevice->blockIoProtocol->Media->BlockSize, (void*)buffer);
        if (status != EFI_SUCCESS){
            Print(L"Unable to read the disk block %d, return code\n", i, status);
        }
        else{
            Print(L"Printing block %d out of %d\n", i, diskDevice->blockIoProtocol->Media->LastBlock);
            for (UINTN j=0;j<diskDevice->blockIoProtocol->Media->BlockSize;j++){
                if (j%16==0){
                    Print(L"\n");
                }
                Print(L"%X ", buffer[j]);
            }
            Print(L"\n");
        }
    }
    FreePool(buffer);
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
                if (programVariables.chosenDisk != GENERAL_ERR_VAL){
                    Print(L"You chose drive number %d\n", programVariables.chosenDisk);
                    PrintDetailedDeviceInfo(programVariables.diskDevices[programVariables.chosenDisk].blockIoProtocol->Media);
                }
                else{
                    Print(L"You've not chosen any drive yet!\n");
                }
                break;
            case CHOOSE_DISK:
                PrintAllDrives(programVariables.diskDevices, programVariables.diskDevicesCount);
                programVariables.chosenDisk = ReadKey(programVariables.inputExProtocol) - ASCII_NUMBERS_BEGINNING;
                if (programVariables.chosenDisk < 0 || programVariables.chosenDisk > programVariables.diskDevicesCount){
                    programVariables.chosenDisk = GENERAL_ERR_VAL;
                    Print(L"You chose a device that is not in the system!\n");
                }
                else{
                    Print(L"You chose drive number %d\n", programVariables.chosenDisk);   
                }
                break;
            case TEST_WRITE_CONTENT_TO_DISK:
                if (programVariables.chosenDisk != GENERAL_ERR_VAL){
                    Print(L"Enter the number to write on the disk\n");
                    UINT8 numberToWrite = ReadKey(programVariables.inputExProtocol) - ASCII_NUMBERS_BEGINNING;
                    EraseTheDrive(&programVariables.diskDevices[programVariables.chosenDisk], numberToWrite);
                }
                else{
                    Print(L"You've not chosen any drive yet!\n");
                }
                break; 
            case ERASE_DISK:
                if (programVariables.chosenDisk != GENERAL_ERR_VAL){
                    EraseTheDrive(&programVariables.diskDevices[programVariables.chosenDisk], 0);
                }
                else{
                    Print(L"You've not chosen any drive yet!\n");
                }
                break; 
            case READ_DISK:
                if (programVariables.chosenDisk != GENERAL_ERR_VAL){
                    ShowDiskContent(&programVariables.diskDevices[programVariables.chosenDisk]);
                }
                else{
                    Print(L"You've not chosen any drive yet!\n");
                }
                break;
            case EXIT:
                programVariables.exitProgram = EXIT;
                break;
            default:
                Print(L"You entered %d option. It's not correct!\n");
        }

    }

    CloseAllProtocolsForAllDrives(programVariables.diskDevices, imgHandle, programVariables.diskDevicesCount);
    return EFI_SUCCESS;
}

void PrintAllDrives(disk_device* diskDevices, UINTN numHandles){
    WARN("We only list drives that are not read-only, nor a Logical Partition!\n");
    UINTN drives_cnt = 0;
    for (UINTN i=0; i < numHandles; i++){
        EFI_BLOCK_IO_MEDIA* media = diskDevices[i].blockIoProtocol->Media;
        if (media->ReadOnly==0){
            drives_cnt++;
            Print(L"Device %d: %s\n", i, diskDevices[i].textDiskPath);
        }
        
    }
    Print(L"There are %d drives in the system. Please select one:\n", drives_cnt);
}

void PrintWelcomeMessage(){
    Print(L"Welcome to DiskSanitizer!\nWe will make sure your drive will be clean and ready to be sold!\n");
}

void PrintMenu(){
    Print(L"Menu:\n%d. Show currently chosen disk.\n%d. Choose disk.\n%d. Test write content to disk.\n%d. Erase the disk.\n%d. Read the disk.\n%d. Exit.\n", SHOW_CURRENTLY_CHOSEN_DISK, CHOOSE_DISK, TEST_WRITE_CONTENT_TO_DISK, ERASE_DISK, READ_DISK, EXIT); /*Make sure the order is aligned with menuOptions enum*/
}

/*
TODO:
a) dynamically alloc disk_device list, not define it's max size
b) add ifdef to file include
c) check if number that user chose is between 0 and numDiskDevices-1
*/
