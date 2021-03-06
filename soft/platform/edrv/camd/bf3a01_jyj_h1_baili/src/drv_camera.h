﻿/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
* All rights reserved.
*
* This software is supplied "AS IS" without any warranties.
* RDA assumes no responsibility or liability for the use of the software,
* conveys no license or title under any patent, copyright, or mask work
* right to the product. RDA reserves the right to make changes in the
* software without notification.  RDA also make no representation or
* warranty that such application will be suitable for the specified use
* without further testing or modification.
*/



#include "hal_i2c.h"
#include "hal_camera.h"
#include "camera_m.h"

//#define  DRV_CAM_FRAME_6    1
#define  DRV_CAM_FRAME_10    1
//#define  DRV_CAM_FRAME_12    1
typedef struct
{
    UINT8 Addr;            // Register Address
    UINT8 Data;             // Register Data
    UINT8 Wait;            // Time to wait after write
} CAM_REG_T;

typedef struct
{
    UINT16 Len;             // Size of array
    CAM_REG_T *RegVal;      // Array of registers
} CAM_REG_GROUP_T;

typedef enum
{
    CAM_MODE_IDLE,
    CAM_MODE_PREVIEW_STAGE_1,
    CAM_MODE_PREVIEW_STAGE_2,
    CAM_MODE_CAPTURE_STAGE_1,
    CAM_MODE_CAPTURE_STAGE_2,
    CAM_MODE_PREP_VID_CAPTURE,
    CAM_MODE_VID_CAPTURE
} CAM_MODE_T;

// On FPGA platform, Pixel Clock must not be too fast or xfer screws up
#define CAM_CLK_CFG 0x81

