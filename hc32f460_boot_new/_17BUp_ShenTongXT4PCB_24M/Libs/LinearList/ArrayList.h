#ifndef __ARRAYLIST_H
#define __ARRAYLIST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "USARTLog.h"

#define NEED_MCU_USART_LOG	1

typedef enum {
	Mistake,
	noMistake
} ArrayListFuncRunState;

typedef unsigned 			 char u8Array;
typedef unsigned short int  u16Array;
typedef unsigned       int  u32Array;
//typedef unsigned       __INT64 u64Array;


ArrayListFuncRunState InitU8ArrayList(u8Array inputU8Array[],unsigned int arrayLens,int initValue);
ArrayListFuncRunState CmpTwoU8ArrayList(u8Array u8Array_A[],u8Array u8Array_B[],unsigned int arrayLens,char *LogMessage);

#endif

