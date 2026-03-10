#include "consoleIO.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readEscSeq(char);
char* usaToEurDate(const char* usaDate);
void startConsole(void);
void displayBaseInfo(void);
void parseCommand(void);
bool printUnitsCommand(int type);
void helpCommand(void);
void CIO_printChar(char);
bool isValidChar(char);
void addToHistory(bool);
void prevRecord(void);
void nextRecord(void);
void printHistory(void);

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
char* CIO_outputBuffer;
readOutputBuffer CIO_printFun;

void CIO_initIO(char* inputBuffer, int inputBufferSize, readOutputBuffer writeFun)
{
    CIO_inputBufferSize = inputBufferSize;
    CIO_inputBuffer = inputBuffer;
    CIO_printFun = writeFun;
    CIO_outputBuffer = malloc(CIO_OUTPUT_BUFFER_SIZE);
}

void CIO_baseInfo(char* deviceName, char* deviceVersion, char* timestamp, char* author)
{
    info.deviceName = deviceName;
    info.deviceVersion = deviceVersion;
    info.timestamp = timestamp;
    info.author = author;

    startConsole();
}

bool echoEnabled = false;
bool escSeqReading = false;
bool tempSaved = false;
void CIO_readBuffer()
{
    CIO_printf("read buffer: %s\n\r", CIO_inputBuffer);
    for (int i = 0; i < CIO_inputBufferSize; i++, commandBufferIndex++) {
        // 		CIO_printf("%c", CIO_inputBuffer[i]);
        // 		continue;
        if (CIO_inputBuffer[i] == '\0') {
            CIO_printChar(CIO_inputBuffer[i]);
            break;
        }
        if (escSeqReading) {
            readEscSeq(CIO_inputBuffer[i]);
            continue;
        }
        if (CIO_inputBuffer[i] == '\x1b') {
            escSeqReading = true;
            continue;
        }
        // символ backspace
        if (CIO_inputBuffer[i] == '\x7f' && commandBufferIndex > 0) {
            CIO_printChar(CIO_inputBuffer[i]);
            command_value[commandBufferIndex - 1] = '\0';
            commandBufferIndex -= 2;
            continue;
        }
        if (CIO_inputBuffer[i] == '\r' || CIO_inputBuffer[i] == '\n') {
            CIO_printChar(CIO_inputBuffer[i]);
            command_value[commandBufferIndex] = '\0';
            CIO_printf("\n");
            commandBufferIndex = 0;
            parseCommand();
            break;
        }
        // 		if(CIO_inputBuffer[i] == '\x41')
        // 		{
        // 			if(!tempSaved)
        // 			{
        // 				addToHistory(true);
        // 				tempSaved = true;
        // 			}
        // 			prevRecord();
        // 			break;
        // 		}
        // 		if(CIO_inputBuffer[i] == '\x42')
        // 		{
        // 			if(!tempSaved)
        // 			{
        // 				addToHistory(true);
        // 				tempSaved = true;
        // 			}
        // 			nextRecord();
        // 			break;
        // 		}
        if (!isValidChar(CIO_inputBuffer[i])) {
            continue;
        }
        if (echoEnabled) {
            CIO_printChar(CIO_inputBuffer[i]);
        }
        command_value[commandBufferIndex] = CIO_inputBuffer[i];
    }
    memset(CIO_inputBuffer, 0, CIO_inputBufferSize);
}