/*/ change teh clk
{0xfa, 0x10},
{0xfe, 0x02},
{0x04, 0x21},
{0xfe, 0x00}
*/
//240 320
PRIVATE const CAM_REG_T RG_InitPowerOnQvgainit[] =
{
    //bf3a01 initial
    //initial black
    {0xfe, 0x00, 0x01},
    {0x3d, 0x00, 0x01},
    {0x30, 0x3b, 0x01},
    {0x31, 0x3b, 0x01},
    {0x3d, 0xff, 0x01},

    {0xfe, 0x01, 0x01},
    {0xe0, 0xba, 0x01},//VCLK=48M//9a
    {0xe1, 0x03, 0x01},
    {0xe2, 0x05, 0x01},
    {0xe3, 0x42, 0x01},//bit[1]=1,bypass PLL
    {0xe4, 0x33, 0x01},//0x22
    {0xe5, 0x03, 0x01},//add lzq

    {0xe7, 0x05, 0x01},//05

    {0x51, 0x10, 0x01},//VSYNC:active low  HSYNC:active high
    {0x50, 0x00, 0x01},//SP mode
    {0x66, 0x01, 0x01},
    {0x52, 0xf1, 0x01},
    {0x53, 0x20, 0x01},//10
    {0x5d, 0x00, 0x01},

    {0xfe, 0x00, 0x01},
    {0x00, 0x4b, 0x01},//bit[5]:1,mirror;bit[4]:1,flip;
    {0x02, 0x10, 0x01},//dummy pixel
    {0x15, 0x0a, 0x01},
    {0x3c, 0x9f, 0x01},//9a
    {0x3e, 0x68, 0x01},
    {0x0f, 0x13, 0x01},

    //initial AWB
    {0xfe, 0x00, 0x01},
    {0xa0, 0x54, 0x01},
    {0xb0, 0x19, 0x01},//blue gain
    {0xb1, 0x25, 0x01},//red gain
    {0xa0, 0x55, 0x01},

    //initial AE
    {0xfe, 0x01, 0x01},
    {0x00, 0x08, 0x01},
    {0x0e, 0x03, 0x01},
    {0x0f, 0x30, 0x01},//{0x0e,0x0f}:exposure time
    {0x10, 0x18, 0x01},//global gain
    {0x00, 0x05, 0x01},

    {0xfe, 0x00, 0x01},
    {0x84, 0xa2, 0x01},
    {0x82, 0x0f, 0x01},//denoise; The larger the smaller noise;//0x08
    {0x86, 0x12, 0x01},//bit[6:4]:bright edge enhancement;bit[2:0]:dark edge enhancement;//0x23

    /*//gamma tongtouxinghao
    {0xfe, 0x00, 0x01},
    {0x60, 0x25, 0x01},
    {0x61, 0x2a, 0x01},
    {0x62, 0x28, 0x01},
    {0x63, 0x28, 0x01},
    {0x64, 0x20, 0x01},
    {0x65, 0x1d, 0x01},
    {0x66, 0x17, 0x01},
    {0x67, 0x15, 0x01},
    {0x68, 0x0f, 0x01},
    {0x69, 0x0e, 0x01},
    {0x6a, 0x0a, 0x01},
    {0x6b, 0x06, 0x01},
    {0x6c, 0x05, 0x01},
    {0x6d, 0x04, 0x01},
    {0x6e, 0x02, 0x01},*/
    {0x72, 0x0f, 0x01},//gamma offset//0x0c
    {0x73, 0x0f, 0x01},//gamma offset//0x0c
    {0x74, 0x44, 0x01},//bit[7:4] and bit[3:0]值越大，整体越绿

    /*//gamma default
    {0xfe, 0x00, 0x01},
    {0x60, 0x38, 0x01},
    {0x61, 0x30, 0x01},
    {0x62, 0x24, 0x01},
    {0x63, 0x1f, 0x01},
    {0x64, 0x1c, 0x01},
    {0x65, 0x16, 0x01},
    {0x66, 0x12, 0x01},
    {0x67, 0x0f, 0x01},
    {0x68, 0x0d, 0x01},
    {0x69, 0x0c, 0x01},
    {0x6a, 0x0b, 0x01},
    {0x6b, 0x09, 0x01},
    {0x6c, 0x09, 0x01},
    {0x6d, 0x08, 0x01},
    {0x6e, 0x07, 0x01},
    {0x72, 0x0c, 0x01},//gamma offset
    {0x73, 0x0c, 0x01},//gamma offset
    {0x74, 0x44, 0x01},//bit[7:4] and bit[3:0]值越大，整体越绿*/

    //gamma low noise
    {0xfe, 0x00, 0x01},
    {0x60, 0x24, 0x01},
    {0x61, 0x30, 0x01},
    {0x62, 0x24, 0x01},
    {0x63, 0x1d, 0x01},
    {0x64, 0x1a, 0x01},
    {0x65, 0x14, 0x01},
    {0x66, 0x11, 0x01},
    {0x67, 0x0f, 0x01},
    {0x68, 0x0e, 0x01},
    {0x69, 0x0d, 0x01},
    {0x6a, 0x0c, 0x01},
    {0x6b, 0x0b, 0x01},
    {0x6c, 0x0a, 0x01},
    {0x6d, 0x09, 0x01},
    {0x6e, 0x09, 0x01},

    //outdoor color
    {0xfe, 0x00, 0x01},
    {0xc7, 0x21, 0x01},
    {0xc8, 0x19, 0x01},
    {0xc9, 0x84, 0x01},
    {0xca, 0x64, 0x01},
    {0xcb, 0x89, 0x01},
    {0xcc, 0x3f, 0x01},
    {0xcd, 0x16, 0x01},

    //indoor color default
    {0xfe, 0x00, 0x01},
    {0xc0, 0x05, 0x01},
    {0xc1, 0x07, 0x01},
    {0xc2, 0x30, 0x01},
    {0xc3, 0x28, 0x01},
    {0xc4, 0x3c, 0x01},
    {0xc5, 0x10, 0x01},
    {0xc6, 0x96, 0x01},

    /*//indoor color vivid green OK
    {0xfe, 0x00, 0x01},
    {0xc0, 0x26, 0x01},
    {0xc1, 0x1b, 0x01},
    {0xc2, 0x5e, 0x01},
    {0xc3, 0x58, 0x01},
    {0xc4, 0x7d, 0x01},
    {0xc5, 0x2b, 0x01},
    {0xc6, 0x96, 0x01}, */

    /*//indoor color vivid blue OK
    {0xfe, 0x00, 0x01},
    {0xc0, 0x39, 0x01},
    {0xc1, 0x31, 0x01},
    {0xc2, 0x76, 0x01},
    {0xc3, 0x64, 0x01},
    {0xc4, 0x57, 0x01},
    {0xc5, 0x07, 0x01},
    {0xc6, 0x96, 0x01},
    */

    //AWB
    {0xfe, 0x00, 0x01},
    {0xb2, 0x01, 0x01},
    {0xb3, 0x11, 0x01},//green gain
    {0xa0, 0x55, 0x01},
    {0xa2, 0x18, 0x01},//low limit of blue gain in indoor scene
    {0xa3, 0x36, 0x01},//high limit of blue gain in indoor scene
    {0xa4, 0x11, 0x01},//low limit of red gain in indoor scene
    {0xa5, 0x36, 0x01},//high limit of red gain in indoor scene
    {0xa7, 0x80, 0x01},//blue target
    {0xa8, 0x7f, 0x01},//red target
    {0xa9, 0x15, 0x01},
    {0xaa, 0x10, 0x01},
    {0xab, 0x10, 0x01},
    {0xac, 0x2c, 0x01},
    {0xad, 0xf0, 0x01},
    {0xae, 0x20, 0x01},//0x80->0x20 Ycbcr LIMIT
    {0xb4, 0x18, 0x01},//low limit of blue gain in outdoor scene
    {0xb5, 0x1a, 0x01},//high limit of blue gain in outdoor scene
    {0xb6, 0x1c, 0x01},//low limit of red gain in outdoor scene
    {0xb7, 0x30, 0x01},//high limit of red gain in outdoor scene
    {0xd0, 0x4c, 0x01},

    //AE
    {0xfe, 0x01, 0x01},
    {0x00, 0x05, 0x01},
    {0x04, 0x42, 0x01},//AE target
    {0x09, 0x0d, 0x01},//bit[5:0]:max integration time step//0x0d
    {0x0a, 0x45, 0x01},
    {0x0b, 0x82, 0x01},//minimum integration time
    {0x0c, 0x2d, 0x01},//50hz banding
    {0x0d, 0x25, 0x01},//60hz banding
    {0x15, 0x42, 0x01},
    {0x17, 0xb5, 0x01},
    {0x18, 0x28, 0x01},//The value should be smaller than the 50hz banding. add 20140702
    {0x1b, 0x28, 0x01},//minimum global gain
    {0x1c, 0x50, 0x01},
    {0x1d, 0x39, 0x01},
    {0x1e, 0x50, 0x01},//0x5d
    {0x1f, 0x55, 0x01},//max global gain//0x77

    {0xfe, 0x00,0x01},
    {0xce, 0x48,0x01},//Contrast

    //saturation
    {0xfe, 0x01, 0x01},
    {0x64, 0xc0, 0x01},//blue saturation
    {0x65, 0xb0, 0x01},//red saturation

    {0xfe, 0x00, 0x01},
#if defined(_CAMERA_DISPLAY_MIRRIO_)
    {0x00, 0x0b, 0x01}, //0x2b,3b,4b,
#else
    {0x00, 0x3b, 0x01}, //0x2b,3b,4b,
#endif

    {0xfe, 0x01, 0x01},
    {0x59, 0x03, 0x01},
};

