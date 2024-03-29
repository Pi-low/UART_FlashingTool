/*
 *  The UART Flashing tool aims to flash embedded software for the
 *  "Big Brain" board system.
 *  Copyright (C) 2023 Nello CHOMMANIVONG
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Config.h"
#include "main.h"
#include "Bootloader/Bootloader.h"
#include "ComPort/ComPort.h"
#include "Logger/Logger.h"

uint32_t main_GetFileSize(FILE* FpHexFile);
const char* pcBuildDateTime = __DATE__ " " __TIME__;
const uint8_t u8ToolVersion = 1;

int main(int argc, char * argv[])
{
    uint8_t u8KeepLoop = 1;
    FILE *MyFile;
#if SEND_UART
    teMainStates teMainCurrentState = eStateSelectComPort;
#else
    teMainStates teMainCurrentState = eStateSelectFile;
#endif
    char pcLogString[256];
    uint32_t u32TotalFileSize = 0;
    char *pcString = NULL;
    char pcBuffer[10];
    bool bTmp = true;
    printf("BigBrain flashing tool\r\nBuild: %s\r\nRelease: %02u\r\n\r\n", pcBuildDateTime, u8ToolVersion);
    while (u8KeepLoop)
    {
        switch(teMainCurrentState)
        {
            case eStateSelectComPort:
                ComPort_Scan();
                printf("Select port COM: \r\n");
                fflush(stdin);
                scanf("%[^\n]", pcBuffer);
                if (ComPort_Open(pcBuffer))
                {
                    teMainCurrentState = eStateSelectFile;
                }
                else
                {
                    teMainCurrentState = eStateQuit;
                }
            break;

            case eStateSelectFile:
                /* Catch filename argument */
                if ((argc > 1) && (argv[1] != NULL) && (pcString == NULL))
                {
                    pcString = argv[1];
                }
                else
                {
#ifdef DEBUG_CONFIG
                    pcString = (char*) malloc(strlen("test01.hex") * sizeof(char));
                    sprintf(pcString, "test01.hex");
#else
                    fflush(stdin);
                    pcString = (char*) malloc(256 * sizeof(char));
                    printf("Enter filename (**.hex): \r\n");
                    scanf("%[^\n]", pcString);
#endif // DEBUG_CONFIG
                }
                MyFile = fopen(pcString, "rb");
                if (MyFile != NULL)
                {
                    Logger_Init();
                    sprintf(pcLogString, "Open file: %s\r\n", pcString);
                    Logger_Append(pcLogString);
                    printf("Open: \"%s\"\r\n", pcString);
                    u32TotalFileSize = main_GetFileSize(MyFile);
                    Bootloader_GetInfoFromiHexFile(MyFile, u32TotalFileSize);
                    Bootloader_GetHexSizeBytes(MyFile, u32TotalFileSize);
                    teMainCurrentState = eStateTargetInfo;
                    system("pause");
                }
                else
                {
                    printf("[Error]: Cannot open file, exit program !\r\n");
                    Logger_Append("Error: Cannot open file, exit program !\r\n");
                    teMainCurrentState = eStateQuit;
                }
            break;

            case eStateTargetInfo:
                if(ComPort_WaitForStartupSequence(10000))
                {
                    Sleep(100);
                    bTmp &= Bootloader_RequestSwVersion(NULL);
                    bTmp &= Bootloader_RequestSwInfo(NULL);
                    if (bTmp)
                    {
                        teMainCurrentState = eStateFlashTarget;
                    }
                    else
                    {
                        printf("[Error]: Abort operation !\r\n");
                        Logger_Append("Error: Abort operation !\r\n");
                        teMainCurrentState = eStateQuit;
                    }
                }
                else
                {
                    teMainCurrentState = eStateQuit;
                }
            break;

            case eStateFlashTarget:
                if (MyFile != NULL)
                {
                    bTmp &= Bootloader_RequestBootSession();
                    bTmp &= Bootloader_RequestEraseFlash();
                    if (bTmp)
                    {
                        Bootloader_ProcessFile(MyFile, u32TotalFileSize);
                        Bootloader_CheckFlash();
                    }
                    else
                    {
                        printf("[Error]: Abort operation !\r\n");
                        Logger_Append("Error: Abort operation !\r\n");
                    }
                }
                teMainCurrentState = eStateQuit;
            break;

            case eStateQuit:

                printf("Restart ? (y) : ");
                fflush(stdin);
                scanf("%[^\n]", pcBuffer);
                if (pcBuffer[0] == 'y')
                {
                    teMainCurrentState = eStateSelectComPort;
                    bTmp = true;
                }
                else
                {
                    u8KeepLoop = 0;
                }
            break;

            default:
                u8KeepLoop = 0;
            break;
        }
    }
    if (MyFile != NULL)
    {
        fclose(MyFile);
    }
    ComPort_Close();
    Logger_Close();
    return EXIT_SUCCESS;
}

uint32_t main_GetFileSize(FILE* FpHexFile)
{
    uint32_t u32FileSize = 0;
    if (fseek(FpHexFile, 0, SEEK_END) == 0)
    {
        u32FileSize = ftell(FpHexFile);
    }
    return u32FileSize;
}
