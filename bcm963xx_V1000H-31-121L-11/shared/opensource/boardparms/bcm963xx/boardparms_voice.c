/*
    Copyright 2000-2010 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/                       

/**************************************************************************
* File Name  : boardparms_voice.c
*
* Description: This file contains the implementation for the BCM63xx board
*              parameter voice access functions.
*
***************************************************************************/

/* ---- Include Files ---------------------------------------------------- */

#include "boardparms_voice.h"

/* ---- Public Variables ------------------------------------------------- */

/* ---- Private Constants and Types -------------------------------------- */

/* Always end the device list in VOICE_BOARD_PARMS with this macro */
#define BP_NULL_DEVICE_MACRO     \
{                                \
   BP_VD_NONE,                   \
   {  0, BP_NOT_DEFINED },       \
   0,                            \
   BP_NOT_DEFINED,               \
   {                             \
      { BP_VOICE_CHANNEL_INACTIVE, BP_VCTYPE_NONE, BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE, BP_VOICE_CHANNEL_NARROWBAND, BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS, BP_VOICE_CHANNEL_ENDIAN_BIG, BP_TIMESLOT_INVALID, BP_TIMESLOT_INVALID }, \
      { BP_VOICE_CHANNEL_INACTIVE, BP_VCTYPE_NONE, BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE, BP_VOICE_CHANNEL_NARROWBAND, BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS, BP_VOICE_CHANNEL_ENDIAN_BIG, BP_TIMESLOT_INVALID, BP_TIMESLOT_INVALID }, \
   }                             \
}

#define BP_NULL_CHANNEL_MACRO             \
{  BP_VOICE_CHANNEL_INACTIVE,             \
   BP_VCTYPE_NONE,                        \
   BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,    \
   BP_VOICE_CHANNEL_NARROWBAND,           \
   BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,   \
   BP_VOICE_CHANNEL_ENDIAN_BIG,           \
   BP_TIMESLOT_INVALID,                   \
   BP_TIMESLOT_INVALID                    \
},


#if defined(_BCM96328_) || defined(CONFIG_BCM96328)

VOICE_BOARD_PARMS voiceBoard_96328AVNG_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNG_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};