// 240x320
PRIVATE const CAM_REG_T RG_Init_QVGA320240[] =
{
    {0xfe, 0x00, 0x01},
    {0x05, 0x0c, 0x01},
    {0x06, 0x00, 0x01},
    {0x07, 0xf8, 0x01},
    {0x08, 0x00, 0x01},
    {0x09, 0x48, 0x01},
    {0x0a, 0x10, 0x01},
    {0xfe, 0x01, 0x01},
    {0x54, 0x00, 0x01},
    {0x55, 0x00, 0x01},
    {0x56, 0x78, 0x01},
    {0x57, 0x00, 0x01},
    {0x58, 0xa0, 0x01},
    {0x52, 0xf0, 0x01},
    {0x53, 0x20, 0x01},
    {0x59, 0x02, 0x01},
    {0x5a, 0x11, 0x01},
    {0x5b, 0x11, 0x01},
    {0x5c, 0x11, 0x01},
};

// 160x120
PRIVATE const CAM_REG_T RG_Init_QQVGA128160[] =
{
    {0xfe, 0x00, 0x01},
    {0x05, 0x0c, 0x01},
    {0x06, 0x00, 0x01},
    {0x07, 0xf8, 0x01},
    {0x08, 0x00, 0x01},
    {0x09, 0x48, 0x01},
    {0x0a, 0x10, 0x01},
    {0xfe, 0x01, 0x01},
    {0x54, 0x00, 0x01},
    {0x55, 0x00, 0x01},
    {0x56, 0x50, 0x01},
    {0x57, 0x2d, 0x01},
    {0x58, 0x69, 0x01},
    {0x52, 0xf3, 0x01},
    {0x53, 0x20, 0x01},
    {0x59, 0x05, 0x01},
    {0x5a, 0x21, 0x01},
    {0x5b, 0x21, 0x01},
    {0x5c, 0x33, 0x01},
};


