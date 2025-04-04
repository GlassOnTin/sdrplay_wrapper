#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* HANDLE;

#define SDRPLAY_MAX_DEVICES 4
#define SDRPLAY_MAX_SER_NO_LEN 64

typedef enum {
    sdrplay_api_Success                     = 0,
    sdrplay_api_Fail                        = 1,
    sdrplay_api_InvalidParam                = 2,
    sdrplay_api_OutOfRange                  = 3,
    sdrplay_api_GainUpdateError             = 4,
    sdrplay_api_RfUpdateError               = 5,
    sdrplay_api_FsUpdateError               = 6,
    sdrplay_api_HwError                     = 7,
    sdrplay_api_AliasingError               = 8,
    sdrplay_api_AlreadyInitialised          = 9,
    sdrplay_api_NotInitialised              = 10,
    sdrplay_api_NotEnabled                  = 11,
    sdrplay_api_HwVerError                  = 12,
    sdrplay_api_OutOfMemError               = 13,
    sdrplay_api_ServiceNotResponding        = 14,
    sdrplay_api_StartPending                = 15,
    sdrplay_api_StopPending                 = 16,
    sdrplay_api_InvalidMode                 = 17,
    sdrplay_api_FailedVerification1         = 18,
    sdrplay_api_FailedVerification2         = 19,
    sdrplay_api_FailedVerification3         = 20,
    sdrplay_api_FailedVerification4         = 21,
    sdrplay_api_FailedVerification5         = 22,
    sdrplay_api_FailedVerification6         = 23,
    sdrplay_api_InvalidServiceVersion       = 24
} sdrplay_api_ErrT;

typedef enum {
    sdrplay_api_Update_None                 = 0,
    sdrplay_api_Update_Dev_Fs               = 1,
    sdrplay_api_Update_Dev_Ppm              = 2,
    sdrplay_api_Update_Dev_SyncUpdate       = 4,
    sdrplay_api_Update_Dev_ResetFlags       = 8,
    sdrplay_api_Update_Tuner_Frf            = 16,
    sdrplay_api_Update_Tuner_BwType         = 32,
    sdrplay_api_Update_Tuner_IfType         = 64,
    sdrplay_api_Update_Tuner_DcOffset       = 128,
    sdrplay_api_Update_Tuner_LoMode         = 256,
    sdrplay_api_Update_Tuner_Gr             = 512,
    sdrplay_api_Update_Ctrl_DCoffsetIQimbalance = 1024,
    sdrplay_api_Update_Ctrl_Decimation      = 2048,
    sdrplay_api_Update_Ctrl_Agc             = 4096,
    sdrplay_api_Update_Ctrl_AdsbMode        = 8192,
    sdrplay_api_Update_Ctrl_OverloadMsgAck  = 16384,
    sdrplay_api_Update_Rsp1a_BiasTControl   = 32768,
    sdrplay_api_Update_Rsp1a_RfNotchControl = 65536,
    sdrplay_api_Update_Rsp1a_RfDabNotchControl = 131072,
    sdrplay_api_Update_Rsp2_BiasTControl    = 262144,
    sdrplay_api_Update_Rsp2_AmPortSelect    = 524288,
    sdrplay_api_Update_Rsp2_AntennaControl  = 1048576,
    sdrplay_api_Update_Rsp2_RfNotchControl  = 2097152,
    sdrplay_api_Update_Rsp2_ExtRefControl   = 4194304,
    sdrplay_api_Update_RspDuo_BiasTControl  = 8388608,
    sdrplay_api_Update_RspDuo_AmPortSelect  = 16777216,
    sdrplay_api_Update_RspDuo_Tuner1AmNotchControl = 33554432,
    sdrplay_api_Update_RspDuo_RfNotchControl = 67108864,
    sdrplay_api_Update_RspDuo_RfDabNotchControl = 134217728,
    sdrplay_api_Update_RspDx_HdrEnable      = 268435456,
    sdrplay_api_Update_RspDx_BiasTControl   = 536870912,
    sdrplay_api_Update_RspDx_AntennaControl = 1073741824
} sdrplay_api_ReasonForUpdateT;

typedef enum {
    sdrplay_api_Update_Ext1_None            = 0,
    sdrplay_api_Update_Ext1_RspDuo_BiasTControl = 1,
    sdrplay_api_Update_Ext1_RspDuo_AmPortSelect = 2,
    sdrplay_api_Update_Ext1_RspDuo_Tuner1AmNotchControl = 4,
    sdrplay_api_Update_Ext1_RspDuo_RfNotchControl = 8,
    sdrplay_api_Update_Ext1_RspDuo_RfDabNotchControl = 16,
    sdrplay_api_Update_Ext1_RspDx_HdrEnable = 32,
    sdrplay_api_Update_Ext1_RspDx_BiasTControl = 64,
    sdrplay_api_Update_Ext1_RspDx_AntennaControl = 128,
    sdrplay_api_Update_Ext1_RspDx_RfNotchControl = 256,
    sdrplay_api_Update_Ext1_RspDx_RfDabNotchControl = 512,
    sdrplay_api_Update_Ext1_RspDx_HdBwEnable = 1024,
    sdrplay_api_Update_Ext1_RspDx_XlnaEnable = 2048
} sdrplay_api_ReasonForUpdateExtension1T;

