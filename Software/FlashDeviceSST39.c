#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/expansion_protos.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FlashDeviceSST39.h"

/*****************************************************************************/
/* Globals *******************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Prototypes ****************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Code **********************************************************************/
/*****************************************************************************/

/*****************************************************************************/
/* Function:    checkFlashStatus()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Polls Flash device for completion or timeout status          */
/*****************************************************************************/
tFlashCommandStatus checkFlashStatus(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;
    ULONG totalLoopCount;
    UWORD /*currentWord, */previousWord;

    flashCommandStatus = flashBusy;
    
#ifndef NDEBUG
    printf("ENTRY: checkFlashStatus(ULONG address 0x%X)\n", address);
#endif
    previousWord = *(UWORD *)address;
    
    if (0 == ((previousWord ^ *(UWORD *)address) & TOGGLE_STATUS))
    {
        flashCommandStatus = flashOK;
    }
    
#ifndef NDEBUG
    printf("EXIT: checkFlashStatus(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    unlockFlashDevice()                                          */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Sends initial unlock command sequence to flash device        */
/*****************************************************************************/
tFlashCommandStatus unlockFlashDevice(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: unlockFlashDevice(ULONG address 0x%X)\n", address);
#endif
    
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_1) = FLASH_UNLOCK_DATA_1;
#ifndef NDEBUG
    printf("FLOW: FLASH_UNLOCK_ADDR_1 0x%X, FLASH_UNLOCK_DATA_1 0x%X\n", (address + FLASH_UNLOCK_ADDR_1), FLASH_UNLOCK_DATA_1);
#endif
    *(UWORD *)(address + FLASH_UNLOCK_ADDR_2) = FLASH_UNLOCK_DATA_2;
#ifndef NDEBUG
    printf("FLOW: FLASH_UNLOCK_ADDR_2 0x%X, FLASH_UNLOCK_DATA_2 0x%X\n", (address + FLASH_UNLOCK_ADDR_2), FLASH_UNLOCK_DATA_2);
#endif

    flashCommandStatus = flashOK;
    
#ifndef NDEBUG
    printf("EXIT: unlockFlashDevice(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);    
}

/*****************************************************************************/
/* Function:    readManufactureID()                                          */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UWORD * pManufactureID                        */
/* Description: Reads manufacturer ID from flash device                      */
/*****************************************************************************/
tFlashCommandStatus readManufactureID(ULONG address, UWORD * pManufactureID)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: readManufactureID(ULONG address 0x%X, UWORD * pManufactureID 0x%X)\n", address, pManufactureID);
#endif
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
#ifndef NDEBUG
    printf("FLOW: FLASH_AUTOSEL_ADDR_1 0x%X, FLASH_AUTOSEL_DATA_1 0x%X\n", (address + FLASH_AUTOSEL_ADDR_1), FLASH_AUTOSEL_DATA_1);
#endif
    *pManufactureID = *(UWORD *)(address + FLASH_MANUFACTOR_ID);
#ifndef NDEBUG
    printf("FLOW: FLASH_MANUFACTOR_ID 0x%X, *pManufactureID 0x%X\n", (address + FLASH_MANUFACTOR_ID), *pManufactureID);
#endif
    flashCommandStatus = resetFlashDevice(address);

#ifndef NDEBUG
    printf("EXIT: readManufactureID(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    readDeviceID()                                               */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UWORD * pDeviceID                             */
/* Description: Reads device ID from flash device                            */
/*****************************************************************************/
tFlashCommandStatus readDeviceID(ULONG address, UWORD * pDeviceID)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: readDeviceID(ULONG address 0x%X, UWORD * pManufactureID 0x%X)\n", address, pDeviceID);
#endif
    
    flashCommandStatus = unlockFlashDevice(address);
    
    *(UWORD *)(address + FLASH_AUTOSEL_ADDR_1) = FLASH_AUTOSEL_DATA_1;    
#ifndef NDEBUG
    printf("FLOW: FLASH_AUTOSEL_ADDR_1 0x%X, FLASH_AUTOSEL_DATA_1 0x%X\n", (address + FLASH_AUTOSEL_ADDR_1), FLASH_AUTOSEL_DATA_1);
#endif
    *pDeviceID = *(UWORD *)(address + FLASH_DEVICE_ID);
#ifndef NDEBUG
    printf("FLOW: FLASH_DEVICE_ID 0x%X, *pDeviceID 0x%X\n", (address + FLASH_DEVICE_ID), *pDeviceID);
#endif
    flashCommandStatus = resetFlashDevice(address);
    
#ifndef NDEBUG
    printf("EXIT: readDeviceID(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    resetFlashDevice()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Sends reset command sequence to flash device                 */
/*****************************************************************************/
tFlashCommandStatus resetFlashDevice(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: resetFlashDevice(ULONG address 0x%X)\n", address);
#endif
    
    *(UWORD *)(address + FLASH_RESET_ADDR_1) = FLASH_RESET_DATA_1;    

    flashCommandStatus = flashOK;

#ifndef NDEBUG
    printf("EXIT: resetFlashDevice(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    eraseFlashSector()                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, UBYTE sectorNumber                            */
/* Description: Erases a specific sector number from flash device            */
/*****************************************************************************/
tFlashCommandStatus eraseFlashSector(ULONG address, UBYTE sectorNumber)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: eraseFlashSector(ULONG address 0x%X, UBYTE sectorNumber 0x%X)\n", address, sectorNumber);
#endif

    if (sectorNumber < MAX_SECTORS)
    {
        if (flashOK == unlockFlashDevice(address + (SECTOR_SIZE * sectorNumber)))
        {
            *(UWORD *)(address + FLASH_ERASE_ADDR_1) = FLASH_ERASE_DATA_1;    
#ifndef NDEBUG
            printf("FLOW: FLASH_ERASE_ADDR_1 0x%X, FLASH_ERASE_DATA_1 0x%X\n", (address + FLASH_ERASE_ADDR_1), FLASH_ERASE_DATA_1);
#endif
            
            if (flashOK == unlockFlashDevice(address + (SECTOR_SIZE * sectorNumber)))
            {
                *(UWORD *)(address + (sectorNumber << 16)) = FLASH_SECTOR_DATA_1;
#ifndef NDEBUG
                printf("FLOW: Sector Address 0x%X, FLASH_SECTOR_DATA_1 0x%X\n", address + (sectorNumber << 16), FLASH_SECTOR_DATA_1);
#endif
                flashCommandStatus = flashOK;                
            }
        }
    }
    else
    {
        flashCommandStatus = flashEraseError;
    }
    
#ifndef NDEBUG
    printf("EXIT: eraseFlashSector(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    eraseCompleteFlash                                           */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address                                                */
/* Description: Erases the complete flash device                             */
/*****************************************************************************/
tFlashCommandStatus eraseCompleteFlash(ULONG address)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

#ifndef NDEBUG
    printf("ENTRY: eraseCompleteFlash(ULONG address 0x%X)\n", address);
#endif

    if (flashOK == unlockFlashDevice(address))
    {
        *(UWORD *)(address + FLASH_ERASE_ADDR_1) = FLASH_ERASE_DATA_1;    
#ifndef NDEBUG
        printf("FLOW: FLASH_ERASE_ADDR_1 0x%X, FLASH_ERASE_DATA_1 0x%X\n", (address + FLASH_ERASE_ADDR_1), FLASH_ERASE_DATA_1);
#endif
        
        if (flashOK == unlockFlashDevice(address))
        {
            *(UWORD *)(address + FLASH_ERASE_ADDR_1) = FLASH_COMPLETE_DATA_1;
#ifndef NDEBUG
            printf("FLOW: Complete Flash 0x%X, FLASH_COMPLETE_DATA_1 0x%X\n", (address + FLASH_ERASE_ADDR_1), FLASH_COMPLETE_DATA_1);
#endif
            flashCommandStatus = flashOK;                
        }
        else
        {
            flashCommandStatus = flashEraseError;    
        }
    }
    else
    {
        flashCommandStatus = flashEraseError;
    }
    
#ifndef NDEBUG
    printf("EXIT: eraseCompleteFlash(flashCommandStatus 0x%X)\n", flashCommandStatus);
#endif
    return (flashCommandStatus);
}

/*****************************************************************************/
/* Function:    writeFlashByte()                                             */
/* Returns:     tFlashCommandStatus                                          */
/* Parameters:  ULONG address, ULONG writeAddress, UWORD data                */
/* Description: Writes a specific word to the flash device                   */
/*****************************************************************************/
tFlashCommandStatus writeFlashWord(ULONG baseAddress, ULONG writeAddress, UWORD data)
{
    tFlashCommandStatus flashCommandStatus = flashIdle;

    flashCommandStatus = unlockFlashDevice(baseAddress);
    
    *(UWORD *)(baseAddress + FLASH_PROGRAM_ADDR_1) = FLASH_PROGRAM_DATA_1;    
    *(UWORD *)(baseAddress + writeAddress) = data;
    
    return (flashCommandStatus);
}