PRIVATE const CAM_REG_T RG_Init_SPI_1_Bit[] =
    {};

PRIVATE const CAM_REG_T RG_Reinit_MODE_DEFAULT[] =
    {};

// use this for 640x480 (VGA) capture
PRIVATE const CAM_REG_T RG_InitPowerOnVga[] =
    {};


// use this for 160*120 (QQVGA) capture
PRIVATE const CAM_REG_T RG_InitPowerOnQqvga[] =
    {};

PRIVATE const CAM_REG_T RG_VidFormatVga[] =
{
};

PRIVATE const CAM_REG_T RG_VidFormatQvga[] =
{
};

PRIVATE const CAM_REG_T RG_VidFormatQqvga[] =
    {};

typedef struct
{
    // Camera information structure
    char                desc[40];

    UINT8               snrtype;            //sensor type(RGB or YUV)
    UINT8               pclk;               //use PCLK of sensor
    UINT8               clkcfg;             //[0]: VSync delay enable; [1]: Fifo VSync delay enable; [2]: Use PCLK Negtive Edge.
    HAL_I2C_BPS_T       i2crate;            //I2C rate : KHz
    UINT8               i2caddress;         //I2C address
    UINT8               i2cispaddress;      //ISP I2C address
    UINT8               pwrcfg;             //sensor power initial configure(SIF REG801 BIT[0]~BIT[2])
    BOOL                rstactiveh;         //sensor reset active hi/low polarity (TRUE = active hi)
    BOOL                snrrst;             //Reset sensor enable

    CAM_MODE_T          cammode;


    BOOL                capturevideo;
    BOOL                captureframe;
    UINT8               numactivebuffers;
    UINT8               currentbuf;

    UINT8               brightness;         //brightness
    UINT8               contrast;           //contrast

    CAM_FORMAT_T        format;
    UINT32              npixels;
    CAM_IMAGE_T         vidbuffer;

    CAM_REG_GROUP_T     standby;            //sensor standby register
    CAM_REG_GROUP_T     Initpoweron;        //SENSOR ISP initial configure

    CAM_REG_GROUP_T     sifpwronseq;        //Sif config sequence(Reg.800 bit0~2) when sensor power on
    CAM_REG_GROUP_T     sifstdbyseq;        //Sif config sequence(Reg.800 bit0~2) when sensor standby
    CAM_REG_GROUP_T     dsif;               //SIF initial configure
    //    TSnrSizeCfgGroup    snrSizeCfg;         //sensor size configure information

    // This 'callback' mechanism is not necessary because the encapsulation is taken
    // care of by the environment
    //    PSensorSetReg       snrSetRegCall;      //set reg callback
    //    PSensorGetReg       snrGetRegCall;
    //    PSnrIdChk           snrIdChkCall;
    //    PSnrMirrorFlip      snrMirrorFlipCall;
    //    PSnrContrast        snrContrastCall;
    //    PSnrBrightness      snrBrightnessCall;
    //    PSnrMode            snrModeCall;
    //    PSnrGetEt           snrGetEt;
    //    PSnrSetEt           snrSetEt;
    //    TFlashParm          flashparm;
    CAM_IRQ_HANDLER_T   camirqhandler;
} CAM_INFO_T;

