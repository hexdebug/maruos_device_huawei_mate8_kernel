
#ifdef  __cplusplus
  #if  __cplusplus
          extern "C"{
  #endif
#endif
/*****************************************************************************
   1 头文件包含
*****************************************************************************/
#include "vos.h"
#include "MnCallTimer.h"
#include "MnCallApi.h"
#include "MnCallMgmt.h"
#include "MnComm.h"
#include "CstNasInterface.h"
#include "MnCallReqProc.h"

#define    THIS_FILE_ID        PS_FILE_ID_MNCALL_CSTPROC_C


/*lint -save -e958 */

/*****************************************************************************
  2 函数实现
*****************************************************************************/

VOS_VOID  MN_CALL_ProcCstErrorInd(
    const VOS_VOID                      *pMsg
)
{
    MN_CALL_END_PARAM_STRU              stEndParm;
    VOS_UINT32                          ulNumOfCalls;
    VOS_UINT8                           i;
    MN_CALL_ID_T                        aCallIds[MN_CALL_MAX_NUM];
    MN_CALL_TYPE_ENUM_U8                enCallType;
    MN_CALL_INFO_STRU                   stCallInfo;
    VOS_UINT32                          ulRet;


    PS_MEM_SET(aCallIds, 0x00, sizeof(aCallIds));

     stEndParm.enEndCause = MN_CALL_INTERWORKING_UNSPECIFIED;


    PS_MEM_SET(&stCallInfo, 0x00, sizeof(stCallInfo));

    /* 查找当前存在的video call，如果存在，就退出循环 */
    MN_CALL_GetNotIdleStateCalls(&ulNumOfCalls, aCallIds);
    for ( i = 0; i < ulNumOfCalls; i++ )
    {
        enCallType = MN_CALL_GetCallType(aCallIds[i]);
        if ( MN_CALL_TYPE_VIDEO == enCallType )
        {
            break;
        }

    }

    if ( ulNumOfCalls == i )
    {
        MN_WARN_LOG("MN_CALL_ProcCstErrorInd: no video type call exist.");
        return;
    }

    MN_CALL_GetCallInfoByCallId(aCallIds[i],&stCallInfo);

    ulRet = MN_CALL_InternalCallEndReqProc(stCallInfo.clientId,
                                           0,
                                           aCallIds[i],
                                           &stEndParm);
    if (TAF_CS_CAUSE_SUCCESS != ulRet)
    {
        MN_WARN_LOG("MN_CALL_ProcCstErrorInd: Fail to MN_CALL_InternalCallEndReqProc.");
    }

}
VOS_VOID  MN_CALL_ProcCstSetupCnf(
    const VOS_VOID                      *pMsg
)
{

    CST_CALL_SETUP_CNF_STRU             *pstSetupCnfMsg;

    pstSetupCnfMsg = (CST_CALL_SETUP_CNF_STRU *)pMsg;

    /* 停止响应的计时器 */
    MN_CALL_StopTimer(MN_CALL_TID_WAIT_CST_SETUP);

    /* 设置信道打开的标识 */
    MN_CALL_SetVideoCallChannelOpenFlg(VOS_TRUE,(MN_CALL_ID_T)pstSetupCnfMsg->usTransId);
}





VOS_VOID  MN_CALL_SendCstSetupReqMsg(
    VOS_UINT8                           ucCallId
)
{

   CALL_CST_SETUP_REQ_STRU             *pstTmpMsg;


    /* 申请VOS消息 */
    pstTmpMsg = (CALL_CST_SETUP_REQ_STRU *)
        PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_TAF, sizeof(CALL_CST_SETUP_REQ_STRU));
    if (VOS_NULL_PTR == pstTmpMsg)
    {
        MN_ERR_LOG("API_CcSyncInd: Failed to alloc VOS message.");
        return;
    }

    /* 填写VOS消息头 */
    pstTmpMsg->ulSenderCpuId                = VOS_LOCAL_CPUID;
    pstTmpMsg->ulSenderPid                  = WUEPS_PID_TAF;
    pstTmpMsg->ulReceiverCpuId              = VOS_LOCAL_CPUID;
    pstTmpMsg->ulReceiverPid                = UEPS_PID_CST;

    /* 这部分消息是CST模块需要用到的参数值 */
    pstTmpMsg->usMsgType                    = ID_CALL_CST_SETUP_REQ;
    pstTmpMsg->usTransId                    = ucCallId;
    pstTmpMsg->stCSTPara.enTranType         = CST_TRAN_TYPE_TRANSPARENT;
    pstTmpMsg->stCSTPara.enTranMode         = CST_TRAN_MODE_UMTS;
    pstTmpMsg->stCSTPara.ucRbId             = (VOS_UINT8)MN_CALL_GetRbId(ucCallId);
    pstTmpMsg->stCSTPara.enSyncType         = CST_SYNC_TYPE_SYNC;

    /* 目前可视电话下面这些赋值都不需要，只是为了CST模块适配过去的代码
       而填入一个在有效值范围内的值 */
    pstTmpMsg->stCSTPara.enUsrRate          = CST_RATE_VALUE_64K;
    pstTmpMsg->stCSTPara.enTchType          = CST_TCH_F_144;
    pstTmpMsg->stCSTPara.enParityType       = CST_FORC_0_PARITY;
    pstTmpMsg->stCSTPara.enDataBitNum       = CST_DATA_BIT_NUM_7;
    pstTmpMsg->stCSTPara.enStopBitNum       = CST_STOP_BIT_NUM_1;
    pstTmpMsg->stCSTPara.enCompressAdopt    = CST_COMPRESS_NOT_ADOPT;
    pstTmpMsg->stCSTPara.ucFaxCall          = VOS_NO;
    pstTmpMsg->stCSTPara.enCallDir          = CST_CALL_DIR_MO;
    pstTmpMsg->stCSTPara.enFaxClass         = CST_FAX_CLASS_1_0;
    (VOS_VOID)VOS_MemSet(pstTmpMsg->stCSTPara.ucRsv, 0, sizeof(pstTmpMsg->stCSTPara.ucRsv));

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_TAF, pstTmpMsg))
    {
        MN_ERR_LOG("MN_CALL_SendCstSetupReqMsg():WARNING:SEND MSG FIAL");  
    }
}



