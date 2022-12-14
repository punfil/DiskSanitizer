#include "DiskDevice.h"

UINTN LocateAllDiskDevices(disk_device* diskDevices, EFI_HANDLE imgHandle, EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* devicePathToTextProtocol){
    EFI_STATUS status;
    EFI_HANDLE* handle = NULL;
    UINTN numHandles;
    status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDiskIoProtocolGuid, NULL, &numHandles, &handle);
    if (status != EFI_SUCCESS){
        ERR("Error loading EFI_DISK_IO_PROTOCOL\n");
        return EFI_PROTOCOL_ERROR;
    }
    for (UINTN i=0; i < numHandles; i++){
        diskDevices[i].handle = handle[i];
        GetDataFromController(imgHandle, &diskDevices[i]);
        diskDevices[i].textDiskPath = devicePathToTextProtocol->ConvertDevicePathToText(diskDevices[i].diskPathProtocol, TRUE, TRUE);
        if (diskDevices[i].textDiskPath == NULL){
            Print(L"Insufficient resources to allocate a string!\n");
            return GENERAL_ERR_VAL;
        }
    }
    return numHandles;
}

EFI_STATUS GetDataFromController(EFI_HANDLE imgHandle, disk_device* diskDevice){
    EFI_STATUS status;
    status = gBS->OpenProtocol(diskDevice->handle, &gEfiBlockIoProtocolGuid, (void**)(&diskDevice->blockIoProtocol), imgHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (status != EFI_SUCCESS){
        ERR("Error loading EFI_BLOCK_IO_PROTOCOL for the drive\n");
        return EFI_PROTOCOL_ERROR;
    }
    status = gBS->OpenProtocol(diskDevice->handle, &gEfiDevicePathProtocolGuid, (void**)(&diskDevice->diskPathProtocol), imgHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (status != EFI_SUCCESS){
        ERR("Error loading EFI_DEVICE_PATH_PROTOCOL for the drive\n");
        return EFI_PROTOCOL_ERROR;
    }

    return EFI_SUCCESS;
}

EFI_STATUS CloseAllProtocolsForAllDrives(disk_device* diskDevices, EFI_HANDLE imgHandle, UINTN numHandles){
    for (UINTN i=0; i < numHandles; i++){
        CloseAllProtocolsForDrive(imgHandle, &diskDevices[i]);
    }
    return EFI_SUCCESS;
}

EFI_STATUS CloseAllProtocolsForDrive(EFI_HANDLE imgHandle, disk_device* diskDevice){
    EFI_STATUS status;
    status = gBS->CloseProtocol(diskDevice->handle, &gEfiBlockIoProtocolGuid, imgHandle, NULL);
    if (status != EFI_SUCCESS){
        ERR("Error closing EFI_BLOCK_IO_PROTOCOL for the drive\n");
        return EFI_PROTOCOL_ERROR;
    }
    status = gBS->CloseProtocol(diskDevice->handle, &gEfiDevicePathProtocolGuid, imgHandle, NULL);
    if (status != EFI_SUCCESS){
        ERR("Error closing EFI_DEVICE_PATH_PROTOCOL for the drive\n");
        return EFI_PROTOCOL_ERROR;
    }
    gBS->FreePool(diskDevice->textDiskPath);
    return EFI_SUCCESS;
}

EFI_STATUS PrintDetailedDeviceInfo(EFI_BLOCK_IO_MEDIA* blockIoMedia){
    if (blockIoMedia == NULL){
        return EFI_DEVICE_ERROR;
    }
    Print(L"Device:\nMedia ID: %d\nRemovable media: %d\nMedia present: %d\nLogical partition: %d\nRead only: %d\nWrite Caching: %d\nBlock size: %d\nIO align: %d\nLast block: %d\n",
    blockIoMedia->MediaId, blockIoMedia->RemovableMedia, blockIoMedia->MediaPresent, blockIoMedia->LogicalPartition, blockIoMedia->ReadOnly, blockIoMedia->WriteCaching, blockIoMedia->BlockSize, blockIoMedia->IoAlign, blockIoMedia->LastBlock);
    return EFI_SUCCESS;
}