PRIVATE volatile CAM_INFO_T gSensorInfo =
{
    "BF3A01",      // char desc[40];

    0,                      // UINT8 snrtype; (rgb)
    1,                      // UINT8 pclk; use or don't use PCLK of sensor???
    0,                      // UINT8 clkcfg; ???
    HAL_I2C_BPS_100K,       // UINT16 i2crate;
    0x6e,                   // UINT8 i2caddress; this is the _REAL_ slave address.
    //  The value written in phase 1 of a write cycle will be 0x42
    //  while the value just before the read will be 0x43
    0x6e,                   // UINT8 i2cispaddress;
    0x02,                   // UINT8 pwrcfg;???
    TRUE,                   // BOOL rstactiveh;  // active high
    FALSE,                  // BOOL snrrst;

    CAM_MODE_IDLE,          // cammode

    FALSE,                  // Capture video mode
    FALSE,                  // Capturing single frame
    0,                      // Number of active buffers reserved by the upper layer
    0,                      // current frame buffer

    3,                      // UINT8 brightness;
    4,                      // UINT8 contrast;
    CAM_FORMAT_RGB565,      // Format
    CAM_NPIX_QVGA,           // npixels(VGA, QVGA, QQVGA...)
    NULL,                   // Video Buffer.  First frame buffer for video or image buffer

    {0, NULL},              // CAM_REG_GROUP_T standby;
    // Should start with QQVGA, but for testing, start with VGA
    //    {sizeof(RG_InitPowerOnQqvga) / sizeof(CAM_REG_T), (CAM_REG_T*)&RG_InitPowerOnQqvga}, // CAM_REG_GROUP_T Initpoweron;
    {sizeof(RG_InitPowerOnVga) / sizeof(CAM_REG_T), (CAM_REG_T*)&RG_InitPowerOnVga}, // CAM_REG_GROUP_T Initpoweron;

    {0, NULL},              // CAM_REG_GROUP_T sifpwronseq;
    {0, NULL},              // CAM_REG_GROUP_T sifstdbyseq;
    {0, NULL},              // CAM_REG_GROUP_T dsif; initial value
    NULL                    // IRQ Callback
};

// ############
// Private
// ############

// These are the commands specific to the SCCB
#define PHASE3_WR_PH1   0x11000     // Start & Write    Slave ID
#define PHASE3_WR_PH2   0x1000      // Write            Slave Register Address
#define PHASE3_WR_PH3   0x1100      // Stop & Write     Slave Register Data

#define PHASE2_WR_PH1   0x11000     // Start & Write    Slave ID
#define PHASE2_WR_PH2   0x1100      // Stop & Write     Slave Register Address (to read)

#define PHASE2_RD_PH1   0x11000     // Start & Write    Slave ID
#define PHASE2_RD_PH2   0x111       // Stop & Read & NACK   Slave Register Read Data

