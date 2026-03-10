/*
 * consoleIO.h
 *
 * Created: 01.12.2025 10:31:31
 *  Author: dhamatnurov
 */

#ifndef CONSOLEIO_H_
#define CONSOLEIO_H_

#include <stdint.h>

#include "industry/CIO.h"

typedef struct stConsoleInfo {
    const char* deviceName;
    const char* deviceVersion;
    const char* timestamp;
    const char* author;
} consoleInfo;

typedef void (*getDataFun)(char* buffer, void* arg);
typedef struct stOutputConsole {
    const char* name;
    const char* param;
    const char* description;
    getDataFun getData;
    void* getDataParam;
    uint8_t type;
} outputConsole;

#define CIO_ADC_TYPE 0x00
#define CIO_GPIO_TYPE 0x01
#define CIO_HUMI_TYPE 0x02
#define CIO_MASTER_TYPE 0x03
#define CIO_PRESS_TYPE 0x04
#define CIO_SLAVE_TYPE 0x05
#define CIO_TEMP_TYPE 0x06

#define CIO_OUTPUT_BUFFER_SIZE 120
#define COMMAND_SIZE 120

void CIO_printf(const char*, ...);
typedef void (*readOutputBuffer)(char* outputBuffer);
void CIO_initIO(readOutputBuffer writeFun);
void CIO_displayBaseInfo(char* deviceName, char* deviceVersion, char* timestamp, char* author);
void CIO_readBuffer(void);
int CIO_printConsoleUnit(const char* name, const char* param, const char* description, const char* data, const char* type);
void CIO_tryToParseCommand(char* command);
void CIO_helpCommand(void);
void CIO_endProgramm(void);

#endif /* CONSOLEIO_H_ */
