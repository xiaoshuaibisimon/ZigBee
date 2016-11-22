#ifndef __SD_DRIVE_H__
#define __SD_DRIVE_H__
#include<ioCC2530.h>
void ON_32MOSC();
void ON_16MRC();
void DriveCfg();

#define OFF_LED0  P1_0=1
#define ON_LED0   P1_0=0
#define OFF_LED2  P0_1=1
#define ON_LED2   P0_1=0
#define OFF_LED3  P0_4=1
#define ON_LED3   P0_4=0

#define OFF_BEE   P0_0=0
#define ON_BEE    P0_0=1

#define LXCHANGER  P0_6
#endif