typedef enum {
    sdrplay_api_Tuner_Neither               = 0,
    sdrplay_api_Tuner_A                     = 1,
    sdrplay_api_Tuner_B                     = 2,
    sdrplay_api_Tuner_Both                  = 3
} sdrplay_api_TunerSelectT;

typedef enum {
    sdrplay_api_GainChange                  = 0,
    sdrplay_api_PowerOverloadChange         = 1,
    sdrplay_api_DeviceRemoved               = 2,
    sdrplay_api_RspDuoModeChange            = 3
} sdrplay_api_EventT;

typedef enum {
    sdrplay_api_Overload_Detected           = 0,
    sdrplay_api_Overload_Corrected          = 1
} sdrplay_api_PowerOverloadCbEventIdT;

typedef enum {
    sdrplay_api_RspDuoMode_Unknown          = 0,
    sdrplay_api_RspDuoMode_Single_Tuner     = 1,
    sdrplay_api_RspDuoMode_Dual_Tuner       = 2,
    sdrplay_api_RspDuoMode_Master           = 4,
    sdrplay_api_RspDuoMode_Slave            = 8
} sdrplay_api_RspDuoModeT;

typedef struct {
    char SerNo[SDRPLAY_MAX_SER_NO_LEN];
    unsigned char hwVer;
    sdrplay_api_TunerSelectT tuner;
    sdrplay_api_RspDuoModeT rspDuoMode;
    unsigned char valid;
    double rspDuoSampleFreq;
    HANDLE dev;
} sdrplay_api_DeviceT;

typedef struct {
    double fsHz;
} sdrplay_api_FsFreqT;

typedef struct {
    double rfHz;
} sdrplay_api_RfFreqT;

typedef struct {
    unsigned char LNAstate;
    int gRdB;
} sdrplay_api_GainT;

typedef struct {
    sdrplay_api_RfFreqT rfFreq;
    sdrplay_api_GainT gain;
} sdrplay_api_TunerParamsT;

typedef struct {
    sdrplay_api_TunerParamsT tunerParams;
} sdrplay_api_RxChannelParamsT;

typedef struct {
    sdrplay_api_FsFreqT fsFreq;
} sdrplay_api_DevParamsT;

typedef struct {
    sdrplay_api_DevParamsT* devParams;
    sdrplay_api_RxChannelParamsT* rxChannelA;
    sdrplay_api_RxChannelParamsT* rxChannelB;
} sdrplay_api_DeviceParamsT;

sdrplay_api_ErrT sdrplay_api_Open(void);
sdrplay_api_ErrT sdrplay_api_Close(void);
void sdrplay_api_ApiVersion(float *apiVer);
const char* sdrplay_api_GetErrorString(sdrplay_api_ErrT err);
sdrplay_api_ErrT sdrplay_api_LockDeviceApi(void);
sdrplay_api_ErrT sdrplay_api_UnlockDeviceApi(void);
sdrplay_api_ErrT sdrplay_api_GetDevices(sdrplay_api_DeviceT *devices, unsigned int *numDevs, unsigned int maxDevs);
sdrplay_api_ErrT sdrplay_api_SelectDevice(sdrplay_api_DeviceT *device);
sdrplay_api_ErrT sdrplay_api_ReleaseDevice(sdrplay_api_DeviceT *device);
// Stream callback parameters
typedef struct {
    unsigned int firstSampleNum;
    int grChanged;
    int rfChanged;
    int fsChanged;
    unsigned int numSamples;
} sdrplay_api_StreamCbParamsT;

// Event callback parameters
typedef struct {
    sdrplay_api_PowerOverloadCbEventIdT powerOverloadChangeType;
    unsigned int unknownEventData;
} sdrplay_api_EventParamsT;

// Callback function types
typedef void (*sdrplay_api_StreamCallback_t)(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params, unsigned int numSamples, unsigned int reset, void *cbContext);
typedef void (*sdrplay_api_EventCallback_t)(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT *params, void *cbContext);

// Callback functions structure
typedef struct {
    sdrplay_api_StreamCallback_t StreamACbFn;
    sdrplay_api_StreamCallback_t StreamBCbFn;
    sdrplay_api_EventCallback_t EventCbFn;
} sdrplay_api_CallbackFnsT;

sdrplay_api_ErrT sdrplay_api_GetDeviceParams(HANDLE dev, sdrplay_api_DeviceParamsT **deviceParams);
sdrplay_api_ErrT sdrplay_api_Update(HANDLE dev, sdrplay_api_TunerSelectT tuner, sdrplay_api_ReasonForUpdateT reasonForUpdate, sdrplay_api_ReasonForUpdateExtension1T reasonForUpdateExt1);
sdrplay_api_ErrT sdrplay_api_Init(HANDLE dev, sdrplay_api_CallbackFnsT *callbackFns, void *cbContext);
sdrplay_api_ErrT sdrplay_api_Uninit(HANDLE dev);

#ifdef __cplusplus
}
#endif