VOICE_BOARD_PARMS voiceBoard_96328AVNG_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,    

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNG_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }    
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNG_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            6,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,   

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNG_SI32267_NTR =
{
   VOICECFG_SI32267_NTR_STR,   /* szBoardId */
   "96328avng",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            6,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,   

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNG_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96328avng",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "96328avngr",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96328avngr",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,       /* Reset line shared to one line only for AVNGR board */

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96328avngr",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_ALAW,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_ALAW,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "96328avngr",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            6,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_ALAW,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_ALAW,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,   

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "96328avngr",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_LE88536_TH =
{
   VOICECFG_LE88536_TH_STR,   /* szBoardId */
   "96328avngr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_LE88264_TH =
{
   VOICECFG_LE88264_TH_STR,   /* szBoardId */
   "96328avngr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88264,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96328AVNGR_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96328avngr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "963293epon",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "963293epon",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963293epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "963293epon",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,       /* Reset line shared to one line only for AVNGR board */

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* No DAA channel on 63293epon board */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963293epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_93715RV_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "93715rv",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_93715RV_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "93715rv",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_93715RV_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "93715rv",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_93715RV_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "93715rv",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,       /* Reset line shared to one line only for AVNGR board */

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* No DAA channel on 63293epon board */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_GPHY_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "963293epon_gphy",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_GPHY_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "963293epon_gphy",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_GPHY_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963293epon_gphy",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963293EPON_GPHY_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "963293epon_gphy",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,       /* Reset line shared to one line only for AVNGR board */

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* No DAA channel on 63293epon board */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

static PVOICE_BOARD_PARMS g_VoiceBoardParms[] = 
{
   &voiceBoard_96328AVNG_VE890_INVBOOST,
   &voiceBoard_96328AVNG_SI3217X,
   &voiceBoard_96328AVNG_LE88506,
#ifdef SI32261ENABLE
   &voiceBoard_96328AVNG_SI32261,
#endif   
#ifdef SI32267ENABLE
   &voiceBoard_96328AVNG_SI32267,
   &voiceBoard_96328AVNG_SI32267_NTR,   
#endif
   &voiceBoard_96328AVNG_LE88266,   

   &voiceBoard_96328AVNGR_SI32176,
   &voiceBoard_96328AVNGR_SI3217X,
   &voiceBoard_96328AVNGR_LE89116,
   &voiceBoard_96328AVNGR_LE88536_TH,
   &voiceBoard_96328AVNGR_LE88264_TH,
   &voiceBoard_96328AVNGR_SI32261,
   &voiceBoard_96328AVNGR_SI32267,
   &voiceBoard_96328AVNGR_LE88266,   

   &voiceBoard_963293EPON_SI32176,
   &voiceBoard_963293EPON_SI3217X_NOFXO,
   &voiceBoard_963293EPON_LE89116,
   &voiceBoard_963293EPON_LE88506,
   &voiceBoard_963293EPON_LE88266,

   &voiceBoard_963293EPON_GPHY_SI32176,
   &voiceBoard_963293EPON_GPHY_LE89116,
   &voiceBoard_963293EPON_GPHY_LE88506,
   &voiceBoard_963293EPON_GPHY_SI3217X_NOFXO,

   &voiceBoard_93715RV_SI32176,
   &voiceBoard_93715RV_SI3217X_NOFXO,
   &voiceBoard_93715RV_LE89116,
   &voiceBoard_93715RV_LE88506,
   0
};

#endif

#if defined(_BCM96362_) || defined(CONFIG_BCM96362)

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_NOSLIC =
{
   VOICECFG_NOSLIC_STR,   /* szBoardId */
   "96362ADVNgr",
   0,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96362ADVNgr",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "96362ADVNgr",
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI32178 =
{
   VOICECFG_SI32178_STR,   /* szBoardId */
   "96362ADVNgr",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 32176 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "96362ADVNgr",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE89316 =
{
   VOICECFG_LE89316_STR,   /* szBoardId */
   "96362ADVNgr",  
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE88536_TH =
{
   VOICECFG_LE88536_TH_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE88264_TH =
{
   VOICECFG_LE88264_TH_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88264,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96362ADVNgr",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "96362ADVNgr",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_VE890HV =
{
   VOICECFG_VE890HV_STR,   /* szBoardId */
   "96362ADVNgr",    
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89136,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89336,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNgr_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96362ADVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNGR2_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96362ADVNgr2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNGR2_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96362ADVNgr2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNGR2_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96362ADVNgr2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVNGR2_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96362ADVNgr2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RADVNGR2_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96362RADVNgr2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RADVNGR2_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96362RADVNgr2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RADVNGR2_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96362RADVNgr2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVN2XH_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96362ADVN2xh",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362ADVN2XH_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96362ADVN2xh",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_30_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_44_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};


VOICE_BOARD_PARMS voiceBoard_96362EPON_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "963629epon",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "963629epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 32176 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "963629epon",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_INVBOOST,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963629epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963629epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963629epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,   

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96362EPON_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963629epon",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361XF_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96361XF",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361XF_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96361XF",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RPVT_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96362RPVT",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_96362RPVT_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96362RPVT",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RPVT_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "96362RPVT",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,   

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96362RPVT_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96362RPVT",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RPVT_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96362RPVT",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_96362RPVT_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96362RPVT",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96362RPVT_LE88266x2_LE89010 =
{
   VOICECFG_LE88266x2_LE89010_STR,   /* szBoardId */
   "96362RPVT",   
   4,             /* numFxsLines */
   1,             /* numFxoLines */
   2,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },
      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89010,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_9_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_26_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on Le89010 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },



      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_35_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361I2_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96361I2",
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361I2_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96361I2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361I2_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "96361I2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361I2_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96361I2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_96361I2_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96361I2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96361I2_LE88266x2_LE89010 =
{
   VOICECFG_LE88266x2_LE89010_STR,   /* szBoardId */
   "96361I2",   
   4,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_28_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_31_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },
      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89010,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_9_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_29_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on Le89010 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },



      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_10_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

static PVOICE_BOARD_PARMS g_VoiceBoardParms[] = 
{
   &voiceBoard_96362ADVNgr_NOSLIC,
   &voiceBoard_96362ADVNgr_SI3217X,
   &voiceBoard_96362ADVNgr_SI32176,
   &voiceBoard_96362ADVNgr_SI32178,
   &voiceBoard_96362ADVNgr_SI3217X_NOFXO,   
   &voiceBoard_96362ADVNgr_VE890_INVBOOST,
   &voiceBoard_96362ADVNgr_LE89116,
   &voiceBoard_96362ADVNgr_LE89316,
   &voiceBoard_96362ADVNgr_LE88506,
   &voiceBoard_96362ADVNgr_LE88536_TH,
   &voiceBoard_96362ADVNgr_LE88264_TH,
#ifdef SI32261ENABLE
   &voiceBoard_96362ADVNgr_SI32261,
#endif
#ifdef SI32267ENABLE
   &voiceBoard_96362ADVNgr_SI32267,
#endif
   &voiceBoard_96362ADVNgr_VE890HV,
   &voiceBoard_96362ADVNgr_LE88266,

   &voiceBoard_96362ADVNGR2_SI3217X,
   &voiceBoard_96362ADVNGR2_LE88506,
   &voiceBoard_96362ADVNGR2_LE88266,   
   &voiceBoard_96362ADVNGR2_VE890_INVBOOST,
   
   &voiceBoard_96362RADVNGR2_SI3217X,
   &voiceBoard_96362RADVNGR2_LE88506,
   &voiceBoard_96362RADVNGR2_LE88266,  
   
   &voiceBoard_96362ADVN2XH_SI3217X,
   &voiceBoard_96362ADVN2XH_LE88506,
      
   /* EPON should not have any FXO related configs */
   &voiceBoard_96362EPON_SI32176,
   &voiceBoard_96362EPON_SI3217X_NOFXO,
   &voiceBoard_96362EPON_LE89116,
   &voiceBoard_96362EPON_LE88506,
#ifdef SI32261ENABLE
   &voiceBoard_96362EPON_SI32261,
#endif
#ifdef SI32267ENABLE
   &voiceBoard_96362EPON_SI32267,
#endif
   &voiceBoard_96362EPON_LE88266,

   &voiceBoard_96361XF_SI3217X,
   &voiceBoard_96361XF_LE88506,
   
   &voiceBoard_96362RPVT_SI3217X,
   &voiceBoard_96362RPVT_SI32267,
   &voiceBoard_96362RPVT_SI32261,
   &voiceBoard_96362RPVT_LE88266,   
   &voiceBoard_96362RPVT_VE890_INVBOOST,
   &voiceBoard_96362RPVT_LE88506,
   &voiceBoard_96362RPVT_LE88266x2_LE89010,
 
   &voiceBoard_96361I2_SI3217X,
   &voiceBoard_96361I2_SI32261,
   &voiceBoard_96361I2_LE88266,   
   &voiceBoard_96361I2_VE890_INVBOOST,
   &voiceBoard_96361I2_LE88506,
   &voiceBoard_96361I2_LE88266x2_LE89010,
        
   0
};

#endif

#if defined(_BCM963268_) || defined(CONFIG_BCM963268)

VOICE_BOARD_PARMS voiceBoard_963268SV_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963268SV1",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963268SV_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963268SV1",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268SV_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963268SV1",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268SV_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963268SV1",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268SV_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963268SV1",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268MBV_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963268MBV",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268MBV_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963268MBV",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963268MBV_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963268MBV",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268MBV_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963268MBV",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268MBV_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963268MBV",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268MBV_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963268MBV",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,
         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963268V30A",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_LE88264_TH =
{
   VOICECFG_LE88264_TH_STR,   /* szBoardId */
   "963268V30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88264,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_SI32260x2 =
{
   VOICECFG_SI32260x2_STR,   /* szBoardId */
   "963268V30A",   
   4,             /* numFxsLines */
   0,             /* numFxoLines */
   2,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
 
      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            1, 
            /* SPI GPIO */
            BP_NOT_DEFINED, 
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            4, 
            /* SPI GPIO */
            BP_GPIO_16_AL, 
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268V30A_LE88266x2_LE89010 =
{
   VOICECFG_LE88266x2_LE89010_STR,   /* szBoardId */
   "963268V30A",   
   4,             /* numFxsLines */
   1,             /* numFxoLines */
   2,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_51_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },
      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89010,

         /* SPI control */
         {  
            /* SPI dev id */
            5,
            /* SPI GPIO */
            BP_GPIO_17_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_50_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on Le89010 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },



      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963168MBV_17A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963168MBV_17A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963168MBV_17A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963168MBV_17A",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963168MBV_17A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_17A_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963168MBV_17A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963168MBV_30A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963168MBV_30A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963168MBV_30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963168MBV_30A",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963168MBV_30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MBV_30A_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963168MBV_30A",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

#if defined(AEI_VDSL_CUSTOMER_NCS)
VOICE_BOARD_PARMS voiceBoard_C2000A_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "C2000A",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_C2000A_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "C2000A",
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }    
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_31_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,   

   /* General-purpose flags */
   ( 0 )

};
#endif

VOICE_BOARD_PARMS voiceBoard_963168XH_VE890_INVBOOST=
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963168XH",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168XH_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963168XH",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_35_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963168MP_VE890_INVBOOST=
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963168MP",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168MP_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963168MP",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963168VX",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963168VX",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963168VX",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963168VX",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963168VX",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963168VX_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963168VX",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963268BU_VE890_INVBOOST=
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963268BU",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963268BU",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};
VOICE_BOARD_PARMS voiceBoard_963268BU_P300_VE890_INVBOOST=
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            4,
            /* SPI GPIO */
            BP_GPIO_16_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_15_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_SI32267 =
{
   VOICECFG_SI32267_STR,   /* szBoardId */
   "963268BU_P300",    
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32267,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS_ISO,

   /* General-purpose flags */
   ( BP_FLAG_ISI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_LE88536_ZSI =
{
   VOICECFG_LE88536_ZSI_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST, 

   /* General-purpose flags */
   ( BP_FLAG_ZSI_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
      
      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_963268BU_P300_LE88266 =
{
   VOICECFG_LE88266_STR,   /* szBoardId */
   "963268BU_P300",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   4,             /* numDectLines */
   1,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
          BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
     /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_19_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },
            

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_39_AH,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};


static PVOICE_BOARD_PARMS g_VoiceBoardParms[] = 
{
   &voiceBoard_963268SV_SI3217X,
   &voiceBoard_963268SV_VE890_INVBOOST,
   &voiceBoard_963268SV_LE88506,
   &voiceBoard_963268SV_SI32267,
   &voiceBoard_963268SV_LE88536_ZSI,
   
   &voiceBoard_963268MBV_SI3217X,
   &voiceBoard_963268MBV_VE890_INVBOOST,
   &voiceBoard_963268MBV_LE88506,
   &voiceBoard_963268MBV_SI32267,
   &voiceBoard_963268MBV_LE88536_ZSI,
   &voiceBoard_963268MBV_LE88266,   
   
   &voiceBoard_963268V30A_SI3217X,
   &voiceBoard_963268V30A_VE890_INVBOOST,
   &voiceBoard_963268V30A_LE88506,
   &voiceBoard_963268V30A_LE88536_ZSI,
   &voiceBoard_963268V30A_LE88266,
   &voiceBoard_963268V30A_SI32261,
   &voiceBoard_963268V30A_SI32267,
   &voiceBoard_963268V30A_LE88264_TH,
   &voiceBoard_963268V30A_SI32260x2,    
   &voiceBoard_963268V30A_LE88266x2_LE89010,

   &voiceBoard_963168MBV_17A_SI3217X,
   &voiceBoard_963168MBV_17A_VE890_INVBOOST,
   &voiceBoard_963168MBV_17A_LE88506,
   &voiceBoard_963168MBV_17A_SI32267,
   &voiceBoard_963168MBV_17A_LE88536_ZSI,
   &voiceBoard_963168MBV_17A_LE88266,      
   
   &voiceBoard_963168MBV_30A_SI3217X,
   &voiceBoard_963168MBV_30A_VE890_INVBOOST,
   &voiceBoard_963168MBV_30A_LE88506,
   &voiceBoard_963168MBV_30A_SI32267,
   &voiceBoard_963168MBV_30A_LE88536_ZSI,
   &voiceBoard_963168MBV_30A_LE88266,      

   &voiceBoard_963168XH_SI3217X,
   &voiceBoard_963168XH_VE890_INVBOOST, 
   &voiceBoard_963168XH_LE88506,
   &voiceBoard_963168XH_SI32267,
   &voiceBoard_963168XH_LE88536_ZSI,
   &voiceBoard_963168XH_SI32261,
   &voiceBoard_963168XH_LE88266,   
   
   &voiceBoard_963168MP_SI3217X,
   &voiceBoard_963168MP_VE890_INVBOOST, 
   &voiceBoard_963168MP_LE88506,
   &voiceBoard_963168MP_SI32267,
   &voiceBoard_963168MP_LE88536_ZSI,
   &voiceBoard_963168MP_SI32261,
   &voiceBoard_963168MP_LE88266,       
   
   &voiceBoard_963168VX_SI3217X,
   &voiceBoard_963168VX_VE890_INVBOOST,
   &voiceBoard_963168VX_LE88506,
   &voiceBoard_963168VX_SI32267,
   &voiceBoard_963168VX_LE88536_ZSI,
   &voiceBoard_963168VX_LE88266,    

   &voiceBoard_963268BU_SI3217X,
   &voiceBoard_963268BU_VE890_INVBOOST, 
   &voiceBoard_963268BU_LE88506,
   &voiceBoard_963268BU_SI32267,
   &voiceBoard_963268BU_LE88536_ZSI,
   &voiceBoard_963268BU_SI32261,
   &voiceBoard_963268BU_LE88266,    

   &voiceBoard_963268BU_P300_SI3217X,
   &voiceBoard_963268BU_P300_VE890_INVBOOST, 
   &voiceBoard_963268BU_P300_LE88506,
   &voiceBoard_963268BU_P300_SI32267,
   &voiceBoard_963268BU_P300_LE88536_ZSI,
   &voiceBoard_963268BU_P300_SI32261,
   &voiceBoard_963268BU_P300_LE88266,    

#if defined(AEI_VDSL_CUSTOMER_NCS)
   &voiceBoard_C2000A_SI3217X,
   &voiceBoard_C2000A_SI32261,
#endif

   0
};

#endif


#if defined(_BCM96368_) || defined(CONFIG_BCM96368)

VOICE_BOARD_PARMS voiceBoard_96368MVWG =
{
   VOICECFG_6368MVWG_STR,   /* szBoardId */
   "96368MVWG",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_10_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_GPIO_29_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_GPIO_10_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               5,
               5
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               6,
               6
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_3_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_BUCKBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "96368MB2G",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_3_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_LE89316 =
{
   VOICECFG_LE89316_STR,   /* szBoardId */
   "96368MB2G",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96368MB2G",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_3_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "96368MB2G",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_3_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_SI32178 =
{
   VOICECFG_SI32178_STR,   /* szBoardId */
   "96368MB2G",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MB2G_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96368MB2G",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_3_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "96368MVNgr",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_SI32178 =
{
   VOICECFG_SI32178_STR,   /* szBoardId */
   "96368MVNgr",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "96368MVNgr",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_LE89316 =
{
   VOICECFG_LE89316_STR,   /* szBoardId */
   "96368MVNgr",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_VE890HV =
{
   VOICECFG_VE890HV_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89136,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89336,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_LE88536_TH =
{
   VOICECFG_LE88536_TH_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgr_LE88264_TH =
{
   VOICECFG_LE88264_TH_STR,   /* szBoardId */
   "96368MVNgr",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88264,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_SI32176 =
{
   VOICECFG_SI32176_STR,   /* szBoardId */
   "96368MVNgrP2",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_SI32178 =
{
   VOICECFG_SI32178_STR,   /* szBoardId */
   "96368MVNgrP2",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_SI3217X_NOFXO =
{
   VOICECFG_SI3217X_NOFXO_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_VE890_INVBOOST =
{
   VOICECFG_VE890_INVBOOST_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_LE89116 =
{
   VOICECFG_LE89116_STR,   /* szBoardId */
   "96368MVNgrP2",   
   1,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89116,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_LE89316 =
{
   VOICECFG_LE89316_STR,   /* szBoardId */
   "96368MVNgrP2",   
   1,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89316,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_VE890HV =
{
   VOICECFG_VE890HV_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89136,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89336,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK, 

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_LE88536_TH =
{
   VOICECFG_LE88536_TH_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88536,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_INVBOOST_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96368MVNgrP2_LE88264_TH =
{
   VOICECFG_LE88264_TH_STR,   /* szBoardId */
   "96368MVNgrP2",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88264,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK_TH, 

   /* General-purpose flags */
   ( BP_FLAG_THALASSA_SUPPORT )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_LE88266x2_LE89010 =
{
   VOICECFG_LE88266x2_LE89010_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   4,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_89010,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_GPIO_29_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_4_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on Le89010 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_LE88266x2 =
{
   VOICECFG_LE88266x2_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   4,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID},
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88266,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device 0 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_MASTERSLAVE_FB,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_SI3217X =
{
   VOICECFG_SI3217X_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   2,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32176,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32178,

         /* SPI control */
         {  
            /* SPI dev id */
            2,
            /* SPI GPIO */
            BP_GPIO_28_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_SI32261 =
{
   VOICECFG_SI32261_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* There is no second channel on 89116 so mark it as inactive */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_SI32260x2_SI3050 =
{
   VOICECFG_SI32260x2_SI3050_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   4,             /* numFxsLines */
   1,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            1,   
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            2,   
            /* SPI GPIO */
            BP_GPIO_28_AL,  
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* voiceDevice3 parameters */
      {
         /* Device type */
         BP_VD_SILABS_3050,

         /* SPI control */
         {  
            /* SPI dev id */
            3,
            /* SPI GPIO */
            BP_GPIO_29_AL,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_4_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_DAA,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               4,
               4
            },
            /* There is no second channel on Si3050 so mark it as inactive */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_NONE,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96368RMVNg_SI32260x2 =
{
   VOICECFG_SI32260x2_STR,   /* szBoardId */
   "96368RMVNg NOR",   
   4,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   2,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_IDECT1,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_INACTIVE,
               BP_VCTYPE_DECT,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            1, 
            /* SPI GPIO */
            BP_NOT_DEFINED, 
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_16_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* voiceDevice2 parameters */
      {
         /* Device type */
         BP_VD_SILABS_32261,  /* Should be Si32260, but use Si32261 for now */

         /* SPI control */
         {  
            /* SPI dev id */
            2, 
            /* SPI GPIO */
            BP_GPIO_28_AL, 
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_17_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               2,
               2
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               3,
               3
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_GPIO_8_AH,  BP_GPIO_13_AH } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FB_TSS,

   /* General-purpose flags */
   ( 0 )

};

static PVOICE_BOARD_PARMS g_VoiceBoardParms[] = 
{
   &voiceBoard_96368MVWG,
   
   &voiceBoard_96368MB2G_LE88506,
   &voiceBoard_96368MB2G_LE89116,
   &voiceBoard_96368MB2G_LE89316,
   &voiceBoard_96368MB2G_SI3217X,
   &voiceBoard_96368MB2G_SI32176,
   &voiceBoard_96368MB2G_SI32178,
   
   &voiceBoard_96368MVNgr_SI3217X,
   &voiceBoard_96368MVNgr_SI32176,
   &voiceBoard_96368MVNgr_SI32178,
   &voiceBoard_96368MVNgr_SI3217X_NOFXO,
   &voiceBoard_96368MVNgr_VE890_INVBOOST,
   &voiceBoard_96368MVNgr_LE89116,          
   &voiceBoard_96368MVNgr_LE89316,
   &voiceBoard_96368MVNgr_VE890HV,        
   &voiceBoard_96368MVNgr_LE88506,
   &voiceBoard_96368MVNgr_LE88536_TH,
   &voiceBoard_96368MVNgr_LE88264_TH, 

   &voiceBoard_96368MVNgrP2_SI3217X,
   &voiceBoard_96368MVNgrP2_SI32176,
   &voiceBoard_96368MVNgrP2_SI32178,
   &voiceBoard_96368MVNgrP2_SI3217X_NOFXO,
   &voiceBoard_96368MVNgrP2_VE890_INVBOOST,
   &voiceBoard_96368MVNgrP2_LE89116,          
   &voiceBoard_96368MVNgrP2_LE89316,
   &voiceBoard_96368MVNgrP2_VE890HV,        
   &voiceBoard_96368MVNgrP2_LE88506,
   &voiceBoard_96368MVNgrP2_LE88536_TH,
   &voiceBoard_96368MVNgrP2_LE88264_TH,
   
   &voiceBoard_96368RMVNg_LE88266x2_LE89010,
   &voiceBoard_96368RMVNg_LE88266x2,
   &voiceBoard_96368RMVNg_SI3217X,
   &voiceBoard_96368RMVNg_SI32261,
   &voiceBoard_96368RMVNg_SI32260x2_SI3050,
   &voiceBoard_96368RMVNg_SI32260x2,

   0
};

#endif

#if defined(_BCM96816_) || defined(CONFIG_BCM96816)

VOICE_BOARD_PARMS voiceBoard_96816SV_SI3239 =
{
   VOICECFG_SI3239_STR,   /* szBoardId */
   "96816SV",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_SILABS_3239,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,

   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )

};

VOICE_BOARD_PARMS voiceBoard_96816PVWM_LE9530_WB =
{
   VOICECFG_LE9530_WB_STR,   /* szBoardId */
   "96816PVWM",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_9530,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_WIDEBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,

   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )

};

VOICE_BOARD_PARMS voiceBoard_96816PVWM_LE9530 =
{
   VOICECFG_LE9530_STR,   /* szBoardId */
   "96816PVWM",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_9530,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,

   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )

};

VOICE_BOARD_PARMS voiceBoard_96816PVWM_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96816PVWM",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

VOICE_BOARD_PARMS voiceBoard_96816PVWM_LE9530_LE88506 =
{
   VOICECFG_LE9530_LE88506_STR,   /* szBoardId */
   "96816PVWM",   
   4,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Test a single channel on 88506 */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* voiceDevice1 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_9530,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               BP_TIMESLOT_INVALID,
               BP_TIMESLOT_INVALID
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },
   
   /* SLIC Device Profile */
   BP_VD_FLYBACK,    

   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )

};

VOICE_BOARD_PARMS voiceBoard_96818RG_LE9530 =
{
   VOICECFG_LE9530_STR,   /* szBoardId */
   "96818RG",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_9530,

         /* SPI control */
         {  
            /* SPI dev id */
            0,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         0,

         /* Reset pin */
         BP_NOT_DEFINED,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_NOT_DEFINED,

   /* General-purpose flags */
   ( BP_FLAG_DSP_APMHAL_ENABLE )

};

VOICE_BOARD_PARMS voiceBoard_96818RG_LE88506 =
{
   VOICECFG_LE88506_STR,   /* szBoardId */
   "96818RG",   
   2,             /* numFxsLines */
   0,             /* numFxoLines */
   0,             /* numDectLines */
   0,             /* numFailoverRelayPins */

   {
      /* voiceDevice0 parameters */
      {
         /* Device type */
         BP_VD_ZARLINK_88506,

         /* SPI control */
         {  
            /* SPI dev id */
            1,
            /* SPI GPIO */
            BP_NOT_DEFINED,
         },

         /* Reset required (1 for yes, 0 for no) */
         1,

         /* Reset pin */
         BP_GPIO_14_AL,

         /* Channel description */
         {
            /* Channel 0 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               0,
               0
            },
            /* Channel 1 on device */
            {  BP_VOICE_CHANNEL_ACTIVE,
               BP_VCTYPE_SLIC,
               BP_VOICE_CHANNEL_PCMCOMP_MODE_NONE,
               BP_VOICE_CHANNEL_NARROWBAND,
               BP_VOICE_CHANNEL_SAMPLE_SIZE_16BITS,
               BP_VOICE_CHANNEL_ENDIAN_BIG,
               1,
               1
            },
         }
      },

      /* Always end the device list with BP_NULL_DEVICE_MACRO */
      BP_NULL_DEVICE_MACRO,
   },

   /* Relay control pins */
   { { BP_NOT_DEFINED,  BP_NOT_DEFINED } },

   /* DECT UART control pins */
   { BP_NOT_DEFINED,  BP_NOT_DEFINED },

   /* SLIC Device Profile */
   BP_VD_FLYBACK,

   /* General-purpose flags */
   ( 0 )

};

static PVOICE_BOARD_PARMS g_VoiceBoardParms[] =
{
   &voiceBoard_96816SV_SI3239,
   &voiceBoard_96816PVWM_LE88506,
   &voiceBoard_96816PVWM_LE9530,
   &voiceBoard_96816PVWM_LE9530_WB,
   &voiceBoard_96816PVWM_LE9530_LE88506,
   &voiceBoard_96818RG_LE9530,
   &voiceBoard_96818RG_LE88506,
   0
};

#endif


static PVOICE_BOARD_PARMS g_pCurrentVoiceBp = 0;

static void bpmemcpy( void* dstptr, const void* srcptr, int size );
static void bpmemcpy( void* dstptr, const void* srcptr, int size )
{
   char* dstp = dstptr;
   const char* srcp = srcptr;
   int i;
   for( i=0; i < size; i++ )
   {
      *dstp++ = *srcp++;
   }
} 

int BpGetVoiceParms( char* pszBoardId, VOICE_BOARD_PARMS* voiceParms, char* pszBaseBoardId )
{
   int nRet = BP_BOARD_ID_NOT_FOUND;
   PVOICE_BOARD_PARMS *ppBp;



   for( ppBp = g_VoiceBoardParms; *ppBp; ppBp++ )
   {
     if( (0 == bpstrcmp((*ppBp)->szBoardId, pszBoardId)) && (0 == bpstrcmp((*ppBp)->szBaseBoardId, pszBaseBoardId) ))
     {
         g_pCurrentVoiceBp = *ppBp;
         bpmemcpy( voiceParms, g_pCurrentVoiceBp, sizeof(VOICE_BOARD_PARMS) );
         nRet = BP_SUCCESS;
         break;
     }
   }

   return( nRet );
}


/**************************************************************************
* Name       : BpSetVoiceBoardId
*
* Description: This function find the BOARD_PARAMETERS structure for the
*              specified board id string and assigns it to a global, static
*              variable.
*
* Parameters : [IN] pszBoardId - Board id string that is saved into NVRAM.
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_BOARD_ID_NOT_FOUND - Error, board id input string does not
*                  have a board parameters configuration record.
***************************************************************************/
int BpSetVoiceBoardId( char *pszBoardId )
{
    int nRet = BP_BOARD_ID_NOT_FOUND;
    PVOICE_BOARD_PARMS *ppBp;


    for( ppBp = g_VoiceBoardParms; *ppBp; ppBp++ )
    {
        if( 0 == bpstrcmp((*ppBp)->szBoardId, pszBoardId) )
        {
            g_pCurrentVoiceBp = *ppBp;
            nRet = BP_SUCCESS;
            break;
        }
    }

    return( nRet );
} /* BpSetVoiceBoardId */


/**************************************************************************
* Name       : BpGetVoiceBoardId
*
* Description: This function returns the current board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  string is returned in.
*
* Returns    : BP_SUCCESS - Success, value is returned.
***************************************************************************/

int BpGetVoiceBoardId( char *pszBoardId )
{
    int i;


    if (g_pCurrentVoiceBp == 0)
        return -1;

    for (i = 0; i < BP_BOARD_ID_LEN; i++)
        pszBoardId[i] = g_pCurrentVoiceBp->szBoardId[i];

    return 0;
}


/**************************************************************************
* Name       : BpGetVoiceBoardIds
*
* Description: This function returns all of the supported voice board id strings.
*
* Parameters : [OUT] pszBoardIds - Address of a buffer that the board id
*                  strings are returned in.  Each id starts at BP_BOARD_ID_LEN
*                  boundary.
*              [IN] nBoardIdsSize - Number of BP_BOARD_ID_LEN elements that
*                  were allocated in pszBoardIds.
*              [IN] pszBaseBoardId - Name of base Board ID to associate Voice
*                  Board ID with.
*
* Returns    : Number of board id strings returned.
***************************************************************************/
int BpGetVoiceBoardIds( char *pszBoardIds, int nBoardIdsSize, char *pszBaseBoardId )
{
    PVOICE_BOARD_PARMS *ppBp;
    int i;
    char *src;
    char *dest;

    for( i = 0, ppBp = g_VoiceBoardParms; *ppBp && nBoardIdsSize; ppBp++ )
    {
        if( 0 == bpstrcmp((*ppBp)->szBaseBoardId, pszBaseBoardId) )
        {
//            printf ("match (*ppBp)->szBaseBoardId=%s, pszBaseBoardId=%s\n",(*ppBp)->szBaseBoardId, pszBaseBoardId);
            
            dest = pszBoardIds;
            src = (*ppBp)->szBoardId;
            while( *src )
                *dest++ = *src++;
            *dest = '\0';
            i++;
            pszBoardIds += BP_BOARD_ID_LEN;
            nBoardIdsSize--;
        }
    }

    return( i );
} /* BpGetVoiceBoardIds */
