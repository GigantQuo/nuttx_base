/****************************************************************************
 * apps/industry/CIO/CIO_main.c
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "consoleIOForNuttx.h"
#include <debug.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nuttx/analog/adc.h>
#include <nuttx/timers/watchdog.h>

#include "CIO.h"

#if defined(CONFIG_INDUSTRY_CIO)
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

int* adc_buf;

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

void readOutput(char* outputBuffer)
{
    printf(outputBuffer);
    usleep(500);
}

char* getADCFun(char* dataBuf, int chanel, int div)
{
    int mV = ADC2MVOLT(adc_buf[chanel]) * div;
    snprintf(dataBuf, 13, "%i.%iV", mV / 1000, (mV % 1000) / 100);
    return dataBuf;
}

void printADC(void)
{
    char dataBuf[13];
#ifdef CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S
    CIO_printConsoleUnit("ADC_5V0   ", "PB0 /  8", "ADC 5.0V", getADCFun(dataBuf, 8, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3   ", "PB1 /  9", "ADC 3.3V", getADCFun(dataBuf, 9, 2), "ADC");
    CIO_printConsoleUnit("ADC_1V8   ", "PB2 / 10", "ADC 1.8V", getADCFun(dataBuf, 10, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V5   ", "PB3 / 11", "ADC 1.5V", getADCFun(dataBuf, 11, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_0", "PB4 / 12", "ADC 1.02V", getADCFun(dataBuf, 12, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_1", "PB5 / 13", "ADC 1.02V", getADCFun(dataBuf, 13, 1), "ADC");
    CIO_printConsoleUnit("ADC_0V9   ", "PB6 / 14", "ADC 0.9V", getADCFun(dataBuf, 14, 1), "ADC");
    CIO_printConsoleUnit("ADC_VBAT  ", "PB7 / 15", "BAT ADC", getADCFun(dataBuf, 15, 11), "ADC");
#elifdef(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    CIO_printConsoleUnit("ADC_5V0   ", "PB0 /  8", "ADC 5.0V", getADCFun(dataBuf, 8, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3   ", "PB1 /  9", "ADC 3.3V", getADCFun(dataBuf, 9, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3_0 ", "PB2 / 10", "ADC 3.3V", getADCFun(dataBuf, 10, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3_1 ", "PB3 / 11", "ADC 3.3V", getADCFun(dataBuf, 11, 2), "ADC");
    CIO_printConsoleUnit("ADC_1V8   ", "PB4 / 12", "ADC 1.8V", getADCFun(dataBuf, 12, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V5   ", "PB5 / 13", "ADC 1.5V", getADCFun(dataBuf, 13, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_0", "PB6 / 14", "ADC 1.02V", getADCFun(dataBuf, 14, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_1", "PB7 / 15", "ADC 1.02V", getADCFun(dataBuf, 15, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V0_0 ", "PB8 /  2", "ADC 1.0V", getADCFun(dataBuf, 2, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V0_1 ", "PB9 /  3", "ADC 1.0V", getADCFun(dataBuf, 3, 1), "ADC");
    CIO_printConsoleUnit("ADC_VBAT  ", "PA6 /  6", "BAT ADC", getADCFun(dataBuf, 6, 11), "ADC");
#elifdef CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S
    CIO_printConsoleUnit("ADC_5V0   ", "PB0 /  8", "ADC 5.0V", getADCFun(dataBuf, 8, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3   ", "PB1 /  9", "ADC 3.3V", getADCFun(dataBuf, 9, 2), "ADC");
    CIO_printConsoleUnit("ADC_1V8   ", "PB2 / 10", "ADC 1.8V", getADCFun(dataBuf, 10, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V5   ", "PB3 / 11", "ADC 1.5V", getADCFun(dataBuf, 11, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_0", "PB4 / 12", "ADC 1.02V", getADCFun(dataBuf, 12, 1), "ADC");
    CIO_printConsoleUnit("ADC_0V9   ", "PB5 / 13", "ADC 0.9V", getADCFun(dataBuf, 13, 1), "ADC");
    CIO_printConsoleUnit("ADC_VBAT  ", "PB6 / 14", "BAT ADC", getADCFun(dataBuf, 14, 11), "ADC");
#elifdef(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
    CIO_printConsoleUnit("ADC_5V0   ", "PB0 /  8", "ADC 5.0V", getADCFun(dataBuf, 8, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3   ", "PB1 /  9", "ADC 3.3V", getADCFun(dataBuf, 9, 2), "ADC");
    CIO_printConsoleUnit("ADC_3V3_0 ", "PB3 / 10", "ADC 3.3V", getADCFun(dataBuf, 11, 2), "ADC");
    CIO_printConsoleUnit("ADC_1V8   ", "PB4 / 12", "ADC 1.8V", getADCFun(dataBuf, 12, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V5   ", "PB5 / 13", "ADC 1.5V", getADCFun(dataBuf, 13, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V02_0", "PB6 / 14", "ADC 1.02V", getADCFun(dataBuf, 14, 1), "ADC");
    CIO_printConsoleUnit("ADC_1V0_0 ", "PB8 /  2", "ADC 1.0V", getADCFun(dataBuf, 2, 1), "ADC");
    CIO_printConsoleUnit("ADC_VBAT  ", "PA6 /  6", "BAT ADC", getADCFun(dataBuf, 6, 11), "ADC");
#endif
}

int get_adc_data(void)
{
    ssize_t ret;

    struct adc_msg_s voltage;

    char* devpath;
    int fd;

    devpath = "/dev/adc0";

    fd = open(devpath, O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/adc0: %d\n\r", ret);
        return ret;
    }

    for (uint8_t i = 0; i < (BOARD_ADC_NUM_CHANNELS + 1); i++) {

        ret = read(fd, &voltage, sizeof(voltage));
        if (ret < 0) {
            printf("CIO: ERROR: Failed to read /dev/adc0: %d\n\r", ret);
            close(fd);
            return ret;
        }

        if (i == 0) {
            continue;
        }

        if (voltage.am_channel == BOARD_ADC_VBAT_CH) {
            voltage.am_data += 1600;
        }

        adc_buf[voltage.am_channel] = voltage.am_data;
    }
    close(fd);

    ret = OK;
    return ret;
}

char* getGPIOFun(char* buf, char* data, int bit)
{
    int ret = (data[bit / 8] & (1 << (bit % 8))) >> (bit % 8);
    snprintf(buf, 4, "%i", ret);
    return buf;
}

int get_gpinp_data(void)
{
    ssize_t ret;
    int fd;
    fd = open("/dev/gpinp0", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpinp0: %d\n\r", ret);
        return ret;
    }
    char buf[2];
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpinp0: %d\n\r", ret);
        close(fd);
        return ret;
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    char retS[4];
    CIO_printConsoleUnit("PWR1_PRE  ", "PA0  / 0", "Power 1 present", getGPIOFun(retS, buf, 0), "GPINP");
    CIO_printConsoleUnit("PWR1_AC_OK", "PA2  / 0", "Power 1 AC ok", getGPIOFun(retS, buf, 1), "GPINP");
    CIO_printConsoleUnit("PWR1_ALERT", "PA4  / 0", "Power 1 alert", getGPIOFun(retS, buf, 2), "GPINP");
    CIO_printConsoleUnit("PWR1_PW_OK", "PA6  / 0", "Power 1 power ok", getGPIOFun(retS, buf, 3), "GPINP");
    CIO_printConsoleUnit("PWR2_PRE  ", "PA1  / 0", "Power 2 present", getGPIOFun(retS, buf, 4), "GPINP");
    CIO_printConsoleUnit("PWR2_AC_OK", "PA3  / 0", "Power 2 AC ok", getGPIOFun(retS, buf, 5), "GPINP");
    CIO_printConsoleUnit("PWR2_ALERT", "PA5  / 0", "Power 2 alert", getGPIOFun(retS, buf, 6), "GPINP");
    CIO_printConsoleUnit("PWR2_PW_OK", "PA7  / 0", "Power 2 power ok", getGPIOFun(retS, buf, 7), "GPINP");
    CIO_printConsoleUnit("AC_OK     ", "PA10 / 0", "AC ok with battery module", getGPIOFun(retS, buf, 8), "GPINP");
    CIO_printConsoleUnit("BAT_LOW   ", "PA11 / 0", "Battery charge low", getGPIOFun(retS, buf, 9), "GPINP");

#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS) || defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
    char retS[4];
    CIO_printConsoleUnit("PWR1_PRE", "PA0  / 0", "Power 1 present", getGPIOFun(retS, buf, 0), "GPINP");
    CIO_printConsoleUnit("PWR1_PG ", "PA2  / 0", "Power 1 ", getGPIOFun(retS, buf, 1), "GPINP");
    CIO_printConsoleUnit("PWR2_PRE", "PA1  / 0", "Power 2 present", getGPIOFun(retS, buf, 2), "GPINP");
    CIO_printConsoleUnit("PWR2_PG ", "PA3  / 0", "Power 2 ", getGPIOFun(retS, buf, 3), "GPINP");
    CIO_printConsoleUnit("AC_OK   ", "PA10 / 0", "AC ok with battery module", getGPIOFun(retS, buf, 4), "GPINP");
    CIO_printConsoleUnit("BAT_LOW ", "PA11 / 0", "Battery charge low", getGPIOFun(retS, buf, 5), "GPINP");
#endif

    close(fd);

    ret = OK;
    return ret;
}

int get_gpout_data(void)
{
    ssize_t ret;
    int fd;
    char buf[2];
    char retS[2];

    fd = open("/dev/gpout0", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpout0: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpout0: %d\n\r", ret);
        close(fd);
        return ret;
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
    CIO_printConsoleUnit("PWR1_ON  ", "PA8  / 0", "Power 1 on", getGPIOFun(retS, buf, 7), "GPOUT");
    CIO_printConsoleUnit("PWR2_ON  ", "PA9  / 0", "Power 2 on", getGPIOFun(retS, buf, 8), "GPOUT");
    CIO_printConsoleUnit("5V0_EN   ", "PB12 / 0", "5.0V on", getGPIOFun(retS, buf, 6), "GPOUT");
    CIO_printConsoleUnit("3V3_EN   ", "PB13 / 0", "3.3V on", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("1V8_EN   ", "PB14 / 0", "1.8V on", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("1V5_EN   ", "PB15 / 0", "1.5V on", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_0", "PB16 / 0", "1.02V_0 on", getGPIOFun(retS, buf, 3), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_1", "PB17 / 0", "1.02V_1 on", getGPIOFun(retS, buf, 4), "GPOUT");
    CIO_printConsoleUnit("0V9_EN   ", "PB22 / 0", "0.9V on", getGPIOFun(retS, buf, 5), "GPOUT");
    CIO_printConsoleUnit("CONTROL  ", "PA14 / 0", "Battery power on", getGPIOFun(retS, buf, 9), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    CIO_printConsoleUnit("5V0_EN   ", "PB14 / 0", "5.0V on", getGPIOFun(retS, buf, 5), "GPOUT");
    CIO_printConsoleUnit("3V3_EN   ", "PB15 / 0", "3.3V on", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("3V3_0_EN ", "PB16 / 0", "3.3V_0 on", getGPIOFun(retS, buf, 6), "GPOUT");
    CIO_printConsoleUnit("3V3_1_EN ", "PB22 / 0", "3.3V_1 on", getGPIOFun(retS, buf, 7), "GPOUT");
    CIO_printConsoleUnit("1V8_EN   ", "PB10 / 0", "1.8V on", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("1V5_EN   ", "PB11 / 0", "1.5V on", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_0", "PB13 / 0", "1.02V_0 on", getGPIOFun(retS, buf, 3), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_1", "PB23 / 0", "1.02V_1 on", getGPIOFun(retS, buf, 4), "GPOUT");
    CIO_printConsoleUnit("1V0_EN_0 ", "PB12 / 0", "1.02V_0 on", getGPIOFun(retS, buf, 8), "GPOUT");
    CIO_printConsoleUnit("1V0_EN_1 ", "PB17 / 0", "1.02V_1 on", getGPIOFun(retS, buf, 9), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    CIO_printConsoleUnit("PWR1_ON  ", "PA8  / 0", "Power 1 on", getGPIOFun(retS, buf, 6), "GPOUT");
    CIO_printConsoleUnit("PWR2_ON  ", "PA9  / 0", "Power 2 on", getGPIOFun(retS, buf, 7), "GPOUT");
    CIO_printConsoleUnit("5V0_EN   ", "PB12 / 0", "5.0V on", getGPIOFun(retS, buf, 5), "GPOUT");
    CIO_printConsoleUnit("3V3_EN   ", "PB13 / 0", "3.3V on", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("1V8_EN   ", "PB14 / 0", "1.8V on", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("1V5_EN   ", "PB15 / 0", "1.5V on", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_0", "PB16 / 0", "1.02V on", getGPIOFun(retS, buf, 3), "GPOUT");
    CIO_printConsoleUnit("0V9_EN   ", "PB17 / 0", "0.9V on", getGPIOFun(retS, buf, 4), "GPOUT");
    CIO_printConsoleUnit("CONTROL  ", "PA14 / 0", "Battery power on", getGPIOFun(retS, buf, 8), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    CIO_printConsoleUnit("5V0_EN   ", "PB14 / 0", "5.0V on", getGPIOFun(retS, buf, 4), "GPOUT");
    CIO_printConsoleUnit("3V3_EN   ", "PB15 / 0", "3.3V on", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("3V3_0_EN ", "PB16 / 0", "3.3V_0 on", getGPIOFun(retS, buf, 5), "GPOUT");
    CIO_printConsoleUnit("1V8_EN   ", "PB10 / 0", "1.8V on", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("1V5_EN   ", "PB11 / 0", "1.5V on", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("1V02_EN_0", "PB13 / 0", "1.02V_0 on", getGPIOFun(retS, buf, 3), "GPOUT");
    CIO_printConsoleUnit("1V0_EN_0 ", "PB12 / 0", "1.02V_0 on", getGPIOFun(retS, buf, 6), "GPOUT");
#endif
    close(fd);

    fd = open("/dev/gpout1", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpout1: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpout1: %d\n\r", ret);
        close(fd);
        return ret;
    }

#if defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_S)
    CIO_printConsoleUnit("POWER_GOOD", "PA24 / 1", "LED POWER_GOOD", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("ALARM     ", "PA23 / 1", "LED ALARM", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW0 ", "PB23 / 1", "Reset PonCat 0", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW1 ", "PB30 / 1", "Reset PonCat 1", getGPIOFun(retS, buf, 3), "GPOUT");
    CIO_printConsoleUnit("PW_OK_TCA ", "PB31 / 1", "Reset TCA", getGPIOFun(retS, buf, 4), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_48GE_FS)
    CIO_printConsoleUnit("POWER_GOOD", "PA24 / 1", "LED POWER_GOOD", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("ALARM     ", "PA23 / 1", "LED ALARM", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW0 ", "PB23 / 1", "Reset PonCat 0", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW1 ", "PB30 / 1", "Reset PonCat 1", getGPIOFun(retS, buf, 3), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_S)
    CIO_printConsoleUnit("POWER_GOOD", "PA24 / 1", "LED POWER_GOOD", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("ALARM     ", "PA23 / 1", "LED ALARM", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW0 ", "PB23 / 1", "Reset PonCat 0", getGPIOFun(retS, buf, 2), "GPOUT");
    CIO_printConsoleUnit("PW_OK_TCA ", "PB31 / 1", "Reset TCA", getGPIOFun(retS, buf, 3), "GPOUT");
#elif defined(CONFIG_ARCH_BOARD_APC3_ARLAN_24GE_FS)
    CIO_printConsoleUnit("POWER_GOOD", "PA24 / 1", "LED POWER_GOOD", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("ALARM     ", "PA23 / 1", "LED ALARM", getGPIOFun(retS, buf, 1), "GPOUT");
    CIO_printConsoleUnit("PW_OK_SW0 ", "PB23 / 1", "Reset PonCat 0", getGPIOFun(retS, buf, 2), "GPOUT");
#endif
    close(fd);

    fd = open("/dev/gpout2", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpout2: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpout2: %d\n\r", ret);
        close(fd);
        return ret;
    }

    CIO_printConsoleUnit("LED_BTN_1", "PA19 / 2", "LED button 1", getGPIOFun(retS, buf, 0), "GPOUT");
    CIO_printConsoleUnit("LED_BTN_2", "PA21 / 2", "LED button 2", getGPIOFun(retS, buf, 1), "GPOUT");
    close(fd);

    fd = open("/dev/gpout3", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpout3: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpout3: %d\n\r", ret);
        close(fd);
        return ret;
    }

    CIO_printConsoleUnit("RESET", "PA25 / 3", "Reset system", getGPIOFun(retS, buf, 0), "GPOUT");
    close(fd);

    ret = OK;
    return ret;
}

int get_gpint_data(void)
{
    ssize_t ret;
    int fd;
    char buf[1];
    char retS[2];

    fd = open("/dev/gpint0", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpint0: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpint0: %d\n\r", ret);
        close(fd);
        return ret;
    }

    CIO_printConsoleUnit("BTN1", "PA18 / 0", "Button 1 interrupt", getGPIOFun(retS, buf, 0), "GPINT");
    CIO_printConsoleUnit("BTN2", "PA20 / 0", "Button 2 interrupt", getGPIOFun(retS, buf, 1), "GPINT");
    close(fd);

    fd = open("/dev/gpint1", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/gpint1: %d\n\r", ret);
        return ret;
    }
    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/gpint1: %d\n\r", ret);
        close(fd);
        return ret;
    }

    CIO_printConsoleUnit("STANDBY", "PA22 / 1", "Standby interrupt", getGPIOFun(retS, buf, 0), "GPINT");
    close(fd);

    ret = OK;
    return ret;
}

int get_i2c_data(void)
{
    ssize_t ret = OK;
    struct stat buffer;
    // ret = (stat("/dev/ttyS", &buffer));
    // printf("read stat: %i\r\n", ret);
    ret = (stat("/dev/i2cslv1", &buffer));

    CIO_printConsoleUnit("I2C_SLAVE1", "PA8/PA9", "I2C slave to PonCat3", ret >= 0 ? "OK" : "BAD", "I2C");

    return ret;
}

int get_uart_data(void)
{
    ssize_t ret = OK;
    struct stat buffer;
    // ret = (stat("/dev/ttyS", &buffer));
    // printf("read stat: %i\r\n", ret);
    ret = (stat("/dev/ttyS0", &buffer));

    CIO_printConsoleUnit("UART2", "PA12/PA13", "UART for console", ret >= 0 ? "OK" : "BAD", "UART");

    return ret;
}

char* getWDFun(char* buf, struct watchdog_status_s wdt_status)
{
    if ((wdt_status.flags & WDFLAGS_ACTIVE) > 0) {
        snprintf(buf, 12, "%u.%u s", (unsigned int)(wdt_status.timeout / 1000), (unsigned int)(wdt_status.timeout % 1000 / 100));
    } else {
        snprintf(buf, 12, "OFF");
    }
    return buf;
}

int get_watchdog_data(void)
{
    ssize_t ret;
    int fd;
    char retS[12];
    struct watchdog_status_s wdt_status;

    fd = open("/dev/watchdog0", O_RDONLY);
    if (fd < 0) {
        ret = fd;
        printf("CIO: ERROR: Failed to open /dev/watchdog0: %d\n\r", ret);
        return ret;
    }
    ret = ioctl(fd, WDIOC_GETSTATUS, &wdt_status);
    if (ret < 0) {
        printf("CIO: ERROR: Failed to read /dev/watchdog0: %d\n\r", ret);
        close(fd);
        return ret;
    }

    CIO_printConsoleUnit("WATCHDOG0", "", "Watchdog timer", getWDFun(retS, wdt_status), "WATCHDOG");
    close(fd);

    ret = OK;
    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int cio_main(int argc, FAR char* argv[])
{
    CIO_initIO(readOutput);
    if (argc <= 1) {
        CIO_displayBaseInfo(PRJ_MODEL, PRJ_VERSION, __DATE__, PRJ_AUTHOR);
        return 0;
    }

    if (strncmp(argv[1], "desc", COMMAND_SIZE) == 0) {
        CIO_displayBaseInfo(PRJ_MODEL, PRJ_VERSION, __DATE__, PRJ_AUTHOR);
    } else if (strncmp(argv[1], "help", COMMAND_SIZE) == 0 || strncmp(argv[1], "h", COMMAND_SIZE) == 0) {
        CIO_helpCommand();
    } else if (strncmp(argv[1], "adc", COMMAND_SIZE) == 0) {
        int adc[CIO_MAX_ADC_CHANNEL_NUM];
        adc_buf = adc;
        get_adc_data();
        printADC();
    } else if (strncmp(argv[1], "gpinp", COMMAND_SIZE) == 0) {
        get_gpinp_data();
    } else if (strncmp(argv[1], "gpint", COMMAND_SIZE) == 0) {
        get_gpint_data();
    } else if (strncmp(argv[1], "gpout", COMMAND_SIZE) == 0) {
        get_gpout_data();
    } else if (strncmp(argv[1], "gpio", COMMAND_SIZE) == 0) {
        get_gpinp_data();
        get_gpout_data();
        get_gpint_data();
    } else if (strncmp(argv[1], "i2c", COMMAND_SIZE) == 0) {
        get_i2c_data();
    } else if (strncmp(argv[1], "uart", COMMAND_SIZE) == 0) {
        get_uart_data();
    } else if (strncmp(argv[1], "sercom", COMMAND_SIZE) == 0) {
        get_i2c_data();
        get_uart_data();
    } else if (strncmp(argv[1], "wd", COMMAND_SIZE) == 0) {
        get_watchdog_data();
    } else if (strncmp(argv[1], "all", COMMAND_SIZE) == 0) {
        int adc[15];
        adc_buf = adc;
        get_adc_data();
        printADC();

        get_gpinp_data();
        get_gpout_data();
        get_gpint_data();

        get_i2c_data();
        get_uart_data();
        get_watchdog_data();
    }

    CIO_endProgramm();
    return 0;
}

#endif /* CONFIG_INDUSTRY_CIO */