VOS_VOID  MN_CALL_SendCstRelReqMsg(
    VOS_UINT8                           ucCallId
)
{

    CALL_CST_REL_REQ_STRU               *pstTmpMsg;

    /* 申请VOS消息 */
    pstTmpMsg = (CALL_CST_REL_REQ_STRU *)
        PS_ALLOC_MSG_WITH_HEADER_LEN(WUEPS_PID_TAF, sizeof(CALL_CST_REL_REQ_STRU));
    if (VOS_NULL_PTR == pstTmpMsg)
    {
        MN_ERR_LOG("API_CcSyncInd: Failed to alloc VOS message.");
        return;
    }

    /* 填写VOS消息头 */
    pstTmpMsg->ulSenderCpuId      = VOS_LOCAL_CPUID;
    pstTmpMsg->ulSenderPid        = WUEPS_PID_TAF;
    pstTmpMsg->ulReceiverCpuId    = VOS_LOCAL_CPUID;
    pstTmpMsg->ulReceiverPid      = UEPS_PID_CST;

    pstTmpMsg->usMsgType    = ID_CALL_CST_REL_REQ;
    pstTmpMsg->usTransId    = (VOS_UINT16)MN_CALL_GetCcTi(ucCallId);
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_TAF, pstTmpMsg))
    {
        MN_ERR_LOG("MN_CALL_SendCstRelReqMsg():WARNING:SEND MSG FIAL");
    }
}

/*lint -restore */


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