#define SUPPORTES_SEQ_NUM 4
char suppurtedSeq[4][5] = {
    "[A", // стрелка вверх
    "[B", // стрелка вниз
    "[C", // стрелка вправо
    "[D", // стрелка влево
    "[3~" // кнопка delete
};
char escSeq[5] = { '\0', '\0', '\0', '\0', '\0' };
int curSymbol = 0;
void readEscSeq(char c)
{
    escSeq[curSymbol] = c;
    bool isExsist = false;
    for (int i = 0; i < SUPPORTES_SEQ_NUM; i++) {
        for (int j = 0; j <= curSymbol; j++) {
            if (escSeq[j] != suppurtedSeq[i][j]) {
                break;
            }
            isExsist = (j == curSymbol) ? true : false;
        }
        if (isExsist) {
            break;
        }
    }
    if (!isExsist) {
        memset(escSeq, '\0', 5);
        curSymbol = 0;
        escSeqReading = false;
        return;
    }
    curSymbol++;

    if (strcmp(escSeq, "[A") == 0) {
        if (!tempSaved) {
            addToHistory(true);
            tempSaved = true;
        }
        memset(escSeq, '\0', 5);
        curSymbol = 0;
        escSeqReading = false;
        prevRecord();
    } else if (strcmp(escSeq, "[B") == 0) {
        if (!tempSaved) {
            addToHistory(true);
            tempSaved = true;
        }
        memset(escSeq, '\0', 5);
        curSymbol = 0;
        escSeqReading = false;
        prevRecord();
    } else if (strcmp(escSeq, "[C") == 0 || strcmp(escSeq, "[D") == 0 || strcmp(escSeq, "[3~") == 0) {
        memset(escSeq, '\0', 5);
        curSymbol = 0;
        escSeqReading = false;
    }
    return;
}

int sizeTbl = 0;
int CIO_addConsoleUnit(const char* name, const char* param, const char* description, getDataFun getData, void* getDataParam, uint8_t type)
{
    printf("start add unit: %s\r\n", name);
    if (unitCnt == 0) {
        unitTbl = malloc(sizeof(outputConsole));
        if (unitTbl == NULL) {
            return -1;
        }
        sizeTbl += sizeof(outputConsole) + strlen(name) + strlen(param) + strlen(description) + sizeof(getDataFun);
        printf("table size: %i\r\n", sizeTbl);
    } else {
        outputConsole* newTbl = realloc(unitTbl, sizeof(outputConsole) * (unitCnt + 1));
        if (newTbl == NULL) {
            return -1;
        }
        unitTbl = newTbl;
        sizeTbl += sizeof(outputConsole) + strlen(name) + strlen(param) + strlen(description) + sizeof(getDataFun);
        printf("table size: %i\r\n", sizeTbl);
    }

    unitTbl[unitCnt].name = name;
    unitTbl[unitCnt].param = param;
    unitTbl[unitCnt].description = description;
    unitTbl[unitCnt].getData = getData;
    unitTbl[unitCnt].getDataParam = getDataParam;
    unitTbl[unitCnt].type = type;

    unitCnt++;
    printf("end add unit\r\n");
    return 0;
}

void displayBaseInfo(void)
{
    CIO_printf(" MODEL        - %s\r\n", info.deviceName);
    CIO_printf(" VERSION      - %s\r\n", info.deviceVersion);
    CIO_printf(" DATE         - %s\r\n", info.timestamp);
    CIO_printf(" AUTHOR       - %s\r\n", info.author);
}

char* usaToEurDate(const char* usaDate)
{
    char* eurDate = malloc(strlen(info.timestamp));
    eurDate[0] = (usaDate[4] == ' ' ? '0' : usaDate[4]);
    eurDate[1] = usaDate[5];
    eurDate[2] = usaDate[6]; // День
    eurDate[3] = usaDate[0];
    eurDate[4] = usaDate[1];
    eurDate[5] = usaDate[2];
    eurDate[6] = usaDate[3]; // Месяц
    eurDate[7] = usaDate[7];
    eurDate[8] = usaDate[8];
    eurDate[9] = usaDate[9];
    eurDate[10] = usaDate[10]; // Год
    eurDate[11] = '\0';
    return eurDate;
}

void startConsole(void)
{
    if (strlen(info.timestamp) == 11 && info.timestamp[0] >= 'A') {
        info.timestamp = (const char*)usaToEurDate(info.timestamp);
    }
    displayBaseInfo();
    CIO_printf("\r\n> ");
}

