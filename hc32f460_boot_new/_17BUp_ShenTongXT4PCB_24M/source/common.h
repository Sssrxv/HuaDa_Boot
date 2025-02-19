#ifndef __COMMON_H__
#define __COMMON_H_

/*==================================================================================================
                                         MISRA VIOLATIONS
==================================================================================================*/

/**
* @page misra_violations MISRA-C:2004 violations
*
* @section common_types_h_REF_1
* Violates MISRA 2004 Advisory Rule 19.7, A function should be used in preference to a function-like macro.
* Function-like macros does not support type control of parameters but inline functions usage causes
* problems with some kind of compilers. Was also checked that macros are used correctly!
*/

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif

#ifndef FALSE
#define FALSE 0U
#endif

#ifndef TRUE
#define TRUE 1U
#endif

#ifndef LITTLE
#define LITTLE 1
#endif

#ifndef BIG
#define BIG 2
#endif

#ifndef MACHINE_ENDIAN
#define MACHINE_ENDIAN LITTLE
#endif

/* Definition of the platform specific types */
typedef signed char sint8_t;
typedef signed short sint16_t;
typedef signed int sint32_t;
typedef unsigned char boolean;
/* Needed by the PTPDRV for timestamps storage and calculations */
typedef unsigned long long uint64;
typedef signed long long sint64_t;

#if (!defined TYPEDEFS_H) && (! defined _STDINT) && (! defined _SYS__STDINT_H ) && (!defined _EWL_CSTDINT)/* ewl_library & S32K SDK - workaround for typedefs collisions */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef volatile unsigned char vuint8_t;
typedef volatile unsigned short vuint16_t;
typedef volatile unsigned int vuint32_t;
typedef volatile char vint8_t;
typedef volatile short vint16_t;
typedef volatile int vint32_t;
#endif  /* _TYPEDEFS_H_ */

/* Old type definitions used by FECLLD */
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
#if(!defined TYPEDEFS_H) && (! defined _STDINT) && (! defined _SYS__STDINT_H) && (!defined _EWL_CSTDINT)/* eCos & S32K SDK- workaround for typedefs collisions */
typedef sint64_t int64_t;
typedef sint32_t int32_t;
typedef sint16_t int16_t;
typedef sint8_t int8_t;
#endif  /* _TYPEDEFS_H_ */
typedef sint8_t sint8;
typedef sint16_t sint16;
typedef sint32_t sint32;

//#if(!defined TYPEDEFS_H) && (!defined _STDINT) /* eCos & S32K SDK - workaround for typedefs collisions */
//    typedef unsigned long long uint64_t;
//#endif

typedef int int_t;
typedef char char_t;
typedef unsigned int uint_t;
typedef double float64_t;
typedef unsigned int uaddr_t;

/* -------------------- Global interrupt define -------------------- */
// #define disableAllInterrupts() __disable_irq()
// #define enableAllInterrupts() __enable_irq()

#define DisableAllInterrupts()
#define EnableAllInterrupts()

/* -------------------- FIFO Configuration -------------------- */
/* RX message from BUS FIFO ID */
#define RX_BUS_FIFO         ('r')       /* RX bus FIFO */
#define RX_BUS_FIFO_LEN     (300)        /* RX BUS FIFO length */

/* TX message to BUS FIFO ID */
#define TX_BUS_FIFO         ('t')       /* RX bus FIFO */
#define TX_BUS_FIFO_LEN     (100u)       /* RX BUS FIFO length */


/* -------------------- Core Define -------------------- */
/* This macro must >= 1 */
#define CORE_NO (1u)
#if (defined CORE_NO) && (CORE_NO < 1)
#undef CORE_NO
#define CORE_NO (1u)
#elif (!defined CORE_NO)
#define CORE_NO (1u)
#endif

/* -------------------------------------------- END OF FILE -------------------------------------------- */


#endif