PRIVATE VOID camerap_WriteOneReg( UINT8 Addr, UINT8 Data);
PRIVATE VOID camerap_Delay(UINT16 Wait_mS);
PRIVATE VOID camerap_WriteReg(CAM_REG_T RegWrite);
PRIVATE UINT8 camerap_ReadReg(UINT8 Addr);
PRIVATE VOID camerap_SendRgSeq(const CAM_REG_T *RegList, UINT16 Len);
PRIVATE VOID camerap_InteruptHandler(HAL_CAMERA_IRQ_CAUSE_T cause);
PRIVATE UINT8 camerap_GetId(VOID);
PRIVATE VOID camerap_Reserve(CAM_IRQ_HANDLER_T FrameReady);
PRIVATE VOID camerap_Release(VOID);
PRIVATE VOID camerap_PowerOn(CAM_SIZE_T CamOutSize, UINT16 IspOutWidth, UINT16 IspOutHeight, CAM_FORMAT_T Format);
PRIVATE VOID camerap_PowerOff(VOID);
PRIVATE VOID camerap_CaptureImage(VOID);
PRIVATE VOID camerap_SetZoomFactorL(UINT32 ZoomFactor);
PRIVATE UINT32 camerap_ZoomFactor(VOID);
PRIVATE VOID camerap_SetDigitalZoomFactorL(UINT32 DigZoomFactor);
PRIVATE UINT32 camerap_DigitalZoomFactor(VOID);
PRIVATE VOID camerap_SetContrastL(UINT32 Contrast);
PRIVATE UINT32 camerap_Contrast(VOID);
PRIVATE VOID camerap_SetBrightnessL(UINT32 Brightness);
PRIVATE UINT32 camerap_Brightness(VOID);
PRIVATE VOID camerap_SetFlashL(CAM_FLASH_T Flash);
PRIVATE CAM_FLASH_T camerap_Flash(VOID);
PRIVATE VOID camerap_SetExposureL(CAM_EXPOSURE_T Exposure);
PRIVATE CAM_EXPOSURE_T camerap_Exposure(VOID);
PRIVATE void camerap_SetBanding(UINT32 iBanding);
PRIVATE VOID camerap_SetWhiteBalanceL(CAM_WHITEBALANCE_T WhiteBalance);
PRIVATE CAM_WHITEBALANCE_T camerap_WhiteBalance(VOID);
PRIVATE VOID camerap_SetSpecialEffect(CAM_SPECIALEFFECT_T SpecialEffect);
PRIVATE CAM_SPECIALEFFECT_T camerap_SpecialEffect(VOID);
PRIVATE VOID camerap_SetNightMode(UINT32 NightMode);
PRIVATE UINT32 camerap_NightMode(VOID);

// This is not _really_ Direct, but it does implement a viewfinder function
PRIVATE VOID camerap_StartViewFinderDirectL();
PRIVATE VOID camerap_StopViewFinder(VOID);
PRIVATE BOOL camerap_ViewFinderActive(VOID);

PRIVATE VOID camerap_SetViewFinderMirror(BOOL Mirror);
PRIVATE BOOL camerap_ViewFinderMirror(VOID);
PRIVATE VOID camerap_CancelCaptureImage(VOID);
PRIVATE VOID camerap_EnumerateCaptureSizes(CAM_SIZE_T *Size, UINT32 SizeIndex, CAM_FORMAT_T Format);
PRIVATE VOID camerap_PrepareVideoCaptureL(CAM_FORMAT_T Format, UINT32 SizeIndex, UINT32 RateIndex, UINT32 BuffersToUse, UINT32 FramesPerBuffer);
PRIVATE VOID camerap_StartVideoCapture(VOID);
PRIVATE VOID camerap_StopVideoCapture(VOID);
PRIVATE BOOL camerap_VideoCaptureActive(VOID);
PRIVATE VOID camerap_EnumerateVideoFrameSizes(CAM_SIZE_T Size, UINT32 SizeIndex, CAM_FORMAT_T Format);
PRIVATE VOID camerap_EnumerateVideoFrameRates();
PRIVATE VOID camerap_GetFrameSize(CAM_SIZE_T Size);
PRIVATE VOID camerap_SetFrameRate(UINT32 FrameRate);
PRIVATE UINT32 camerap_FrameRate(VOID);
PRIVATE UINT32 camerap_BuffersInUse(VOID);
PRIVATE UINT32 camerap_FramesPerBuffer(VOID);