void parseCommand(void)
{
    CIO_printf("parse start\n\r");
    char* firstComm = strtok(command_value, " ");
    if (strlen(firstComm) == 2) // Введена пустая строка
    {
        CIO_printf("> ");
        return;
    } else if (strncmp(firstComm, "desc", COMMAND_SIZE) == 0) {
        displayBaseInfo();
    } else if (strncmp(firstComm, "help", COMMAND_SIZE) == 0 || strncmp(firstComm, "h", COMMAND_SIZE) == 0) {
        helpCommand();
    } else if (strncmp(firstComm, "adc", COMMAND_SIZE) == 0) {
        CIO_printf("parse OK\n\r");
        printUnitsCommand(CIO_ADC_TYPE);
    } else if (strncmp(firstComm, "gpio", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_GPIO_TYPE);
    } else if (strncmp(firstComm, "humi", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_HUMI_TYPE);
    } else if (strncmp(firstComm, "master", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_MASTER_TYPE);
    } else if (strncmp(firstComm, "press", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_PRESS_TYPE);
    } else if (strncmp(firstComm, "slave", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_SLAVE_TYPE);
    } else if (strncmp(firstComm, "temp", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_TEMP_TYPE);
    } else if (strncmp(firstComm, "sens", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_ADC_TYPE);
        printUnitsCommand(CIO_HUMI_TYPE);
        printUnitsCommand(CIO_PRESS_TYPE);
        printUnitsCommand(CIO_TEMP_TYPE);
    } else if (strncmp(firstComm, "inter", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_GPIO_TYPE);
        printUnitsCommand(CIO_MASTER_TYPE);
        printUnitsCommand(CIO_SLAVE_TYPE);
    } else if (strncmp(firstComm, "check", COMMAND_SIZE) == 0) {
        printUnitsCommand(CIO_ADC_TYPE);
        printUnitsCommand(CIO_HUMI_TYPE);
        printUnitsCommand(CIO_PRESS_TYPE);
        printUnitsCommand(CIO_TEMP_TYPE);
        printUnitsCommand(CIO_GPIO_TYPE);
        printUnitsCommand(CIO_MASTER_TYPE);
        printUnitsCommand(CIO_SLAVE_TYPE);
    } else if (strncmp(firstComm, "test", COMMAND_SIZE) == 0) {

    } else if (strncmp(firstComm, "history", COMMAND_SIZE) == 0) {
        printHistory();
    } else {
        CIO_printf(" Unrecognized command \"%s\"\n\r", command_value);
        CIO_printf(" Use \"help\" to see the list of commands\r\n");
    }
    addToHistory(false);
    tempSaved = false;
    memset(command_value, '\0', COMMAND_SIZE);

    CIO_printf("> ");
}

void CIO_tryToParseCommand(char* command)
{
    sprintf(command_value, command);
    parseCommand();
}

bool printUnitsCommand(int type)
{
    char* value = malloc(10);
    bool first = 1;
    CIO_printf("count of units: %i\n\r", unitCnt);
    CIO_printf("type of units: %s\n\r", titleTbl[type]);
    for (int i = 0; i < unitCnt; i++) {
        if (unitTbl[i].type == type) {
            if (first) {
                CIO_printf("---------------------------%s-----------------------------------------\r\n", titleTbl[type]);
                first = 0;
            }
            if (unitTbl[i].getData != NULL) {
                unitTbl[i].getData(value, unitTbl[i].getDataParam);
                CIO_printf(" %-10s / %-10s | %-10s | %-40s\r\n", unitTbl[i].name, unitTbl[i].param, value, unitTbl[i].description);
            } else {
                CIO_printf(" %-10s / %-10s | %-10s | %-40s\r\n", unitTbl[i].name, unitTbl[i].param, "--", unitTbl[i].description);
            }
        }
    }
    if (!first) {
        CIO_printf("\r\n");
    }
    free(value);
    return (!first);
}

