#include "consoleIOForNuttx.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

void readEscSeq(char);
char* usaToEurDate(const char* usaDate);
void startConsole(void);
void displayBaseInfo(void);
void parseCommand(void);
bool printUnitsCommand(int type);
void CIO_printChar(char);
bool isValidChar(char);

consoleInfo info = { 0 };
outputConsole* unitTbl;
int unitCnt = 0;
char command_value[COMMAND_SIZE] = { 0 };
int commandBufferIndex = 0;
int historySize = 0;
int newRecord = -1;
int curRecord = 0;
int cntRecord = 0;

const char* titleTbl[] = {
    "ADC",
    "GPIO",
    "HUMI",
    "MASTER",
    "PRESS",
    "SLAVE",
    "TEMP"
};

int CIO_inputBufferSize;
char* CIO_inputBuffer;
char CIO_outputBuffer[CIO_OUTPUT_BUFFER_SIZE];
readOutputBuffer CIO_printFun;

void CIO_initIO(readOutputBuffer writeFun)
{
    CIO_printFun = writeFun;
    // CIO_outputBuffer = malloc(CIO_OUTPUT_BUFFER_SIZE);
}

void displayBaseInfo(void)
{
    CIO_printf(" MODEL        - %s\r\n", info.deviceName);
    CIO_printf(" VERSION      - %s\r\n", info.deviceVersion);
    CIO_printf(" DATE         - %s\r\n", info.timestamp);
    CIO_printf(" AUTHOR       - %s\r\n", info.author);
}

void CIO_displayBaseInfo(char* deviceName, char* deviceVersion, char* timestamp, char* author)
{
    CIO_printf(" MODEL        - %s\r\n", deviceName);
    CIO_printf(" VERSION      - %s\r\n", deviceVersion);
    CIO_printf(" DATE         - %s\r\n", timestamp);
    CIO_printf(" AUTHOR       - %s\r\n", author);
}

void startConsole(void)
{
    if (strlen(info.timestamp) == 11 && info.timestamp[0] >= 'A') {
        info.timestamp = (const char*)usaToEurDate(info.timestamp);
    }
    displayBaseInfo();
    CIO_printf("\r\n> ");
}

char* usaToEurDate(const char* usaDate)
{
    char* eurDate = malloc(strlen(info.timestamp));
    eurDate[0] = (usaDate[4] == ' ' ? '0' : usaDate[4]);
    eurDate[1] = usaDate[5];
    eurDate[2] = usaDate[6]; // день
    eurDate[3] = usaDate[0];
    eurDate[4] = usaDate[1];
    eurDate[5] = usaDate[2];
    eurDate[6] = usaDate[3]; // мес€ц
    eurDate[7] = usaDate[7];
    eurDate[8] = usaDate[8];
    eurDate[9] = usaDate[9];
    eurDate[10] = usaDate[10]; // год
    eurDate[11] = '\0';
    return eurDate;
}

bool isFirst = true;
char lastType[20] = "";
int sizeTbl = 0;
int CIO_printConsoleUnit(const char* name, const char* param, const char* description, const char* data, const char* type)
{
    if (isFirst) {
        CIO_printf("/*****************************************************************************\r\n");
        CIO_printf(" * UNITS                   | VALUE | DESCRIPTION                             *\r\n");
        isFirst = false;
    }

    if (strcmp(type, lastType)) {
        // char* temp = realloc(lastType, strlen(type) + 1);
        // if (temp == NULL) {
        //     return -1;
        // }
        // lastType = temp;
        strcpy(lastType, type);

        CIO_printf(" *---------------------------------------------------------------------------*\r\n");
        CIO_printf(" *                                     %-38s*\r\n", type);
        CIO_printf(" *---------------------------------------------------------------------------*\r\n");
    }

    CIO_printf(" * %-10s / %-10s | %5s | %-39s *\r\n", name, param, data, description);

    return 0;
}

void CIO_helpCommand(void)
{
    CIO_printf(" adc     - show the ADC data\r\n");
    CIO_printf(" check   - perform the system self check and show status\r\n");
    CIO_printf(" desc    - show description of the project and firmware\r\n");
    CIO_printf(" gpio    - show the status of gpio inputs and outputs\r\n");
    CIO_printf(" help    - show the command list\r\n");
    CIO_printf(" humi    - show the humidity data\r\n");
    CIO_printf(" inter   - show all interfaces\r\n");
    CIO_printf(" master  - show the master interface bus data\r\n");
    CIO_printf(" press   - show the pressure data\r\n");
    CIO_printf(" scheme  - show the structure scheme of project\r\n");
    CIO_printf(" sens    - show the data from all sensors\r\n");
    CIO_printf(" slave   - show the slave interface data buffer\r\n");
    CIO_printf(" temp    - show the temperature data\r\n");
}

void CIO_printf(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vsprintf(CIO_outputBuffer, format, argptr);
    va_end(argptr);

    CIO_printFun(CIO_outputBuffer);
    return;

    int fd;
    char* devpath;

    devpath = "/dev/console";

    fd = open(devpath, O_WRONLY);

    write(fd, CIO_outputBuffer, CIO_OUTPUT_BUFFER_SIZE);

    close(fd);
}

void CIO_printChar(char c)
{
    sprintf(CIO_outputBuffer, "%c", c);
    CIO_printFun(CIO_outputBuffer);
}

bool isValidChar(char c)
{
    return (c >= 0x20 && c <= 0x7e);
}

void CIO_endProgramm()
{
    if (!isFirst)
        CIO_printf(" *****************************************************************************/\r\n");
    // free(CIO_outputBuffer);
    isFirst = true;
    // if (lastType != NULL)
    //     free(lastType);
    // lastType = NULL;
    lastType[0] = '\0';
}
