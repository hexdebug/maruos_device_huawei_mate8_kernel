

#ifndef __TAF_MMA_FSM_IMS_SWITCH_H__
#define __TAF_MMA_FSM_IMS_SWITCH_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "TafTypeDef.h"
#include "TafAppMma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 TAF_MMA_RcvTafImsSwitchSet_ImsSwitch_Init(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
);
VOS_UINT32 TAF_MMA_RcvMsccImsStartCnf_ImsSwitch_WaitMsccImsStartCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
);

VOS_UINT32 TAF_MMA_RcvTiWaitMsccImsStartCnfExpired_ImsSwitch_WaitMsccImsStartCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
);
VOS_UINT32 TAF_MMA_RcvMsccImsStopCnf_ImsSwitch_WaitMsccImsStopCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
);
VOS_UINT32 TAF_MMA_RcvTiWaitMsccImsStopCnfExpired_ImsSwitch_WaitMsccImsStopCnf(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
);

VOS_VOID TAF_MMA_ReportImsSwitchSetCnf_ImsSwitch(
    TAF_MMA_APP_OPER_RESULT_ENUM_UINT32 enResult,
    TAF_ERROR_CODE_ENUM_UINT32          enErrorCause
);

#endif /* FEATURE_IMS */


#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TafMmaFsmImsSwitch.h */