void helpCommand(void)
{
    CIO_printf(" adc     - show the ADC data\r\n");
    CIO_printf(" check   - perform the system self check and show status\r\n");
    CIO_printf(" desc    - show description of the project and firmware\r\n");
    CIO_printf(" gpio    - show the status of gpio inputs and outputs\r\n");
    CIO_printf(" help    - show the command list\r\n");
    if (historySize > 0)
        CIO_printf(" history - show the commands history\r\n");
    CIO_printf(" humi    - show the humidity data\r\n");
    CIO_printf(" inter   - show all interfaces\r\n");
    CIO_printf(" master  - show the master interface bus data\r\n");
    CIO_printf(" press   - show the pressure data\r\n");
    CIO_printf(" scheme  - show the structure scheme of project\r\n");
    CIO_printf(" sens    - show the data from all sensors\r\n");
    CIO_printf(" slave   - show the slave interface data buffer\r\n");
    CIO_printf(" temp    - show the temperature data\r\n");
    // 		   "test <system> <unit> <value>  - perform the testing of selected system\r\n"
    // 		   "unit <system> <unit>          - show an unit information\r\n");
}

void CIO_printf(const char* format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vsprintf(CIO_outputBuffer, format, argptr);
    va_end(argptr);
    CIO_printFun(CIO_outputBuffer);
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

char** history;
void enableHistory(int size)
{
    if (historySize > 0)
        free(history);
    historySize = size + 1;
    if (size > 0)
        history = (char**)malloc(historySize * sizeof(char*));
}

void addToHistory(bool isTemp)
{
    if (historySize == 0)
        return;
    newRecord = (newRecord + 1) % historySize;
    curRecord = newRecord;
    if (strlen(command_value) == 0 && !isTemp)
        return;
    if (tempSaved) {
        newRecord = (newRecord > 0) ? (newRecord - 1) : (cntRecord - 1);
        curRecord = newRecord;
        if (cntRecord == historySize)
            free(history[newRecord]);
        cntRecord--;
    } else if (cntRecord == historySize)
        free(history[newRecord]);
    history[newRecord] = (char*)malloc((strlen(command_value) + 1) * sizeof(char));
    strncpy(history[newRecord], command_value, (strlen(command_value) + 1));

    cntRecord = (cntRecord + 1 < historySize) ? (cntRecord + 1) : historySize;
}

void prevRecord()
{
    if (cntRecord < 2)
        return;
    memset(command_value, ' ', strlen(command_value));
    command_value[strlen(command_value) + 1] = '\0';
    CIO_printf("\r  %s", command_value);
    curRecord = (curRecord > 0) ? (curRecord - 1) : (cntRecord - 1);
    memcpy(command_value, history[curRecord], strlen(history[curRecord]) + 1);
    CIO_printf("\r> %s", command_value);
    commandBufferIndex = strlen(history[curRecord]);
}

void nextRecord()
{
    if (cntRecord < 2)
        return;
    // printHistory();
    memset(command_value, ' ', strlen(command_value));
    command_value[strlen(command_value) + 1] = '\0';
    CIO_printf("\r  %s", command_value);
    curRecord = (curRecord + 1) % cntRecord;
    memcpy(command_value, history[curRecord], strlen(history[curRecord]) + 1);
    CIO_printf("\r> %s", command_value);
    commandBufferIndex = strlen(history[curRecord]);
}

void printHistory()
{

    if (historySize == 0) {
        return;
    }
    int i = newRecord + 1;
    if (cntRecord == historySize) {
        if (!tempSaved) {
            i++;
        }
        for (; i < cntRecord; i++) {
            CIO_printf(" - %s\r\n", history[i]);
        }
    }
    for (int i = 0; i < ((tempSaved) ? (newRecord) : (newRecord + 1)); i++) {
        CIO_printf(" - %s\r\n", history[i]);
    }
    CIO_printf("\r\n");
}

void CIO_clearMemory()
{
    free(CIO_outputBuffer);
    if (unitCnt > 0)
        free(unitTbl);
}
