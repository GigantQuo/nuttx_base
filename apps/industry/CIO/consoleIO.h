/*
 * consoleIO.h
 *
 * Created: 01.12.2025 10:31:31
 *  Author: dhamatnurov
 */

#ifndef CONSOLEIO_H_
#define CONSOLEIO_H_

#include <stdint.h>

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
void CIO_initIO(char* inputBuffer, int inputBufferSize, readOutputBuffer writeFun);
void CIO_baseInfo(char* deviceName, char* deviceVersion, char* timestamp, char* author);
void CIO_readBuffer(void);
int CIO_addConsoleUnit(const char* name, const char* param, const char* description, getDataFun getData, void* getDataParam, uint8_t type);
void CIO_tryToParseCommand(char* command);
void enableHistory(int);
void CIO_clearMemory(void);

#endif /* CONSOLEIO_H_ */
