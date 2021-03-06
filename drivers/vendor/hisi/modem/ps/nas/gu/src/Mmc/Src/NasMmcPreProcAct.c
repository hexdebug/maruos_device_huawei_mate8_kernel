/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : NasMmcPreProcAct.c
  版 本 号   : 初稿
  作    者   : zhoujun /40661
  生成日期   : 2010年11月15日
  最近修改   :
  功能描述   :预处理
  函数列表   :
  修改历史   :
  1.日    期   : 2010年11月12日
    作    者   : zhoujun /40661
    修改内容   : 创建文件

******************************************************************************/

/*****************************************************************************
  1 头文件包含
*****************************************************************************/


#include "PsTypeDef.h"
#include "PsCommonDef.h"
#include "NasComm.h"
#include "NasCommDef.h"
#include "NasMmcPreProcAct.h"
#include "NasMmcProcNvim.h"
#include "NasMmcMain.h"
#include "NasMmcFsmSyscfg.h"
#include "NasMmcSndMscc.h"
#include "NasMmcFsmMainTbl.h"
#include "NasMmcSndGmm.h"
#include "NasMmcSndMm.h"
#include "NasMmcSndGuAs.h"
#include "NasMmcSndMscc.h"
#include "NasMmcSndOm.h"
#include "NasMmcSndInternalMsg.h"
#include "NasMmcFsmInterSysCcoTbl.h"
#include "NasMmcFsmInterSysOosTbl.h"
#include "NasMmcFsmInterSysCellReselTbl.h"
#include "NasMmcFsmPlmnSelection.h"
#include "NasMmcTimerMgmt.h"
#include "UsimPsInterface.h"
#include "NasMmlCtx.h"
#include "NasMmlLib.h"
#include "NasMmlMsgProc.h"
#include "NasMmcProcNvim.h"
#include "NasMmcProcUsim.h"
#include "NasMmcFsmSyscfg.h"
#include "NasMmcProcRegRslt.h"
/* 删除ExtAppMmcInterface.h*/
#include "NasMmcComFunc.h"
#include "NasMmcPlmnSelectionStrategy.h"
#include "NVIM_Interface.h"
#include "NasMmcComFunc.h"
#include "NasUtranCtrlInterface.h"
#include "NasUsimmApi.h"

#include  "NasMmcFsmPlmnSelectionTbl.h"
#include "NasMmcCtx.h"

#if (FEATURE_ON == FEATURE_PTM)
#include "NasErrorLog.h"
#endif

#include "NasMmcFsmBgPlmnSearchTbl.h"
#include "NasMmcFsmBgPlmnSearch.h"

#if   (FEATURE_ON == FEATURE_LTE)
#include "NasMmcSndLmm.h"
#include "MmcLmmInterface.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define    THIS_FILE_ID        PS_FILE_ID_NAS_MMCBGPREPROC_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*lint -save -e958 */

/*****************************************************************************
  3 函数实现
*****************************************************************************/

#if defined (__PS_WIN32_RECUR__)

VOS_UINT32 NAS_MMC_RestoreFixedContextData_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MMC_FIXED_CONTEXT_MSG_STRU                          stRcvMsgCB;
    NVIM_EQUIVALENT_PLMN_LIST_STRU                          stEquivPlmn;
    VOS_UINT8                                               i;
    NAS_MMC_NVIM_NETWORK_CAPABILITY_STRU                    stNetworkCap;/*network capability*/
    NAS_NVIM_RPLMN_WITH_RAT_STRU                            stRplmn;
    VOS_UINT32                                              ulSrcLen;
    VOS_UINT32                                              ulDataLen;
    NAS_MML_PC_REPLAY_COMPRESS_CONTEXT_STRU                *pstRcMsg;
    VOS_UINT8                                              *pucSrc;
    VOS_UINT32                                              ulCount;
    VOS_UINT32                                              ulNvLength;

    NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU               stQuickStartFlg;
    NAS_NVIM_AUTOATTACH_STRU                            stAutoattachFlag;
    NAS_NVIM_HPLMN_FIRST_TIMER_STRU                     stHplmnTimerLen;
    NAS_NVIM_SELPLMN_MODE_STRU                          stSelplmnMode;

    stQuickStartFlg.ulQuickStartSta = 0;
    PS_MEM_SET(&stAutoattachFlag, 0x00, sizeof(NAS_NVIM_AUTOATTACH_STRU));

    PS_MEM_SET(&stHplmnTimerLen, 0x00, sizeof(NAS_NVIM_HPLMN_FIRST_TIMER_STRU));
    PS_MEM_SET(&stSelplmnMode, 0x00, sizeof(NAS_NVIM_SELPLMN_MODE_STRU));
    ulNvLength = 0;

    ulSrcLen  = 0;
    ulDataLen = sizeof(NAS_MMC_FIXED_CONTEXT_MSG_STRU);

    pstRcMsg = (NAS_MML_PC_REPLAY_COMPRESS_CONTEXT_STRU *)pstMsg;
    pucSrc   = pstRcMsg->aucData;

    /* 解析总长度 */
    PS_MEM_CPY(&ulCount, pucSrc, sizeof(VOS_UINT32));
    pucSrc += sizeof(VOS_UINT32);

    if( ulDataLen != ulCount )
    {
        return VOS_FALSE;
    }

    NAS_MML_UnCompressData(&stRcvMsgCB, &ulDataLen,
                           pucSrc, &ulSrcLen);

    /* NV项的恢复,恢复IMEI的内容 */
    NV_Write(en_NV_Item_IMEI, stRcvMsgCB.aucImeisv, NAS_MML_MAX_IMEISV_LEN);

    /* NV项的恢复,恢复搜网模式 */
    stSelplmnMode.usSelPlmnMode = (VOS_UINT16)stRcvMsgCB.enSelectionMode;
    NV_Write(en_NV_Item_SelPlmn_Mode, &stSelplmnMode, sizeof(NAS_NVIM_SELPLMN_MODE_STRU));

    /* NV项的恢复,恢复等效PLMN */
    PS_MEM_SET(&stEquivPlmn, 0, sizeof (NVIM_EQUIVALENT_PLMN_LIST_STRU));
    stEquivPlmn.ucCount = stRcvMsgCB.stEquPlmnInfo.ucEquPlmnNum;

    for (i=0; i<stEquivPlmn.ucCount; i++)
    {
        stEquivPlmn.struPlmnList[i].ucMcc[0]
            = (VOS_UINT8)(stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMcc & 0x0f);
        stEquivPlmn.struPlmnList[i].ucMcc[1]
            = (VOS_UINT8)((stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMcc>>8) & 0x0f);
        stEquivPlmn.struPlmnList[i].ucMcc[2]
            = (VOS_UINT8)((stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMcc>>16) & 0x0f);
        stEquivPlmn.struPlmnList[i].ucMnc[0]
            = (VOS_UINT8)(stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMnc & 0x0f);
        stEquivPlmn.struPlmnList[i].ucMnc[1]
            = (VOS_UINT8)((stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMnc>>8) & 0x0f);
        stEquivPlmn.struPlmnList[i].ucMnc[2]
            = (VOS_UINT8)((stRcvMsgCB.stEquPlmnInfo.astEquPlmnAddr[i].ulMnc>>16) & 0x0f);
    }

    NV_Write(en_NV_Item_EquivalentPlmn, &stEquivPlmn, sizeof(NVIM_EQUIVALENT_PLMN_LIST_STRU));

    /* NV项的恢复,恢复等效网络能力 */
    stNetworkCap.aucNetworkCapability[0] = stRcvMsgCB.stMsNetworkCapability.ucNetworkCapabilityLen;

    if (stRcvMsgCB.stMsNetworkCapability.ucNetworkCapabilityLen > NAS_MML_MAX_NETWORK_CAPABILITY_LEN)
    {
        stNetworkCap.aucNetworkCapability[0] = NAS_MML_MAX_NETWORK_CAPABILITY_LEN;
    }

    /* 存贮网络能力 */
    PS_MEM_CPY(&(stNetworkCap.aucNetworkCapability[1]),
               stRcvMsgCB.stMsNetworkCapability.aucNetworkCapability,
               stNetworkCap.aucNetworkCapability[0]);

    NV_Write(en_NV_Item_NetworkCapability, &stNetworkCap, NV_ITEM_NET_CAPABILITY_MAX_SIZE);

    /* NV项的恢复,恢复RPLMN信息 */
    stRplmn.ucLastRplmnRat          = stRcvMsgCB.stRplmnCfg.enLastRplmnRat;
    stRplmn.ucLastRplmnRatEnableFlg = stRcvMsgCB.stRplmnCfg.ucLastRplmnRatFlg;
    stRplmn.stGRplmn.ulMcc          = stRcvMsgCB.stRplmnCfg.stGRplmnInNV.ulMcc;
    stRplmn.stGRplmn.ulMnc          = stRcvMsgCB.stRplmnCfg.stGRplmnInNV.ulMnc;
    stRplmn.stWRplmn.ulMcc          = stRcvMsgCB.stRplmnCfg.stWRplmnInNV.ulMcc;
    stRplmn.stWRplmn.ulMnc          = stRcvMsgCB.stRplmnCfg.stWRplmnInNV.ulMnc;

    NV_Write(en_NV_Item_RPlmnWithRat, &stRplmn, sizeof(stRplmn));

    /* 恢复 Ps Auto attach */
    (VOS_VOID)NV_GetLength(en_NV_Item_Autoattach, &ulNvLength);
    stAutoattachFlag.usAutoattachFlag = (VOS_UINT16)stRcvMsgCB.ucPsAutoAttachFlg;
    NV_Write(en_NV_Item_Autoattach, &(stAutoattachFlag.usAutoattachFlag), ulNvLength);

    /* NV项的恢复,恢复HPLMN定时器时长,NV项中是以分钟来保存的,实际上启动定时器时需要以ms来启动 */
    stHplmnTimerLen.ucHplmnTimerLen = (VOS_UINT8)((stRcvMsgCB.stBGSrchCfg.ulFirstStartHplmnTimerLen) / (NAS_MML_SIXTY_SECOND * NAS_MML_ONE_THOUSAND_MILLISECOND));
    NV_Write(en_NV_Item_HPlmnFirstTimer, &(stHplmnTimerLen.ucHplmnTimerLen), NV_ITEM_HPLMN_FIRST_SEARCH_SIZE);

    /* NV项的恢复,恢复快速开机 */
    stQuickStartFlg.ulQuickStartSta = stRcvMsgCB.ulQuickStartFlag;
    NV_Write(en_NV_Item_FollowOn_OpenSpeed_Flag, &(stQuickStartFlg.ulQuickStartSta), sizeof(NAS_NVIM_FOLLOWON_OPENSPEED_FLAG_STRU));


    NAS_INFO_LOG(WUEPS_PID_MMC, "NAS_MMC_RestoreFixedContextData_PreProc - data is restored.");

    return VOS_TRUE;
}
VOS_UINT32 NAS_MMC_RestoreContextData_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MMC_CONTEXT_STRU                                    *pstMmcCtx;
    NAS_MML_CTX_STRU                                        *pstMmlCtx;
    NAS_MML_PC_REPLAY_COMPRESS_CONTEXT_STRU                 *pstRcMsg;
    VOS_UINT32                                               ulExpectCount;
    VOS_UINT32                                               ulCount;
    VOS_UINT32                                               ulItemCount;
    VOS_UINT8                                               *pucDest;
    VOS_UINT8                                               *pucSrc;
    VOS_UINT32                                               ulDestLen;

    pstMmcCtx     = NAS_MMC_GetMmcCtxAddr();
    pstMmlCtx     = NAS_MML_GetMmlCtx();
    ulExpectCount = 0;
    ulCount       = 0;
    ulItemCount   = 0;
    ulExpectCount = sizeof(NAS_MML_CTX_STRU)
                  + sizeof(MM_SUB_LAYER_SHARE_STRU)
                  + sizeof(NAS_MMC_CONTEXT_STRU)
                  + sizeof(TAF_PLMN_ID_STRU);

    /*注册状态机*/
    NAS_MMC_RegFsm();

    pstRcMsg = (NAS_MML_PC_REPLAY_COMPRESS_CONTEXT_STRU *)pstMsg;
    pucSrc   = pstRcMsg->aucData;

    /* 解析总长度 */
    PS_MEM_CPY(&ulCount, pucSrc, sizeof(VOS_UINT32));
    pucSrc += sizeof(VOS_UINT32);

    if( ulExpectCount != ulCount )
    {
        return VOS_FALSE;
    }

    /* g_stNasMmlCtx目标地址 */
    pucDest     = (VOS_UINT8 *)pstMmlCtx;
    ulDestLen   = sizeof(NAS_MML_CTX_STRU);

    /* 解压缩 */
    if (VOS_FALSE == NAS_MML_UnCompressData(pucDest, &ulDestLen, pucSrc, &ulItemCount) )
    {
        return VOS_FALSE;
    }

    /* 不还原内部消息，因为有可能内部消息被保存后立即被处理了，然后RECUR后又要处理内部消息。*/
    pstMmlCtx->stInternalMsgQueue.ucHeader = 0;
    pstMmlCtx->stInternalMsgQueue.ucTail = 0;

    pucSrc += (ulItemCount);

    /* g_MmSubLyrShare目标地址 */
    pucDest     = (VOS_UINT8 *)&g_MmSubLyrShare;
    ulDestLen   = sizeof(MM_SUB_LAYER_SHARE_STRU);

    /* 解压缩 */
    if (VOS_FALSE == NAS_MML_UnCompressData(pucDest, &ulDestLen, pucSrc, &ulItemCount) )
    {
        return VOS_FALSE;
    }
    pucSrc += (ulItemCount);

    /* g_stNasMmcCtx目标地址 */
    pucDest     = (VOS_UINT8 *)pstMmcCtx;
    ulDestLen   = sizeof(NAS_MMC_CONTEXT_STRU);

    /* 解压缩 */
    if (VOS_FALSE == NAS_MML_UnCompressData(pucDest, &ulDestLen, pucSrc, &ulItemCount) )
    {
        return VOS_FALSE;
    }
    pucSrc += (ulItemCount);

    NAS_MMC_ReloadFsmDesc(NAS_MMC_GetCurFsmAddr(), pstMmcCtx->stCurFsm.enFsmId);

    /* 根据PC回放消息，给PC工程中状态机的FsmStack地址重新赋值 */
    NAS_MMC_RestoreFsmStack(&pstMmcCtx->stFsmStack);

#if (FEATURE_ON == FEATURE_LTE)
#ifdef WIN32
    PS_MEM_CPY(&g_stSetRplmn,pucSrc,sizeof(TAF_PLMN_ID_STRU));
#endif
#endif
     pucSrc += sizeof(TAF_PLMN_ID_STRU);


    NAS_INFO_LOG(WUEPS_PID_MMC, "MMC: NAS_MMC_RestoreContextData - data is restored.");

    return VOS_TRUE;
}



VOS_VOID NAS_MMC_RestoreFsmStack(
    NAS_MMC_FSM_STACK_STRU             *pstPcFsmStack
)
{
    NAS_MMC_FSM_CTX_STRU               *pstFsmCtx   = VOS_NULL_PTR;
    NAS_MMC_FSM_STACK_STRU             *pstFsmStack = VOS_NULL_PTR;
    VOS_UINT32                          ulPcRecurFsmId;
    VOS_UINT32                          i;

    /* 根据并行状态机栈弹出的状态机获取当前状态机描述地址  */
    pstFsmStack                     = NAS_MMC_GetFsmStackAddr();

    PS_MEM_CPY(pstFsmStack, pstPcFsmStack, sizeof(NAS_MMC_FSM_STACK_STRU));

    /* 初始化栈中的当前状态机的FsmDesc */
    for ( i = 0 ; i < (pstPcFsmStack->usStackDepth); i++ )
    {
        pstFsmCtx                           = &(pstFsmStack->astFsmStack[i]);

        ulPcRecurFsmId                      = pstPcFsmStack->astFsmStack[i].enFsmId;

        NAS_MMC_ReloadFsmDesc(pstFsmCtx, ulPcRecurFsmId);
    }
}


VOS_VOID NAS_MMC_ReloadFsmDesc(
    NAS_MMC_FSM_CTX_STRU               *pstFsmCtx,
    VOS_UINT32                          ulPcRecurFsmId
)
{
    switch ( ulPcRecurFsmId )
    {
        case NAS_MMC_FSM_L1_MAIN :
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetMainFsmDescAddr();
            break;

        case NAS_MMC_FSM_SWITCH_ON :
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetSwitchOnFsmDescAddr();
            break;

        case NAS_MMC_FSM_POWER_OFF :
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetPowerOffFsmDescAddr();
            break;

        case NAS_MMC_FSM_PLMN_SELECTION :
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetPlmnSelectionFsmDescAddr();
            break;

        case NAS_MMC_FSM_ANYCELL_SEARCH :
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetAnyCellSearchFsmDescAddr();
            break;

        case NAS_MMC_FSM_SYSCFG:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetSysCfgFsmDescAddr();
            break;

        case NAS_MMC_FSM_INTER_SYS_CELLRESEL:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetInterSysCellReselFsmDescAddr();
            break;

        case NAS_MMC_FSM_INTER_SYS_OOS:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetInterSysOosFsmDescAddr();
            break;

        case NAS_MMC_FSM_INTER_SYS_HO:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetInterSysHoFsmDescAddr();
            break;

        case NAS_MMC_FSM_INTER_SYS_CCO:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetInterSysCcoFsmDescAddr();
            break;

        case NAS_MMC_FSM_PLMN_LIST:
            pstFsmCtx->pstFsmDesc       = NAS_MMC_GetPlmnListFsmDescAddr();
            break;

        case NAS_MMC_FSM_BG_PLMN_SEARCH:
            pstFsmCtx->pstFsmDesc               = NAS_MMC_GetBgPlmnSearchFsmDescAddr();
            break;

        default:
            break;
    }

}


#endif
VOS_UINT32  NAS_MMC_RcvMsccModeChange_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{

    MSCC_MMC_MODE_CHANGE_REQ_STRU                          *pstModeChangeReq = VOS_NULL_PTR;
    NAS_MML_MS_MODE_ENUM_UINT8                              enMsMode;
    VOS_UINT8                                              *pucEf           = VOS_NULL_PTR;
    NAS_MML_MS_MODE_ENUM_UINT8                              enOldMsMode;
    NAS_MMC_CS_DOMAIN_CAPA_CHANGE_TYPE_ENUM_UINT8           enCsChgType;
    VOS_UINT32                                              ulEfLen;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                                              ulDisableLteRoamFlg;
#endif

    /* Added by h00313353 for iteration 9, 2015-2-9, begin */
    VOS_UINT32                                              ulFileId;

    if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
    {
        ulFileId = USIMM_USIM_EFLOCI_ID;
    }
    else
    {
        ulFileId = USIMM_GSM_EFLOCI_ID;
    }
    /* Added by h00313353 for iteration 9, 2015-2-9, end */

    /* 检查参数的合法性 */
    pstModeChangeReq    = (MSCC_MMC_MODE_CHANGE_REQ_STRU*)pstMsg;

    enMsMode = NAS_MMC_ConvertMsccMsModeToMmlMsMode(pstModeChangeReq->enMsMode);

    /* 异常检查:参数类型错误，直接返回 */
    if (NAS_MML_MS_MODE_BUTT == enMsMode)
    {
        return VOS_TRUE;
    }

    if (VOS_TRUE == NAS_MML_GetSvlteSupportFlag())
    {
        ulEfLen         = 0;

        /* 获取原来的手机模式 */
        enOldMsMode = NAS_MML_GetMsMode();

        /* 获取CS域能力变化类型 */
        enCsChgType = NAS_MMC_GetCsDomainCapaChangeType(enOldMsMode, enMsMode);

        /* CS域由不支持到支持时，对应SVLTE中双待切单待的情况，
            MODEM0需要同步一下之前MODEM1在USIM上更新过的CS域的相关信息 */
        if (NAS_MMC_CS_DOMAIN_CAPA_UNAVAIL_TO_AVAIL == enCsChgType)
        {
            /* 同步SIM卡中的EFloci和EFkeys文件 */
            /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
            if (USIMM_API_SUCCESS == NAS_USIMMAPI_GetCachedFile(ulFileId, &ulEfLen, &pucEf, USIMM_GUTL_APP))
            {
                /* 读取文件成功后，MMC内部做相应处理 */
                NAS_MMC_ProcAgentUsimReadFileCnf_PreProc((VOS_UINT16)ulFileId, (VOS_UINT16)ulEfLen, pucEf);
            }

            if (USIMM_API_SUCCESS == NAS_USIMMAPI_GetCachedFile(USIMM_USIM_EFKEYS_ID, &ulEfLen, &pucEf, USIMM_GUTL_APP))
            {
                /* 读取文件成功后，MMC内部做相应处理 */
                NAS_MMC_ProcAgentUsimReadFileCnf_PreProc(USIMM_USIM_EFKEYS_ID, (VOS_UINT16)ulEfLen, pucEf);
            }
            /* Modified by h00313353 for iteration 9, 2015-2-9, end */
        }
        else
        {

        }

    }

    /* 更新MML保存的MODE */
    NAS_MML_SetMsMode(enMsMode);

    /* 通知GMM模块模式改变 */
    NAS_MMC_SndGmmModeChangeReq(enMsMode);

    /* 通知MM模块模式改变 */
    NAS_MMC_SndMmModeChangeReq(enMsMode);

#if (FEATURE_ON == FEATURE_LTE)

    ulDisableLteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

    /* 判断是否需要disable LTE */
    if (VOS_TRUE == NAS_MMC_IsNeedDisableLte_ModeChange())
    {
        if (NAS_MML_SIM_TYPE_SIM == NAS_MML_GetSimType())
        {
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_SIM_CARD_SWITCH_ON);
        }
        else
        {
             NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_USIM_INVALID);
        }

        if ( NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
        {
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);
        }
        else
        {
            /* 如果漫游导致的disable已经通知了GU接入层,此时不需要再重复通知 */
            if (VOS_FALSE == ulDisableLteRoamFlg)
            {
                NAS_MMC_SndDisableLteNotify(NAS_MML_GetDisableLteReason());

                /* 记录disable lte时驻留的网络信息 */
                NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);
        }

        return VOS_TRUE;
    }

    /* 判断是否需要enable LTE */
    if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_ModeChange())
    {
        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisableLteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }
#endif

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvGmmInfo_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MMCGMM_INFO_IND_STRU                *pRcvMsg = VOS_NULL_PTR;

    pRcvMsg = (MMCGMM_INFO_IND_STRU *)pstMsg;

    NAS_MMC_SndMsccMmInfo(&pRcvMsg->stMmInfo);

    return VOS_TRUE;
}


/*****************************************************************************
 函 数 名  : NAS_MMC_PreProcPdpStatusInd
 功能描述  : 预处理MMCGMM_PDP_STATUS_IND消息
 输入参数  : ulEventType:消息类型
             pstMsg:当前处理的消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:函数预处理完成
             VOS_FALSE:函数预处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2010年11月16日
    作    者   : zhoujun /40661
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvGmmPdpStatusInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MMCGMM_PDP_STATUS_IND_STRU          *pstRcvMsg;

    pstRcvMsg = (MMCGMM_PDP_STATUS_IND_STRU*)pstMsg;
    if ( (MMC_GMM_ACTIVE_PDP_CONTEXT_EXIST != pstRcvMsg->ulPdpStatusFlg)
      && (MMC_GMM_ACTIVE_PDP_CONTEXT_NOT_EXIST != pstRcvMsg->ulPdpStatusFlg))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvPdpStatusInd_PreProc:Msg is invalid.");
        return VOS_TRUE;
    }

    if ( MMC_GMM_ACTIVE_PDP_CONTEXT_EXIST == pstRcvMsg->ulPdpStatusFlg)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_PDP_ACT);
    }
    else
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_PDP_DEACT);
    }

    return VOS_TRUE;

}





VOS_UINT32  NAS_MMC_RcvMmInfo_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MMCMM_INFO_IND_STRU                 *pstRcvMsg;

    pstRcvMsg = (MMCMM_INFO_IND_STRU *)pstMsg;

    NAS_MMC_SndMsccMmInfo(&(pstRcvMsg->stMmInfo));

    return VOS_TRUE;
}





VOS_UINT32  NAS_MMC_RcvGasScellRxInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    GRRMM_SCELL_MEAS_IND_ST            *pstScellRx      = VOS_NULL_PTR;
    NAS_MML_CAMP_CELL_INFO_STRU        *pstCampCellInfo = VOS_NULL_PTR;
    VOS_UINT32                          ulReportRssiFlg;
    VOS_UINT32                          ulReportCellIdFlg;
    MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN   unMeasReportType;

    pstCampCellInfo         =  NAS_MML_GetCampCellInfo();
    pstScellRx              = (GRRMM_SCELL_MEAS_IND_ST *)pstMsg;
    ulReportRssiFlg         = VOS_FALSE;
    ulReportCellIdFlg       = VOS_FALSE;

    /*误码率超过99，信号强度大于0或小于-114都认为错误*/
    if ( (pstScellRx->ucChannelQual > NAS_MML_GSM_CHANNEL_QUAL_INVALID)
      || (pstScellRx->sRxlev > NAS_MML_RSSI_UPPER_LIMIT)
      || (pstScellRx->sRxlev < NAS_MML_RSSI_LOWER_LIMIT))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC ,"NAS_MMC_PreProcGasScellRxInd():WARNING:Value is unvalid");
        return VOS_TRUE;
    }

    /* 不判断是否与上次的数值一样,与GAS孙冰确认,完全相信GAS上报数值 */
    if (((GRRMM_SCELL_MEAS_TYPE_RXLEV == (pstScellRx->unMeasReportType.ucMeasReportType & GRRMM_SCELL_MEAS_TYPE_RXLEV))
      || (GRRMM_SCELL_MEAS_TYPE_NONE == pstScellRx->unMeasReportType.ucMeasReportType)) )
    {
        ulReportRssiFlg = VOS_TRUE;
    }

    /* 仅ber改变时，需要通知mma改变全局变量 */
    if ((GRRMM_SCELL_MEAS_TYPE_RXLEV     != (pstScellRx->unMeasReportType.ucMeasReportType & GRRMM_SCELL_MEAS_TYPE_RXLEV))
     || (GRRMM_SCELL_MEAS_TYPE_RXQUALITY == (pstScellRx->unMeasReportType.ucMeasReportType & GRRMM_SCELL_MEAS_TYPE_RXQUALITY)))
    {
        ulReportRssiFlg = VOS_TRUE;
    }


    /* 驻留态下上报类型指示是CELL ID或查询上报时,CELL ID变化则上报CELL ID */
    if ((NAS_MMC_AS_CELL_CAMP_ON == NAS_MMC_GetAsCellCampOn())
     && ((GRRMM_SCELL_MEAS_TYPE_CELLID == (pstScellRx->unMeasReportType.ucMeasReportType & GRRMM_SCELL_MEAS_TYPE_CELLID))
      || (GRRMM_SCELL_MEAS_TYPE_NONE == pstScellRx->unMeasReportType.ucMeasReportType)))
    {
        ulReportCellIdFlg = VOS_TRUE;
    }

    /*通过消息更新全局变量*/
    pstCampCellInfo->ucCellNum                      = 1;
    pstCampCellInfo->ucRssiNum                      = 1;

    /*更新信号质量*/
    pstCampCellInfo->astRssiInfo[0].sRssiValue      = pstScellRx->sRxlev;

    /*更新误码率*/
    pstCampCellInfo->astRssiInfo[0].ucChannalQual   = pstScellRx->ucChannelQual;
    pstCampCellInfo->sUeRfPower                     = pstScellRx->sUeRfPower;
    pstCampCellInfo->usCellDlFreq                   = pstScellRx->usCellDlFreq;
    pstCampCellInfo->usCellUlFreq                   = pstScellRx->usCellUlFreq;

    pstCampCellInfo->astCellInfo[0].sCellRssi       = pstScellRx->sRxlev;



    /* CELL ID发生变化 */
    if (VOS_TRUE == ulReportCellIdFlg)
    {
        pstCampCellInfo->astCellInfo[0].ulCellId = pstScellRx->usCellId;
        NAS_MMC_SndMsccSysInfo();
    }

    PS_MEM_SET(((VOS_UINT8*)&unMeasReportType), 0, sizeof(MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN));
    unMeasReportType.ucMeasReportType = pstScellRx->unMeasReportType.ucMeasReportType;

    /*调用函数向TAF上报结果*/
    if ( VOS_TRUE == ulReportRssiFlg )
    {
        NAS_MMC_SndMsccRssiInd(&unMeasReportType);
    }

    return VOS_TRUE;
}
VOS_VOID  NAS_MMC_RcvWasCellSubMode_PreProc(
    RRC_NAS_AT_MSG_SUBMODE_ENUM_UINT8   enSubMode
)
{
    /* 上报W下速率模式,R99 */
    if (RRC_NAS_AT_R99_ONLY == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_R99_ONLY);
    }
    /* 上报W下速率模式,HSDPA */
    else if (RRC_NAS_AT_HSDPA_ONLY == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_HSDPA);
    }
    /* 上报W下速率模式,HSUPA */
    else if (RRC_NAS_AT_HSUPA_ONLY == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_HSUPA);
    }
    /* 上报W下速率模式,HSUPA+HSDPA */
    else if (RRC_NAS_AT_HSDPA_AND_HSUPA == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_HSDPA_AND_HSUPA);
    }
    /* 上报W下速率模式,HSPA+ */
    else if (RRC_NAS_AT_HSPA_PLUS == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_HSPA_PLUS);
    }
    else if (RRC_NAS_AT_DC_HSPA_PLUS == enSubMode)
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_DC_HSPA_PLUS);
    }
    else if ( RRC_NAS_AT_DC_MIMO == enSubMode )
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_DC_MIMO);
    }
    else
    {
        ;
    }
}
VOS_UINT32  NAS_MMC_RcvWasAtMsgInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    RRMM_AT_MSG_IND_ST                  *pstAtMsg;

    pstAtMsg = (RRMM_AT_MSG_IND_ST *)pstMsg;

    /* 上报小区ID */
    if ( RRC_NAS_AT_CELL_ID == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_ID) )
    {
        NAS_MMC_RcvWasCellId_PreProc((RRC_NAS_CELL_ID_ST*)pstAtMsg->aucData);
    }

    /* 上报小区Bler */
    if ( RRC_NAS_AT_CELL_BLER == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_BLER) )
    {
        NAS_MMC_RcvWasCellBler_PreProc((RRC_NAS_CELL_TRCH_BLER_STRU_LIST*)pstAtMsg->aucData);
    }

    /* 上报小区信号质量 */
    if  (RRC_NAS_AT_CELL_SIGN == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_SIGN) )
    {
        NAS_MMC_RcvWasCellRssi_PreProc((RRC_NAS_CELL_RSCP_STRU_LIST*)pstAtMsg->aucData);
    }

    if (RRC_NAS_AT_CELL_SUBMODE == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_SUBMODE) )
    {
        NAS_MMC_RcvWasCellSubMode_PreProc((RRC_NAS_AT_MSG_SUBMODE_ENUM_UINT8)pstAtMsg->aucData[0]);
    }



    return VOS_TRUE;
}



VOS_UINT32 NAS_MMC_RcvWasAcInfoChange_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_W_AC_INFO_CHANGE_IND_STRU     *pstWasAcInfoChangeInd = VOS_NULL_PTR;
    NAS_MML_NETWORK_INFO_STRU          *pstNetWorkInfo        = VOS_NULL_PTR;
    VOS_UINT8                           ucOldCsResTrictionRegister;
    VOS_UINT8                           ucOldPsResTrictionRegister;
    NAS_MML_ACCESS_RESTRICTION_STRU                         *pstCurCsRestrictInfo = VOS_NULL_PTR;
    NAS_MML_ACCESS_RESTRICTION_STRU                          stOldCsRestrictInfo;
    NAS_MML_ACCESS_RESTRICTION_STRU                         *pstCurPsRestrictInfo = VOS_NULL_PTR;
    NAS_MML_ACCESS_RESTRICTION_STRU                          stOldPsRestrictInfo;
    NAS_MML_CAMP_PLMN_INFO_STRU        *pstCampInfo     = VOS_NULL_PTR;
    NAS_MML_LAI_STRU                   *pstLastSuccLai  = VOS_NULL_PTR;

    PS_MEM_SET(&stOldCsRestrictInfo, 0, sizeof(NAS_MML_ACCESS_RESTRICTION_STRU));
    PS_MEM_SET(&stOldPsRestrictInfo, 0, sizeof(NAS_MML_ACCESS_RESTRICTION_STRU));

    pstCampInfo      = NAS_MML_GetCurrCampPlmnInfo();
    pstLastSuccLai   = NAS_MML_GetCsLastSuccLai();

    pstWasAcInfoChangeInd       = (RRMM_W_AC_INFO_CHANGE_IND_STRU*)pstMsg;
    pstNetWorkInfo              =  NAS_MML_GetNetworkInfo();
    ucOldCsResTrictionRegister  = NAS_MML_GetCsRestrictRegisterFlg();
    ucOldPsResTrictionRegister  = NAS_MML_GetPsRestrictRegisterFlg();

    pstCurCsRestrictInfo    = NAS_MML_GetCsAcRestrictionInfo();
    pstCurPsRestrictInfo    = NAS_MML_GetPsAcRestrictionInfo();

    PS_MEM_CPY(&stOldCsRestrictInfo, pstCurCsRestrictInfo, sizeof(NAS_MML_ACCESS_RESTRICTION_STRU));
    PS_MEM_CPY(&stOldPsRestrictInfo, pstCurPsRestrictInfo, sizeof(NAS_MML_ACCESS_RESTRICTION_STRU));

    /* 保存CS PS注册受限信息 */
    NAS_MMC_UpdateCsPsRestriction_AcChangeInd(pstWasAcInfoChangeInd, pstNetWorkInfo);

    /* 向MM发送AC INFO CHANGE */
    NAS_MMC_SndMmWasAcInfoChangeInd(pstWasAcInfoChangeInd->stCsRestriction.enRestrictRegister,
                                    pstWasAcInfoChangeInd->stPsRestriction.enRestrictRegister,
                                    ucOldCsResTrictionRegister,
                                    ucOldPsResTrictionRegister);

    /* 向GMM发送AC INFO CHANGE */
    NAS_MMC_SndGmmWasAcInfoChangeInd(pstWasAcInfoChangeInd->stCsRestriction.enRestrictRegister,
                                     pstWasAcInfoChangeInd->stPsRestriction.enRestrictRegister,
                                     ucOldCsResTrictionRegister,
                                     ucOldPsResTrictionRegister);

    if (NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_WAS_AC_INFO_CHANGE);
    }
    else
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_TAS_AC_INFO_CHANGE);
    }

    /* 驻留小区的接入受限信息变化时,通知MSCC模块当前接入受限信息 */
    if (VOS_TRUE == NAS_MML_IsAcInfoChanged(&stOldCsRestrictInfo, pstCurCsRestrictInfo))
    {
        NAS_MMC_SndMsccAcInfoChangeInd(NAS_MSCC_PIF_SRVDOMAIN_CS, pstCurCsRestrictInfo);

        /* 从unbar到bar , lai发生变化到上报CS限制服务,CS注册状态为未注册未搜索 */
        if ((VOS_FALSE == ucOldCsResTrictionRegister)
         && (VOS_TRUE  == pstWasAcInfoChangeInd->stCsRestriction.enRestrictRegister))
        {
            if (VOS_FALSE == NAS_MML_CompareLai(&(pstCampInfo->stLai), pstLastSuccLai))
            {
                NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MMC_LIMITED_SERVICE);
                NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            }
        }
    }

    if (VOS_TRUE == NAS_MML_IsAcInfoChanged(&stOldPsRestrictInfo, pstCurPsRestrictInfo))
    {
        NAS_MMC_SndMsccAcInfoChangeInd(NAS_MSCC_PIF_SRVDOMAIN_PS, pstCurPsRestrictInfo);

        /* 从unbar到bar , rai发生变化到上报PS限制服务,PS注册状态为未注册未搜索 */
        if ((VOS_FALSE == ucOldPsResTrictionRegister)
         && (VOS_TRUE == pstWasAcInfoChangeInd->stPsRestriction.enRestrictRegister))
        {
            if (VOS_TRUE == NAS_MML_IsRaiChanged())
            {
                NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MMC_LIMITED_SERVICE);
                NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            }
        }
    }

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvWasAtMsgCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    RRMM_AT_MSG_CNF_ST                  *pstAtMsg = VOS_NULL_PTR;

    pstAtMsg = (RRMM_AT_MSG_CNF_ST *)pstMsg;

    /* 判断消息的有效性*/
    if ( RRC_NAS_AT_VALID != pstAtMsg->ucMsgFlg)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC ,"NAS_MMC_RcvWasAtMsgCnf_PreProc():WARNING:the msg is not valid!");

        return VOS_TRUE;
    }

    if (RRC_NAS_AT_START_FOREVER == pstAtMsg->ucActionType)
    {
        /* W会把当前小区信号质量上报，无需处理，等待at msg ind消息*/
        return VOS_TRUE;
    }

    if ( RRC_NAS_AT_CELL_ID == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_ID) )
    {
        NAS_MMC_RcvWasCellId_PreProc((RRC_NAS_CELL_ID_ST*)pstAtMsg->aucData);
    }

    if ( RRC_NAS_AT_CELL_BLER == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_BLER) )
    {
        NAS_MMC_RcvWasCellBler_PreProc((RRC_NAS_CELL_TRCH_BLER_STRU_LIST*)pstAtMsg->aucData);
    }

    if  (RRC_NAS_AT_CELL_SIGN == (pstAtMsg->ucMsgType & RRC_NAS_AT_CELL_SIGN) )
    {
        NAS_MMC_RcvWasCellRssi_PreProc((RRC_NAS_CELL_RSCP_STRU_LIST*)pstAtMsg->aucData);
    }

    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_PreProcPlmnQryReq
 功能描述  : 处理WRR和GAS发送过来的PLMN INFO查询信息
 输入参数  : ulEventType:消息类型
             pstMsg:当前处理的消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:函数预处理完成
             VOS_FALSE:函数预处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2010年11月29日
   作    者   : zhoujun /40661
   修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvRrMmPlmnQryReqPreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    NAS_MMC_SndRrcPlmnQueryCnf(((RRMM_PLMN_QUERY_REQ_STRU *)pstMsg)->enQueryWPlmn);

    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_RcvRrMmNotCampOn_PreProc
 功能描述  : 收到底层NOT CAMP ON消息的处理
 输入参数  : ulEventType - 消息类型
             pstMsg      - 消息的首地址
 输出参数  : 无
 返 回 值  : VOS_TRUE:函数预处理完成
             VOS_FALSE:函数预处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2011年07月20日
   作    者   : zhoujun \40661
   修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvRrMmNotCampOn_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MMC_SetAsCellCampOn(NAS_MMC_AS_CELL_NOT_CAMP_ON);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvRrMmEquplmnQuery_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    EQUIVALENT_PLMN_STRU                                    stEplmn;
    VOS_UINT8                                               i;
    NAS_MML_EQUPLMN_INFO_STRU                              *pstEquPlmnList = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU                                   *pstPlmnId      = VOS_NULL_PTR;
    RRMM_EPLMN_QUERY_REQ_STRU                              *pstEqueryMsg;
    NAS_MML_PLMN_WITH_RAT_STRU                              stPlmnWithRat;

    NAS_MML_PLMN_ID_STRU                                    stCurPlmnId;
    VOS_UINT32                                              ulOpCurPlmnId;
    NAS_MML_PLMN_WITH_RAT_STRU                             *pstUserSpecPlmnId = VOS_NULL_PTR;
    VOS_UINT32                                              ulEplmnValidFlg;
    VOS_UINT32                                              ulRegFlag;

    VOS_UINT32                                              ulIsPlmnIdInDestPlmnWithRatList;
    VOS_UINT32                                              ulIsPlmnIdInForbidPlmnList;

    ulOpCurPlmnId     = VOS_FALSE;
    stCurPlmnId.ulMcc = NAS_MML_INVALID_MCC;
    stCurPlmnId.ulMnc = NAS_MML_INVALID_MNC;
    pstUserSpecPlmnId = NAS_MMC_GetUserSpecPlmnId();
    ulEplmnValidFlg   = NAS_MML_GetEplmnValidFlg();
    ulRegFlag         = NAS_MMC_GetUserSpecPlmnRegisterStatus();

    pstEqueryMsg = (RRMM_EPLMN_QUERY_REQ_STRU*)pstMsg;

    PS_MEM_SET(&stEplmn, 0, sizeof(stEplmn));
    pstEquPlmnList  = NAS_MML_GetEquPlmnList();

    pstPlmnId       = NAS_MML_GetCurrCampPlmnId();

    if (WUEPS_PID_WRR == pstEqueryMsg->MsgHeader.ulSenderPid)
    {
        stPlmnWithRat.enRat         = NAS_MML_NET_RAT_TYPE_WCDMA;
    }
    else
    {
        stPlmnWithRat.enRat         = NAS_MML_NET_RAT_TYPE_GSM;
    }

    if (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        stPlmnWithRat.stPlmnId = pstUserSpecPlmnId->stPlmnId;
    }
    else
    {
        stPlmnWithRat.stPlmnId = *pstPlmnId;
    }

    ulIsPlmnIdInDestPlmnWithRatList = NAS_MML_IsPlmnIdWithRatInDestDisabledPlmnWithForbiddenPeriodList(&stPlmnWithRat);

    ulIsPlmnIdInForbidPlmnList = NAS_MML_IsPlmnIdInForbidPlmnList(&stPlmnWithRat.stPlmnId);

    if ((VOS_FALSE == ulIsPlmnIdInDestPlmnWithRatList)
     && (VOS_FALSE == ulIsPlmnIdInForbidPlmnList))
    {
        ulOpCurPlmnId     = VOS_TRUE;
        stCurPlmnId.ulMcc = stPlmnWithRat.stPlmnId.ulMcc;
        stCurPlmnId.ulMnc = stPlmnWithRat.stPlmnId.ulMnc;
    }

    if (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        /* 手动搜网模式判断eplmn列表是否有效，无效则返回 */
        if ((VOS_FALSE == ulRegFlag)
         || (VOS_FALSE == ulEplmnValidFlg))
        {
            stEplmn.ulEquPlmnNum = 0;

            NAS_MMC_SndAsEquPlmnQueryCnf(&stEplmn, ulOpCurPlmnId, &stCurPlmnId, pstMsg->ulSenderPid);
            return VOS_TRUE;
        }
    }

    /* 把等效plmn网络带给接入层 */
    for ( i = 0; i < pstEquPlmnList->ucEquPlmnNum; i++ )
    {
        if (VOS_TRUE == NAS_MML_IsBcchPlmnIdInDestSimPlmnList(&(pstEquPlmnList->astEquPlmnAddr[i]),
                         (VOS_UINT8)stEplmn.ulEquPlmnNum, (NAS_MML_PLMN_ID_STRU *)stEplmn.aEquPlmnIdList))
        {
            continue;
        }

        if (WUEPS_PID_WRR == pstEqueryMsg->MsgHeader.ulSenderPid)
        {
            stPlmnWithRat.enRat         = NAS_MML_NET_RAT_TYPE_WCDMA;

        }
        else
        {
            stPlmnWithRat.enRat         = NAS_MML_NET_RAT_TYPE_GSM;
        }

        /* 构造带接入技术的PLMN ID */
        stPlmnWithRat.stPlmnId      = pstEquPlmnList->astEquPlmnAddr[i];

        /* 判断等效PLMN是否在禁止接入技术的PLMN列表中,如果在则不将该PLMN ID发给RRC */
        if (VOS_TRUE == NAS_MML_IsPlmnIdWithRatInDestDisabledPlmnWithForbiddenPeriodList(&stPlmnWithRat))
        {
            continue;
        }

        if (VOS_TRUE == NAS_MML_IsPlmnIdInForbidPlmnList(&(pstEquPlmnList->astEquPlmnAddr[i])))
        {
            continue;
        }

        stEplmn.aEquPlmnIdList[stEplmn.ulEquPlmnNum].ulMcc = pstEquPlmnList->astEquPlmnAddr[i].ulMcc;
        stEplmn.aEquPlmnIdList[stEplmn.ulEquPlmnNum].ulMnc = pstEquPlmnList->astEquPlmnAddr[i].ulMnc;
        stEplmn.ulEquPlmnNum++;
    }


    NAS_MMC_SndAsEquPlmnQueryCnf(&stEplmn, ulOpCurPlmnId, &stCurPlmnId, pstMsg->ulSenderPid);

    return VOS_TRUE;

}


VOS_UINT32  NAS_MMC_RcvRrMmLimitServiceCamp_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    RRMM_LIMIT_SERVICE_CAMP_IND_STRU    *pstLimitServiceMsg;
    NAS_MMC_PLMN_SEARCH_CTRL_CTX_STRU   *pstSearchCtrl;

    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enCsMsccServiceStatus;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enPsMsccServiceStatus;
    NAS_MML_REG_STATUS_ENUM_UINT8        enCsRegStatus;
    NAS_MML_REG_STATUS_ENUM_UINT8        enPsRegStatus;

    pstSearchCtrl        = NAS_MMC_GetPlmnSearchCtrl();

    pstLimitServiceMsg   = (RRMM_LIMIT_SERVICE_CAMP_IND_STRU *)pstMsg;

    if ( NAS_MML_NET_RAT_TYPE_WCDMA != NAS_MML_GetCurrNetRatType())
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvRrMmLimitServiceCamp_PreProc: receive wrong message");
        return VOS_TRUE;
    }

    if(VOS_FALSE == NAS_MML_GetSimPresentStatus())
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvRrMmLimitServiceCamp_PreProc: sim is not present");
        return VOS_TRUE;
    }


    if (NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_WAS_LIMITED_CAMP);
    }
    else
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_TAS_LIMITED_CAMP);
    }

    if (RRC_LIMIT_CAMP_ACTION_START == pstLimitServiceMsg->enLimitCampAction)
    {
        pstSearchCtrl->ucWrrLimitServiceInd     = VOS_TRUE;

        if (VOS_FALSE == pstLimitServiceMsg->ulServiceAvailFlag)
        {
            /* RRMM_LIMIT_SERVICE_CAMP_IND时系统消息不会变化，为了保证MMA可以延迟上报，需要PS和CS的服务状态一起上报 */
            NAS_MMC_SndMsccServiceStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,
                                           NAS_MSCC_PIF_SERVICE_STATUS_NO_SERVICE);

        }
        else
        {
            /* RRMM_LIMIT_SERVICE_CAMP_IND时系统消息不会变化，为了保证MMA可以延迟上报，需要PS和CS的服务状态一起上报 */
            NAS_MMC_SndMsccServiceStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,
                                           NAS_MSCC_PIF_SERVICE_STATUS_LIMITED_SERVICE);

        }

        enCsRegStatus = NAS_MML_GetCsRegStatus();
        enPsRegStatus = NAS_MML_GetPsRegStatus();

        if ( (NAS_MML_REG_REGISTERED_HOME_NETWORK == enCsRegStatus)
          || (NAS_MML_REG_REGISTERED_ROAM == enCsRegStatus) )
        {

            NAS_MMC_ErrLogRecordCsOosCause(enCsRegStatus, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ErrLogClrNwSearchCount(enCsRegStatus, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);

            /* 上报注册状态发生改变 */
            NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
        }

        if ( (NAS_MML_REG_REGISTERED_HOME_NETWORK == enPsRegStatus)
          || (NAS_MML_REG_REGISTERED_ROAM == enPsRegStatus) )
        {
            /* 上报注册状态发生改变 */
            NAS_MMC_ErrLogRecordPsOosCause(enPsRegStatus, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ErrLogClrNwSearchCount(enPsRegStatus, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);

            NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
        }

        /* 只有在可以提供服务,存在RRC连接,当前的操作为start,
           并且上报的PLMN个数不为0时,需要发起搜网,进状态机处理，否则无需进状态机处理返回VOS_TRUE */
        if (VOS_FALSE == NAS_MMC_IsNeedPlmnSearch_Oos(pstLimitServiceMsg))
        {
            return VOS_TRUE;
        }

        return VOS_FALSE;
    }
    else if (RRC_LIMIT_CAMP_ACTION_END == pstLimitServiceMsg->enLimitCampAction)
    {
        pstSearchCtrl->ucWrrLimitServiceInd     = VOS_FALSE;

        /* ulServiceAvailFlag为VOS_TRUE,代表重进服务区，上报之前的服务状态 */
        if (VOS_TRUE == pstLimitServiceMsg->ulServiceAvailFlag)
        {
            enCsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);
            enPsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);

            NAS_MMC_SetAsCellCampOn(NAS_MMC_AS_CELL_CAMP_ON);

            NAS_MMC_SndMsccServiceStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS,
                                           enCsMsccServiceStatus);

            NAS_MMC_SndMsccServiceStatusInd(NAS_MSCC_PIF_SRVDOMAIN_PS,
                                           enPsMsccServiceStatus);

            /* 上报注册状态发生改变 */
            NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MML_GetCsRegStatus());

            /* 上报注册状态发生改变 */
            NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MML_GetPsRegStatus());
        }
        /* ulServiceAvailFlag为VOS_FALSE代表出服务区或模式切换，此处不上报服务状态 */
        else
        {
            /* PS卡无效或CS卡无效,如果W上报LIMITED CAMP ON指示结束后,需要再上报一次卡状态 */
            if (VOS_FALSE == NAS_MML_GetSimCsRegStatus())
            {
                /* 上报注册状态发生改变 */
                NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MML_GetCsRegStatus());
            }

            if (VOS_FALSE == NAS_MML_GetSimPsRegStatus())
            {
                /* 上报注册状态发生改变 */
                NAS_MMC_SndMsccRegStatusInd(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MML_GetPsRegStatus());
            }
        }

        return VOS_FALSE;
    }
    else
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvRrMmLimitServiceCamp_PreProc: receive wrong message");
        return VOS_TRUE;
    }
}


VOS_VOID NAS_MMC_ProcActingHplmnRefresh_PreProc(
    VOS_UINT8                              *pucBufferRefresh
)
{
    VOS_UINT32                              ulEfLen;
    VOS_UINT8                              *pucEf               = VOS_NULL_PTR;
    NAS_MML_SIM_EHPLMN_INFO_STRU           *pstEHplmnList       = VOS_NULL_PTR;
    /* Added by h00313353 for iteration 9, 2015-2-9, begin */
    VOS_UINT32                              ulFileId;

    if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
    {
        ulFileId = USIMM_ATTUSIM_EFACTINGHPLMN_ID;
    }
    else
    {
        ulFileId = USIMM_ATTGSM_EFACTINGHPLMN_ID;
    }
    /* Added by h00313353 for iteration 9, 2015-2-9, end */

    *pucBufferRefresh = VOS_FALSE;


    ulEfLen = 0;

    /* 如果搜索控制NV未打开，直接返回 */
    if (VOS_FALSE == NAS_MML_GetActingHplmnSupportFlg())
    {
        return;
    }

    pstEHplmnList                       = NAS_MML_GetSimEhplmnList();

    /* 调用API读取USIM文件 */
    /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
    if (USIMM_API_SUCCESS != NAS_USIMMAPI_GetCachedFile(ulFileId, &ulEfLen, &pucEf, USIMM_GUTL_APP))
    /* Modified by h00313353 for iteration 9, 2015-2-9, end */
    {
        /*读当前文件失败，读取下一个文件*/
        NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_ProcActingHplmnRefresh_PreProc():ERROR:READ FILE FAIL!");
        return;
    }

    /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
    NAS_MMC_SndOmGetCacheFile(ulFileId, ulEfLen, pucEf);
    /* Modified by h00313353 for iteration 9, 2015-2-9, end */

    /* BG搜状态机正在运行 */
    if (NAS_MMC_FSM_BG_PLMN_SEARCH == NAS_MMC_GetCurrFsmId())
    {
        *pucBufferRefresh = VOS_TRUE;
    }

    /* 读取文件成功后，MMC内部做相应处理 */
    /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
    NAS_MMC_ProcAgentUsimReadFileCnf_PreProc((VOS_UINT16)ulFileId, (VOS_UINT16)ulEfLen, pucEf);
    /* Modified by h00313353 for iteration 9, 2015-2-9, end */

    /* 更新HPLMN接入技术 */
    NAS_MMC_UpdateEhplmnRat();

    /* 将HPLMN冲禁止列表中删除 */
    if (VOS_FALSE == NAS_MML_DelForbPlmn(&(pstEHplmnList->astEhPlmnInfo[0].stPlmnId)))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_ProcActingHplmnRefresh_PreProc():ERROR:NAS_MML_DelForbPlmn FAIL!");
    }

    /* 初始化高优先级搜网列表 */
    NAS_MMC_InitPlmnSelectionList(NAS_MMC_PLMN_SEARCH_SCENE_HIGH_PRIO_PLMN_SEARCH,
                                  VOS_NULL_PTR,
                                  NAS_MMC_GetHighPrioPlmnList());

    /* 向接入层发送当前的HPLMN和EHPLMN */
    NAS_MMC_SndAsHplmnReq(UEPS_PID_GAS);
    NAS_MMC_SndAsHplmnReq(WUEPS_PID_WRR);

    /* 停止高优先定时器和周期性尝试定时器 */
    NAS_MMC_StopTimer(TI_NAS_MMC_HPLMN_TIMER);
    NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH);
    NAS_MMC_StopTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER);
    NAS_MMC_ResetCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
    NAS_MMC_InitTdHighRatSearchCount();

    /* 判断是否需要发起BG搜 */


    if (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        return;
    }

    if (VOS_TRUE == NAS_MMC_IsCampOnHighestPrioPlmn())
    {
        return;
    }

    /* 启动高优先级尝试定时器，时长1s */
    (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH, TI_NAS_MMC_ACTING_HPLMN_REFRESH_TRYING_HIGH_PRIO_PLMN_SEARCH_LEN);

    return;
}

/* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
/*****************************************************************************
 函 数 名  : NAS_MMC_DelayBgSearch
 功能描述  : 高优先级网络文件更新触发背景搜
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无

 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2015年04月28日
   作    者   : c00318887
   修改内容   : 新生成函数

*****************************************************************************/
VOS_VOID NAS_MMC_DelayBgSearch(VOS_VOID)
{
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH))
    {
        return;
    }

    /* 如果HPLMN定时器正在运行,停止该定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_HPLMN_TIMER))
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_HPLMN_TIMER);

        /* HPLMN TIMER停止事件上报 */
        NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_STOP,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

    }

    if (VOS_TRUE == NAS_MMC_IsNeedStartHPlmnTimer())
    {
        /* 重新启动HPLMN */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER,
                           NAS_MML_GetHighPrioPlmnRefreshTriggerBGSearchDelayLen());

        /* HPLMN TIMER启动事件上报 */
        NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

        /* 将HPLMN启动类型设置为非首次启动 */
        NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);
    }

}
VOS_UINT32  NAS_MMC_ProHighPrioPlmnRefreshInd_PreProc(VOS_VOID)
{

    /* 当前状态机标识 */
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

    /* 当前状态机的状态 */
    VOS_UINT32                          ulState;

    enFsmId = NAS_MMC_GetCurrFsmId();
    ulState = NAS_MMC_GetFsmTopState();

    /* 通知MSCC优选PLMN信息 */
    NAS_MMC_SndMsccPrefPlmnInfoInd(NAS_MSCC_PREF_PLMN_OPLMN);

    if (VOS_FALSE == NAS_MML_Get3GPP2UplmnNotPrefFlg())
    {
        NAS_MMC_SndMsccPrefPlmnInfoInd(NAS_MSCC_PREF_PLMN_UPLMN);
    }

    /* BG搜状态机正在运行,缓存当前消息，待BG状态机退出后处理 */
    if ( NAS_MMC_FSM_BG_PLMN_SEARCH == enFsmId )
    {
        return VOS_FALSE;
    }

    /* 初始化高优先级搜网列表 */
    NAS_MMC_InitPlmnSelectionList(NAS_MMC_PLMN_SEARCH_SCENE_HIGH_PRIO_PLMN_SEARCH,
                                  VOS_NULL_PTR,
                                  NAS_MMC_GetHighPrioPlmnList());

    /* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */

    if (VOS_TRUE == NAS_MML_GetHighPrioPlmnRefreshTriggerBGSearchFlag())
    {
        NAS_MMC_DelayBgSearch();
    }
    /* Added by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */

    /* 在异系统，ON Plmn状态尝试启动高优先级搜网定时器 */
    if ( (NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_OOS       == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_HO        == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_CCO       == enFsmId)
      || ( (NAS_MMC_FSM_L1_MAIN           == enFsmId)
        && (NAS_MMC_L1_STA_ON_PLMN        == ulState) ) )
    {
        /* 判断是否启动HPLMN */
        if (VOS_TRUE == NAS_MMC_IsNeedStartHPlmnTimer())
        {
            /* 启动定时器 */
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER, NAS_MMC_GetHplmnTimerLen());

            /* HPLMN TIMER启动事件上报 */
            NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                            VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

            /* 将HPLMN启动类型设置为非首次启动 */
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);
        }

        if (VOS_TRUE == NAS_MMC_IsNeedStartHighPrioRatHPlmnTimer())
        {
            /* 启动定时器 */
            if (VOS_TRUE == NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen()))
            {
                NAS_MMC_AddCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
                NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();
            }
        }
    }

    /* 消息处理完成，直接退出 */
    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_IsHighPrioPlmnFileRefreshed
 功能描述  : 判断refresh的文件是否是与高优先级搜网相关
 输入参数  : usEfId - refresh的文件id
 输出参数  : 无
 返 回 值  : VOS_TRUE
             VOS_FALSE
 调用函数  :
 被调函数  :

 修改历史      :
10.日    期   : 2015年3月2日
   作    者   : c00318887
   修改内容   : AT&T phaseII增加0x6F7B的处理

*****************************************************************************/
VOS_UINT32  NAS_MMC_IsHighPrioPlmnFileRefreshed(
    VOS_UINT32                          ulFileId
)
{
    VOS_UINT32                          ulNvOplmnAvail;

    /* 卡里UPLMN和OPLMN的个数为0，会使用NAS_MML_READ_PLMN_SEL_FILE_ID卡文件内容初始化高优先级列表 */
    ulNvOplmnAvail = NAS_MML_IsNvimOplmnAvail();

    if ((USIMM_USIM_EFPLMNWACT_ID == ulFileId)
     || (USIMM_GSM_EFPLMNWACT_ID == ulFileId)
     || ( (VOS_FALSE == ulNvOplmnAvail)
       && ((USIMM_USIM_EFOPLMNWACT_ID == ulFileId)
        || (USIMM_GSM_EFOPLMNWACT_ID  == ulFileId)) )
     || (USIMM_GSM_EFPLMNSEL_ID == ulFileId)
     || (USIMM_USIM_EFFPLMN_ID  == ulFileId)
     || (USIMM_GSM_EFFPLMN_ID   == ulFileId))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}



VOS_UINT32  NAS_MMC_RcvUsimRefreshFileInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* Modified by h00313353 for iteration 9, 2015-2-6, begin */
    USIMM_STKREFRESH_IND_STRU          *pstUsimRefreshInd = VOS_NULL_PTR;
    /* Modified by h00313353 for iteration 9, 2015-2-6, end */
    VOS_UINT32                          i;
    VOS_UINT32                          ulHighPrioPlmnRefreshFlag;
    VOS_UINT32                          ulFileId;
    VOS_UINT32                          ulEfLen;
    VOS_UINT8                          *pucEf = VOS_NULL_PTR;

    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
    NAS_MMC_USIMM_REFRESH_FILE_CONTENT_STRU    *pstRefreshFileContent   = VOS_NULL_PTR;
    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */

    VOS_UINT8                           ucBufferRefreshFlg;

    NAS_MML_SIM_EHPLMN_INFO_STRU       *pstEHPlmnInfo = VOS_NULL_PTR;

    pstEHPlmnInfo                       = NAS_MML_GetSimEhplmnList();

    pstUsimRefreshInd                   = (USIMM_STKREFRESH_IND_STRU *)pstMsg;

    if ( (USIMM_REFRESH_FILE_LIST != pstUsimRefreshInd->enRefreshType)
      /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
      && (USIMM_REFRESH_ALL_FILE != pstUsimRefreshInd->enRefreshType))
      /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */
    {
        /* refresh消息目前只处理指定文件变更类型和ALL类型 */
        return VOS_TRUE;
    }

    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
    pstRefreshFileContent = (NAS_MMC_USIMM_REFRESH_FILE_CONTENT_STRU*)PS_MEM_ALLOC(WUEPS_PID_MMC,
                                         sizeof(NAS_MMC_USIMM_REFRESH_FILE_CONTENT_STRU));

    if (VOS_NULL_PTR == pstRefreshFileContent)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvUsimRefreshFileInd_PreProc:ERROR:Alloc Mem Fail.");
        return VOS_TRUE;
    }

    PS_MEM_SET(pstRefreshFileContent, 0x00, sizeof(NAS_MMC_USIMM_REFRESH_FILE_CONTENT_STRU));

    NAS_MMC_BuildRefreshFileList(pstUsimRefreshInd, pstRefreshFileContent);
    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */

    ulHighPrioPlmnRefreshFlag   =  VOS_FALSE;

    ucBufferRefreshFlg          =  VOS_FALSE;


    ulEfLen                     = 0;

    /* 一次遍历USIM上报的文件ID */
    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, begin */
    for (i = 0; i < pstRefreshFileContent->usEfNum; i++ )
    {
        ulFileId = pstRefreshFileContent->astRefreshFile[i].usFileId;

        /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
        if (((USIMM_ATTGSM_EFACTINGHPLMN_ID == ulFileId)
        || (USIMM_ATTUSIM_EFACTINGHPLMN_ID == ulFileId))
         && (USIMM_GUTL_APP == pstRefreshFileContent->astRefreshFile[i].enAppType))
         /* Modified by h00313353 for iteration 9, 2015-2-9, end */
        {
            NAS_MMC_ProcActingHplmnRefresh_PreProc(&ucBufferRefreshFlg);
            continue;
        }

        /* Modified by c00318887 for file refresh需要触发背景搜, 2015-6-3, begin */
        /* 跳过6f73 6f7e，否则注册时可能失败 */
        if ( (USIMM_USIM_EFPSLOCI_ID == ulFileId)
           ||(USIMM_USIM_EFLOCI_ID == ulFileId)
           ||(USIMM_GSM_EFLOCI_ID == ulFileId))
        {
            continue;
        }
        /* Modified by c00318887 for file refresh需要触发背景搜, 2015-6-3, end */

        /* 调用API读取USIM文件 */
        if (USIMM_API_SUCCESS != NAS_USIMMAPI_GetCachedFile(ulFileId, &ulEfLen, &pucEf, pstRefreshFileContent->astRefreshFile[i].enAppType))
        {
            /*读当前文件失败，读取下一个文件*/
            NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_RcvUsimRefreshFileInd_PreProc():ERROR:READ FILE FAIL!");
            continue;
        }

        NAS_MMC_SndOmGetCacheFile(ulFileId, ulEfLen, pucEf);

        /* Modified by c00318887 for AT&T phaseII, 2015-3-2, begin */
        if (VOS_TRUE == NAS_MMC_IsHighPrioPlmnFileRefreshed(ulFileId))
        /* Modified by c00318887 for AT&T phaseII, 2015-3-2, end */
        {
            ulHighPrioPlmnRefreshFlag = VOS_TRUE;
        }

        /* 读取文件成功后，MMC内部做相应处理 */
        NAS_MMC_ProcAgentUsimReadFileCnf_PreProc((VOS_UINT16)ulFileId, (VOS_UINT16)ulEfLen, pucEf);

        /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
        if (USIMM_USIM_EFEHPLMN_ID == ulFileId)
        /* Modified by h00313353 for iteration 9, 2015-2-9, end */
        {
            /* 需要更新高优先级网络列表处理 */
            ulHighPrioPlmnRefreshFlag = VOS_TRUE;

            /* 更新HPLMN接入技术 */
            NAS_MMC_UpdateEhplmnRat();

            /* 删除FPLMN中的HPLMN */
            NAS_MMC_DelHplmnInFplmn();

            /* 向接入层发送当前的HPLMN和EHPLMN */
            NAS_MMC_SndAsHplmnReq(UEPS_PID_GAS);
            NAS_MMC_SndAsHplmnReq(WUEPS_PID_WRR);
        }
        /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
        if ((USIMM_USIM_EFHPLMNwACT_ID == ulFileId)
            || (USIMM_GSM_EFHPLMNACT_ID == ulFileId))
        /* Modified by h00313353 for iteration 9, 2015-2-9, end */
        {
            /* 需要更新高优先级网络列表处理 */
            ulHighPrioPlmnRefreshFlag = VOS_TRUE;

            /* HOME PLMN WITH RAT发生了刷新后，需要将SIM中的EHplmn进行反向构建，
              即将接入技术进行初始化为默认值，删除重复的PLMN ID */
            NAS_MML_ResumeSimEhplmnList(&pstEHPlmnInfo->ucEhPlmnNum,
                                         pstEHPlmnInfo->astEhPlmnInfo);

            /* 更新HPLMN接入技术 */
            NAS_MMC_UpdateEhplmnRat();
        }

        /* 6F31文件处理搜索控制 */
        /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
        if ((USIMM_USIM_EFHPPLMN_ID == ulFileId)
            || (USIMM_GSM_EFHPLMN_ID == ulFileId))
        /* Modified by h00313353 for iteration 9, 2015-2-9, end */
        {
            NAS_MMC_ProcScanCtrl_PreProc((VOS_UINT16)ulEfLen, pucEf);
        }

    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstRefreshFileContent);
    /* Modified by c00318887 for file refresh需要触发背景搜, 2015-4-28, end */




    /* H、U、O文件发生变化需要刷新时,如果BG搜网状态机在运行时,为避免对当前搜网列表产生影响
       先缓存当前USIM文件刷新消息待BG搜网状态机退出后再刷新高优先级搜网列表;否则直接刷高优
       先级搜网列表 */

    if (VOS_TRUE == ucBufferRefreshFlg)
    {
        return VOS_FALSE;
    }

    if (VOS_TRUE == ulHighPrioPlmnRefreshFlag)
    {
        return NAS_MMC_ProHighPrioPlmnRefreshInd_PreProc();
    }


    return VOS_FALSE;
}




VOS_UINT32  NAS_MMC_RcvMsccUpdateUplmnNtf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulRslt;

    /* 进行高优先级列表刷新的预处理 */
    ulRslt = NAS_MMC_ProHighPrioPlmnRefreshInd_PreProc();

    return ulRslt;
}
VOS_VOID  NAS_MMC_ProcAgentUsimReadFileCnf_PreProc(
    VOS_UINT16                          usEfId,
    VOS_UINT16                          usEfLen,
    VOS_UINT8                          *pucEf
)
{
    USIMM_READFILE_CNF_STRU            *pstUsimMmcMsg   = VOS_NULL_PTR;
    VOS_CHAR                           *pucFilePathStr  = VOS_NULL_PTR;
    /* Added by h00313353 for iteration 9, 2015-2-17, begin */
    VOS_UINT32                          ulPathLength;

    ulPathLength    = 0;

    if (VOS_OK != USIMM_ChangeDefFileToPath(usEfId, &pucFilePathStr))
    {
        return;
    }

    /* 文件路径长度保护 */
    ulPathLength = VOS_StrLen(pucFilePathStr);
    if (0 == ulPathLength)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_SndUsimmUpdateFileReq: File Path Str Error.");

        return;
    }
    /* Added by h00313353 for iteration 9, 2015-2-17, end */

    /*lint -e961*/
    pstUsimMmcMsg = (USIMM_READFILE_CNF_STRU *)PS_MEM_ALLOC(WUEPS_PID_MMC, (VOS_UINT32)(sizeof(USIMM_READFILE_CNF_STRU) - 4 + usEfLen));
    /*lint +e961*/
    if (VOS_NULL_PTR == pstUsimMmcMsg)
    {
       NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_ProcAgentUsimReadFileCnf_PreProc():ERROR:MALLOC FAIL");
       return;
    }
    /*lint -e961*/
    PS_MEM_SET(pstUsimMmcMsg, 0, (VOS_UINT32)(sizeof(USIMM_READFILE_CNF_STRU) - 4 + usEfLen));
    /*lint +e961*/
    /* 读取的USIM文件内容拷贝本地 */
    pstUsimMmcMsg->stCmdResult.ulResult = VOS_OK;
    pstUsimMmcMsg->usEfLen  = usEfLen;
    /* Modified by h00313353 for iteration 9, 2015-2-6, begin */
    pstUsimMmcMsg->stFilePath.ulPathLen = ulPathLength;
    PS_MEM_CPY(pstUsimMmcMsg->stFilePath.acPath, pucFilePathStr, pstUsimMmcMsg->stFilePath.ulPathLen + 1);
    /* Modified by h00313353 for iteration 9, 2015-2-6, end */
    PS_MEM_CPY(pstUsimMmcMsg->aucEf, pucEf, usEfLen);

    NAS_MMC_RcvAgentUsimReadFileCnf((PS_USIM_GET_FILE_RSP_FOR_PCLINT_STRU*)pstUsimMmcMsg);

    PS_MEM_FREE(WUEPS_PID_MMC, pstUsimMmcMsg);

}


VOS_UINT32 NAS_MMC_IsMsccAttachTypeValid(
    NAS_MSCC_PIF_ATTACH_TYPE_ENUM_UINT32                    enAttachType
)
{
    if ((NAS_MSCC_PIF_ATTACH_TYPE_IMSI      != enAttachType)
     && (NAS_MSCC_PIF_ATTACH_TYPE_GPRS      != enAttachType)
     && (NAS_MSCC_PIF_ATTACH_TYPE_GPRS_IMSI != enAttachType))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMsccAttachReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 根据需要发起的Attach类型通知MM,GMM发起注册 */
    MSCC_MMC_ATTACH_REQ_STRU           *pstTafAttachReq;
    VOS_UINT32                          ulForbType = NAS_MML_PLMN_FORBID_NULL;

#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     ucCurrNetRatType;

    NAS_MML_LTE_CAPABILITY_STATUS_ENUM_UINT32               enLteCapabilityStatus;
#endif


    pstTafAttachReq = (MSCC_MMC_ATTACH_REQ_STRU*)pstMsg;

#if (FEATURE_ON == FEATURE_LTE)
    ucCurrNetRatType = NAS_MML_GetCurrNetRatType();

    enLteCapabilityStatus = NAS_MML_GetLteCapabilityStatus();
#endif

    /* attach类型错误，丢弃 */
    if (VOS_FALSE == NAS_MMC_IsMsccAttachTypeValid(pstTafAttachReq->enAttachType))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_RcvMsccAttachReq_PreProc: NORMAL: Attach Type is invalid.");
        return VOS_TRUE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    if ((VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
     && (NAS_MSCC_PIF_ATTACH_TYPE_IMSI != pstTafAttachReq->enAttachType))
    {
        if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_AttachPs())
        {
            /* LTE国际漫游导致的disable LTE,此时不能通知GU接入层 */
            if ((VOS_FALSE == NAS_MML_GetDisableLteRoamFlg())
             && (NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == enLteCapabilityStatus))
            {
                NAS_MMC_SndEnableLteNotify();
                NAS_MMC_ClearDisabledLtePlmnId();
                NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
        }
    }


#endif



    if (NAS_MMC_SPEC_PLMN_SEARCH_STOP == NAS_MMC_GetSpecPlmnSearchState())
    {
        ulForbType = NAS_MMC_GetCurrentLaiForbbidenType();
    }

    NAS_MMC_SetAttachReqOpId(pstTafAttachReq->ulOpID);

#if (FEATURE_ON == FEATURE_LTE)
    /* 当前在L下，黑名单禁止L的时候，通知LMM进行ATTACH,在收到LTE的ATTACH CNF时候，判断是否需要进行搜网 */
    if ( (VOS_TRUE                 == NAS_MML_IsRatInForbiddenList(NAS_MML_NET_RAT_TYPE_LTE))
      && (NAS_MML_NET_RAT_TYPE_LTE == ucCurrNetRatType) )
    {
         NAS_MMC_SndLmmAttachReq(pstTafAttachReq->ulOpID, pstTafAttachReq->enAttachType, pstTafAttachReq->enEpsAttachReason);

#ifdef NAS_STUB
        if (NAS_MSCC_PIF_ATTACH_TYPE_IMSI != pstTafAttachReq->enAttachType)
        {
            NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);
        }
#endif
#ifdef __PC_TRACE_RECUR__
        if (NAS_MSCC_PIF_ATTACH_TYPE_IMSI != pstTafAttachReq->enAttachType)
        {
            NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);
        }
#endif
        return VOS_TRUE;
    }
#endif

    if (NAS_MML_PLMN_FORBID_NULL != ulForbType)
    {
        if (VOS_TRUE == NAS_MMC_ProcMsccAttachReqInForbLa_PreProc(pstTafAttachReq, ulForbType))
        {
            return VOS_TRUE;
        }
    }


#if   (FEATURE_ON == FEATURE_LTE)
    /* 向LMM发送ATTACH请求 */
    if ( NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
    {
        NAS_MMC_SndLmmAttachReq(pstTafAttachReq->ulOpID, pstTafAttachReq->enAttachType, pstTafAttachReq->enEpsAttachReason);

#ifdef NAS_STUB
        if (NAS_MSCC_PIF_ATTACH_TYPE_IMSI != pstTafAttachReq->enAttachType)
        {
            NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);
        }
#endif
#ifdef __PC_TRACE_RECUR__
        if (NAS_MSCC_PIF_ATTACH_TYPE_IMSI != pstTafAttachReq->enAttachType)
        {
            NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);
        }
#endif
    }

    else
#endif
    {
        /* 向MM发送ATTACH请求 */
        NAS_MMC_SndMmAttachReq(pstTafAttachReq->ulOpID, pstTafAttachReq->enAttachType);

        /* 向GMM发送ATTACH请求 */
        NAS_MMC_SndGmmAttachReq(pstTafAttachReq->ulOpID, pstTafAttachReq->enAttachType);
    }


    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvGmmGprsServiceInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
#if (FEATURE_ON == FEATURE_LTE)
    MMCGMM_GPRS_SERVICE_IND_STRU                           *pstGprsServiceInd;
    NAS_MML_LTE_CAPABILITY_STATUS_ENUM_UINT32               enLteCapabilityStatus;

    enLteCapabilityStatus = NAS_MML_GetLteCapabilityStatus();
    pstGprsServiceInd     = (MMCGMM_GPRS_SERVICE_IND_STRU *)pstMsg;

    if (NAS_MMC_GMM_GPRS_SERVICE_ATTACH == pstGprsServiceInd->enGprsServiceType)
    {
        if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_AttachPs())
        {
            /* LTE国际漫游导致的disable LTE,此时不能通知GU接入层 */
            if ((VOS_FALSE == NAS_MML_GetDisableLteRoamFlg())
             && (NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == enLteCapabilityStatus))
            {
                NAS_MMC_SndEnableLteNotify();
                NAS_MMC_ClearDisabledLtePlmnId();
                NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
        }
    }
#endif

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMsccSignReportReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_SIGN_REPORT_REQ_STRU       *pstSignReport = VOS_NULL_PTR;
    NAS_MMC_MAINTAIN_CTX_STRU           *pstMainCtx    = VOS_NULL_PTR;

    pstSignReport = (MSCC_MMC_SIGN_REPORT_REQ_STRU *)pstMsg;


    if ( (pstSignReport->ucActionType > NAS_MMC_AT_ACTION_TYPE_STOP)
      || (pstSignReport->ucRrcMsgType > NAS_MML_SIGN_REPORT_MSG_TYPE))
    {

        NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_RcvMsccSignReportReq_PreProc: NORMAL: ucActionType or ucRrcMsgType is invalid.");
        return VOS_TRUE;
    }

    pstMainCtx = NAS_MMC_GetMaintainInfo();

    pstMainCtx->ucActionType  = pstSignReport->ucActionType;
    pstMainCtx->ucMsgType     = pstSignReport->ucRrcMsgType;

    if ((pstSignReport->ucSignThreshold != pstMainCtx->ucSignThreshold)
     || (pstSignReport->ucMinRptTimerInterval != pstMainCtx->ucMinRptTimerInterval))
    {
        pstMainCtx->ucSignThreshold       = pstSignReport->ucSignThreshold;
        pstMainCtx->ucMinRptTimerInterval = pstSignReport->ucMinRptTimerInterval;

        /* 信号质量门限或者间隔时间改变时写nv */
        NAS_MMC_WriteCellSignReportCfgNvim(pstMainCtx->ucSignThreshold,pstMainCtx->ucMinRptTimerInterval);
    }

    /* 通知接入层信号质量门限和时间间隔 */
    NAS_MMC_SndWasInfoReportReq();
    NAS_MMC_SndGasInfoReportReq();
#if (FEATURE_LTE == FEATURE_ON)
    NAS_MMC_SndLmmCellSignReportNotify();
#endif

    return VOS_TRUE;
}



#if (FEATURE_MULTI_MODEM == FEATURE_ON)

VOS_UINT32  NAS_MMC_RcvMsccOtherModemInfoNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
#if   (FEATURE_ON == FEATURE_LTE)
    NAS_MML_PLMN_ID_STRU                                    stOtherModemPlmnId;
    MMC_LMM_OTHER_MODEM_INFO_NOTIFY_STRU                    stSndLmmOtherModemInfoMsg;
#endif

    MSCC_MMC_OTHER_MODEM_INFO_NOTIFY_STRU                   *pstOtherModemInfoNotify    = VOS_NULL_PTR;

    NAS_MMC_NCELL_SEARCH_INFO_STRU                         *pstMmcNcellSearchInfo = VOS_NULL_PTR;

    pstMmcNcellSearchInfo = NAS_MMC_GetNcellSearchInfo();

    pstOtherModemInfoNotify = (MSCC_MMC_OTHER_MODEM_INFO_NOTIFY_STRU *)pstMsg;

#if (FEATURE_ON == FEATURE_LTE)
    PS_MEM_SET(&stSndLmmOtherModemInfoMsg, 0, sizeof(stSndLmmOtherModemInfoMsg));

    if (VOS_TRUE == pstOtherModemInfoNotify->bitOpCurrCampPlmnId)
    {
        stOtherModemPlmnId.ulMcc = pstOtherModemInfoNotify->stCurrCampPlmnId.ulMcc;
        stOtherModemPlmnId.ulMnc = pstOtherModemInfoNotify->stCurrCampPlmnId.ulMnc;
        NAS_MMC_SetOtherModemPlmnId(&stOtherModemPlmnId);

        /* 平台能力支持L则通知L */
        if (VOS_TRUE == NAS_MML_IsPlatformSupportLte())
        {
            stSndLmmOtherModemInfoMsg.bitOpCurrCampPlmnId = VOS_TRUE;
            NAS_MML_ConvertNasPlmnToLMMFormat(&(stSndLmmOtherModemInfoMsg.stCurrCampPlmnId), &stOtherModemPlmnId);

            /* 通知L副卡驻留plmn信息 */
            NAS_MMC_SndLmmOtherModemInfoNotify(&stSndLmmOtherModemInfoMsg);
        }
    }
#endif

    if (VOS_TRUE == pstOtherModemInfoNotify->bitOpEplmnInfo)
    {
        pstMmcNcellSearchInfo->stOtherModemEplmnInfo.ucEquPlmnNum = pstOtherModemInfoNotify->stEplmnInfo.ucEquPlmnNum;
        PS_MEM_CPY(pstMmcNcellSearchInfo->stOtherModemEplmnInfo.astEquPlmnAddr, pstOtherModemInfoNotify->stEplmnInfo.astEquPlmnAddr,
                   sizeof(pstMmcNcellSearchInfo->stOtherModemEplmnInfo.astEquPlmnAddr));
    }

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMsccOtherModemDplmnNplmnInfoNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_OTHER_MODEM_DPLMN_NPLMN_INFO_NOTIFY_STRU       *pstOtherModemDplmnNplmnInfoNotify    = VOS_NULL_PTR;
    NAS_MMC_DPLMN_NPLMN_CFG_INFO_STRU                      *pstDPlmnNPlmnCfgInfo = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU                                    stHplmnId;
    VOS_UINT8                                               ucHplmnType;
    VOS_UINT8                                              *pucImsi = VOS_NULL_PTR;
    NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU                 *pstNvimCfgDPlmnNPlmnInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulStep;
    NAS_MML_PLMN_ID_STRU                                    stTempPlmn;
    NAS_MML_SIM_FORMAT_PLMN_ID                              stNvimPlmn;
    VOS_UINT16                                              usSimRat;
    NAS_MMC_REG_DOMAIN_ENUM_UINT8                           enRegDomain;
    VOS_UINT32                                              usIsInDplmnList;
    VOS_UINT32                                              usIsInNplmnList;

    pstOtherModemDplmnNplmnInfoNotify = (MSCC_MMC_OTHER_MODEM_DPLMN_NPLMN_INFO_NOTIFY_STRU *)pstMsg;

    PS_MEM_SET(&stHplmnId, 0, sizeof(stHplmnId));
    PS_MEM_SET(&stTempPlmn, 0, sizeof(stTempPlmn));
    PS_MEM_SET(&stNvimPlmn, 0, sizeof(stNvimPlmn));
    enRegDomain              = NAS_MMC_REG_DOMAIN_BUTT;

    /* 取得手机卡中IMSI的信息 */
    pucImsi    = NAS_MML_GetSimImsi();

    /* 从当前的IMSI中取出home plmn */
    stHplmnId                = NAS_MML_GetImsiHomePlmn(pucImsi);
    pstDPlmnNPlmnCfgInfo     = NAS_MMC_GetDPlmnNPlmnCfgInfo();
    ucHplmnType              = NAS_MMC_JudegeHplmnType(&stHplmnId);

    if (VOS_FALSE == pstDPlmnNPlmnCfgInfo->ucActiveFlg)
    {
        return VOS_TRUE;
    }

    pstNvimCfgDPlmnNPlmnInfo = (NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));

    if (VOS_NULL_PTR == pstNvimCfgDPlmnNPlmnInfo)
    {
        return VOS_TRUE;
    }

    if (NAS_MMC_HPLMN_TYPE_CMCC == ucHplmnType)
    {
        PS_MEM_CPY(pstNvimCfgDPlmnNPlmnInfo,
            &pstOtherModemDplmnNplmnInfoNotify->stCmccDplmnNplmnInfo, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));
    }
    else if (NAS_MMC_HPLMN_TYPE_UNICOM == ucHplmnType)
    {
        PS_MEM_CPY(pstNvimCfgDPlmnNPlmnInfo,
            &pstOtherModemDplmnNplmnInfoNotify->stUnicomDplmnNplmnInfo, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));
    }
    else if (NAS_MMC_HPLMN_TYPE_CT == ucHplmnType)
    {
        PS_MEM_CPY(pstNvimCfgDPlmnNPlmnInfo,
            &pstOtherModemDplmnNplmnInfoNotify->stCtDplmnNplmnInfo, sizeof(NAS_MMC_NVIM_CFG_DPLMN_NPLMN_INFO_STRU));
    }
    else
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);
        return VOS_TRUE;
    }

    /* 把other modem的DPLMN和本modem的DPLMN拼起来，nplmn不拼 */
    for ( ulStep = 0; ulStep < pstNvimCfgDPlmnNPlmnInfo->usDplmnListNum; ulStep++ )
    {
        /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-30, begin */
        (VOS_VOID)PS_MEM_CPY(stNvimPlmn.aucSimPlmn,
            pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList + (ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN), NAS_MML_SIM_PLMN_ID_LEN);

        /* 转换PLMN ID为MMC内部的格式并存储 */
        NAS_MMC_ConvertSimPlmnToNasPLMN(&stNvimPlmn, &stTempPlmn);

        /* 存储PLMN对应的RAT */
        usSimRat    = (VOS_UINT16)(pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + NAS_MML_PLMN_WITH_RAT_FIRST_RAT_OFFSET] << NAS_MML_OCTET_MOVE_EIGHT_BITS)
                                 | pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + NAS_MML_PLMN_WITH_RAT_SECOND_RAT_OFFSET];

        enRegDomain = (VOS_UINT8)(pstNvimCfgDPlmnNPlmnInfo->aucDPlmnList[(ulStep * NAS_MMC_DPLMN_NPLMN_NV_INFO_LEN) + NAS_MML_PLMN_WITH_RAT_UNIT_LEN]);

        /* other Modem的DPLMN拼接道本MODEM时，类型信息作为自学习类型，否则修改了本MODEM的预置类型信息 */
        /* Modified by c00318887 for DPlmn扩容和优先接入HPLMN, 2015-5-30, end */

        /* 添加LTE RAT记录 */
        if (NAS_MML_SIM_RAT_E_UTRN == (NAS_MML_SIM_RAT_E_UTRN & usSimRat))
        {
            usIsInDplmnList = VOS_FALSE;
            usIsInDplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_E_UTRN, pstDPlmnNPlmnCfgInfo->usNplmnListNum, pstDPlmnNPlmnCfgInfo->astNPlmnList);
            usIsInNplmnList = VOS_FALSE;
            usIsInNplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_E_UTRN, pstDPlmnNPlmnCfgInfo->usDplmnListNum, pstDPlmnNPlmnCfgInfo->astDPlmnList);
            /* 若是需要添加的DPLMN不在NPLMN也不在DPLMN中，则需要添加 */
            if ((VOS_TRUE != usIsInDplmnList)
                && (VOS_TRUE != usIsInNplmnList))
            {
                NAS_MMC_UpdateDPlmnNPlmnList(&stTempPlmn,NAS_MML_NET_RAT_TYPE_LTE,enRegDomain,&pstDPlmnNPlmnCfgInfo->usDplmnListNum,pstDPlmnNPlmnCfgInfo->astDPlmnList);
            }
        }

        /* 添加WCDMA RAT记录 */
        if (NAS_MML_SIM_RAT_UTRN == (NAS_MML_SIM_RAT_UTRN & usSimRat))
        {
            usIsInDplmnList = VOS_FALSE;
            usIsInDplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_UTRN, pstDPlmnNPlmnCfgInfo->usNplmnListNum, pstDPlmnNPlmnCfgInfo->astNPlmnList);
            usIsInNplmnList = VOS_FALSE;
            usIsInNplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_UTRN, pstDPlmnNPlmnCfgInfo->usDplmnListNum, pstDPlmnNPlmnCfgInfo->astDPlmnList);
            /* 若是需要添加的DPLMN在NPLMN中，则不需要添加 */
            if ((VOS_TRUE != usIsInDplmnList)
                && (VOS_TRUE != usIsInNplmnList))
            {
                NAS_MMC_UpdateDPlmnNPlmnList(&stTempPlmn,NAS_MML_NET_RAT_TYPE_WCDMA,enRegDomain,&pstDPlmnNPlmnCfgInfo->usDplmnListNum,pstDPlmnNPlmnCfgInfo->astDPlmnList);
            }
        }

        /* 添加GSM RAT记录 */
        if (NAS_MML_SIM_RAT_GSM == (NAS_MML_SIM_RAT_GSM & usSimRat))
        {
            usIsInDplmnList = VOS_FALSE;
            usIsInDplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_GSM, pstDPlmnNPlmnCfgInfo->usNplmnListNum, pstDPlmnNPlmnCfgInfo->astNPlmnList);
            usIsInNplmnList = VOS_FALSE;
            usIsInNplmnList = NAS_MMC_IsPlmnIdWithSimRatInSimPlmnWithRegDomainList(&stTempPlmn, NAS_MML_SIM_RAT_GSM, pstDPlmnNPlmnCfgInfo->usDplmnListNum, pstDPlmnNPlmnCfgInfo->astDPlmnList);
            /* 若是需要添加的DPLMN在NPLMN中，则不需要添加 */
            if ((VOS_TRUE != usIsInDplmnList)
                && (VOS_TRUE != usIsInNplmnList))
            {
                NAS_MMC_UpdateDPlmnNPlmnList(&stTempPlmn,NAS_MML_NET_RAT_TYPE_GSM,enRegDomain,&pstDPlmnNPlmnCfgInfo->usDplmnListNum,pstDPlmnNPlmnCfgInfo->astDPlmnList);
            }
        }
    }

    NAS_MMC_WriteDplmnNplmnToNvim();

    NAS_MMC_LogDplmnNplmnList();

    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgDPlmnNPlmnInfo);

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvMsccNcellInfoNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_NCELL_INFO_NOTIFY_STRU                         *pstNcellInfoInd = VOS_NULL_PTR;
    NAS_MMC_NCELL_SEARCH_INFO_STRU                         *pstMmcNcellSearchInfo = VOS_NULL_PTR;
    VOS_UINT8                                               ucIsNeedNcellSearch;
    VOS_UINT8                                               ucIsNcellInfoChgFromNone;

    ucIsNcellInfoChgFromNone = VOS_FALSE;
    ucIsNeedNcellSearch      = VOS_FALSE;

    pstNcellInfoInd = (MSCC_MMC_NCELL_INFO_NOTIFY_STRU *)pstMsg;
    pstMmcNcellSearchInfo = NAS_MMC_GetNcellSearchInfo();

    if (((0 == pstMmcNcellSearchInfo->stTdsNcellInfo.ucTdsArfcnNum)
      && (0 == pstMmcNcellSearchInfo->stLteNcellInfo.ucLteArfcnNum) )
     && ((0 != pstNcellInfoInd->stTdsNcellInfo.ucTdsArfcnNum)
      || (0 != pstNcellInfoInd->stLteNcellInfo.ucLteArfcnNum) ) )
    {
        ucIsNcellInfoChgFromNone = VOS_TRUE;
    }

    /* 更新mmc 保持的ncell相关全局变量*/
    pstMmcNcellSearchInfo->stTdsNcellInfo.ucTdsArfcnNum = pstNcellInfoInd->stTdsNcellInfo.ucTdsArfcnNum;
    PS_MEM_CPY(pstMmcNcellSearchInfo->stTdsNcellInfo.ausTdsArfcnList, pstNcellInfoInd->stTdsNcellInfo.ausTdsArfcnList,
               sizeof(pstMmcNcellSearchInfo->stTdsNcellInfo.ausTdsArfcnList));

    pstMmcNcellSearchInfo->stLteNcellInfo.ucLteArfcnNum = pstNcellInfoInd->stLteNcellInfo.ucLteArfcnNum;
    /* Modified by w00316404 for R11 Update LTE Arf, 2015-07-20, begin */
    PS_MEM_CPY(pstMmcNcellSearchInfo->stLteNcellInfo.aulLteArfcnList, pstNcellInfoInd->stLteNcellInfo.aulLteArfcnList,
               sizeof(pstMmcNcellSearchInfo->stLteNcellInfo.aulLteArfcnList));
    /* Modified by w00316404 for R11 Update LTE Arf, 2015-07-20, end */


    /* 收到tds和lte邻区频点信息，如果此时普通的available定时器在运行，
       进层1 mian处理，需要停止重新启动1s定时器，1s定时器超时发起ncell搜 */
    if (NAS_MMC_FSM_L1_MAIN == NAS_MMC_GetCurrFsmId())
    {
        ucIsNeedNcellSearch = NAS_MMC_IsNeedNcellSearch();
    }

    if ((NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER))
     && (VOS_TRUE == ucIsNeedNcellSearch)
     && (VOS_TRUE == ucIsNcellInfoChgFromNone))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvMsccPsTransferInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PS_TRANSFER_NOTIFY_STRU       *pstPsTransferInd    = VOS_NULL_PTR;
    NAS_MML_AVAIL_TIMER_CFG_STRU       *pstAvailTimerCfg    = VOS_NULL_PTR;
    NAS_MMC_TIMER_STATUS_ENUM_U8        enAvailableTimerStatus;
    VOS_UINT8                           ucPsAttachAllow;
    NAS_MML_MS_MODE_ENUM_UINT8          enMsMode;

    enAvailableTimerStatus = NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER);
    ucPsAttachAllow        = NAS_MML_GetPsAttachAllowFlg();
    enMsMode               = NAS_MML_GetMsMode();

    pstPsTransferInd       = (MSCC_MMC_PS_TRANSFER_NOTIFY_STRU *)pstMsg;

    /* 如果pstransfer上报为1，且当前available定时器在运行，则停止重新启动available定时器 */
    if ( (NAS_MMC_FSM_L1_MAIN == NAS_MMC_GetCurrFsmId())
      && (NAS_MML_MS_MODE_PS_ONLY == enMsMode)
      && (VOS_FALSE == ucPsAttachAllow)
      && (NAS_MMC_TIMER_STATUS_RUNING == enAvailableTimerStatus) )
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_AVAILABLE_TIMER);

        /* 离网方式重选不再启动定时器 */
        if (MTC_PS_TRANSFER_OFF_AREA != pstPsTransferInd->enSolutionCfg)
        {
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, NAS_MMC_GetNextAvailableTimerValue());
        }
        else
        {
            pstAvailTimerCfg           = NAS_MML_GetAvailTimerCfg();

            NAS_MMC_SetCurNormalAvailableTimerCount(pstAvailTimerCfg->ulFirstSearchTimeCount);

            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, NAS_MMC_GetNextAvailableTimerValue());
        }
    }

    return VOS_TRUE;
}


#endif


VOS_UINT32 NAS_MMC_ProcMsccAttachReqInForbLa_PreProc(
    MSCC_MMC_ATTACH_REQ_STRU           *pstTafAttachReq,
    VOS_UINT32                          ulForbType
)
{
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8                    enCsMsccServiceStatus;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8                    enPsMsccServiceStatus;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                           ulDisableLteRoamFlg;

    VOS_UINT32                           ulNeedEnableLte;
#endif

    VOS_UINT32                           ulResult;

#if (FEATURE_ON == FEATURE_LTE)
    ulDisableLteRoamFlg         = NAS_MML_GetDisableLteRoamFlg();
#endif

    enCsMsccServiceStatus       = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);
    enPsMsccServiceStatus       = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);
#if (FEATURE_ON == FEATURE_LTE)
    ulNeedEnableLte             = VOS_FALSE;
#endif

    ulResult                    = VOS_FALSE;

    switch (pstTafAttachReq->enAttachType)
    {
        case NAS_MSCC_PIF_ATTACH_TYPE_GPRS:

            NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);

            /* PS Attach是Fobid,回复Attach结果 */
            NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                                    enPsMsccServiceStatus,
                                    pstTafAttachReq->ulOpID);

#if (FEATURE_ON == FEATURE_LTE)
            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_AttachPs();
#endif

            /* 当前处于禁止网络, 不能发起 PS ATTACH 动作, 处理完毕 */
            ulResult        = VOS_TRUE;

            break;

        case NAS_MSCC_PIF_ATTACH_TYPE_IMSI :

            if (NAS_MML_PLMN_FORBID_PLMN_FOR_GPRS != ulForbType)
            {
                NAS_MML_SetCsAttachAllowFlg(VOS_TRUE);

                /* 直接回复ATTACH 结果 */
                NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                                        enCsMsccServiceStatus,
                                        pstTafAttachReq->ulOpID);


                /* 该情况不能发起 IMSI ATTACH, 处理完毕 */
                ulResult = VOS_TRUE;

            }
            else
            {
                /* GPRS禁止情况下可以发起 IMSI ATTACH, 后续还需要继续处理, 未处理完毕 */
                ulResult = VOS_FALSE;

            }

            break;

        case NAS_MSCC_PIF_ATTACH_TYPE_GPRS_IMSI :
            if (NAS_MML_PLMN_FORBID_PLMN_FOR_GPRS == ulForbType)
            {
                NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);

                /* CS+PS Attach,且gprs not allow,进行CS Attach */
                NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                                        enPsMsccServiceStatus,
                                        pstTafAttachReq->ulOpID);
                pstTafAttachReq->enAttachType = NAS_MSCC_PIF_ATTACH_TYPE_IMSI;


                /* 该情况可以发起 IMSI ATTACH, 后续还需要继续处理, 未处理完毕 */
                ulResult = VOS_FALSE;

            }
            else
            {
                NAS_MML_SetCsAttachAllowFlg(VOS_TRUE);
                NAS_MML_SetPsAttachAllowFlg(VOS_TRUE);

                /* 直接回复结果 */
                NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                                        enCsMsccServiceStatus,
                                        pstTafAttachReq->ulOpID);
                NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                                        enPsMsccServiceStatus,
                                        pstTafAttachReq->ulOpID);


                /* 该情况不能发起 GPRS_IMSI ATTACH, 处理完毕 */
                ulResult = VOS_TRUE;
            }

#if (FEATURE_ON == FEATURE_LTE)
            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_AttachCsPs();
#endif
            break;

        default:

            /* attach类型错误，丢弃 */
            NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_RcvMsccAttachReq_PreProc: NORMAL: Attach Type is invalid.");
            return VOS_TRUE;
    }

#if (FEATURE_ON == FEATURE_LTE)


    /* 判断是否需要enable LTE */
    if (VOS_TRUE == ulNeedEnableLte)
    {
        ulDisableLteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisableLteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }

#endif

    return ulResult;

}
VOS_UINT8 NAS_MMC_IsNeedSndRfAvailInd()
{
    VOS_UINT32                          ulState;
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

    ulState = NAS_MMC_GetFsmTopState();
    enFsmId = NAS_MMC_GetCurrFsmId();

    if (NAS_MMC_FSM_L1_MAIN == enFsmId)
    {
        if (NAS_MMC_L1_STA_DEACT == ulState)
        {
            return VOS_FALSE;
        }
    }
    else if (NAS_MMC_FSM_POWER_OFF == enFsmId)
    {
        return VOS_FALSE;
    }
    else
    {}

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvMsccDetachReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MSCC_MMC_DETACH_REQ_STRU           *pstTafDetachReq = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

#if   (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulLteOnlyFlg;
#endif

    MMC_MM_DETACH_REASON_ENUM_UINT32    enDetachReason;

    pstTafDetachReq = (MSCC_MMC_DETACH_REQ_STRU *)pstMsg;

    /* 检查输入参数的合法性 */
    if ( ( pstTafDetachReq->enDetachType > NAS_MSCC_PIF_DETACH_TYPE_GPRS_IMSI )
      || ( pstTafDetachReq->enDetachType == NAS_MSCC_PIF_DETACH_NULL ))
    {
        NAS_WARNING_LOG1(WUEPS_PID_MMC, "NAS_MMC_RcvMsccDetachReq_PreProc:invalid parm", pstTafDetachReq->enDetachType);
        return VOS_TRUE;
    }

    NAS_MMC_RecordOosEventForDetachReason(pstTafDetachReq->enDetachReason);



    if (VOS_TRUE == NAS_MMC_IsNeedSndRfAvailInd())
    {
        /* 在NO RF丢网后，如果用户触SYSCFG/DETACH动作,则通知MSCC清除NO RF STATUS */
        if (VOS_FALSE == NAS_MML_GetRfAvailFlg())
        {
            NAS_MML_SetCurrRatRfAvailFlg(NAS_MML_NET_RAT_TYPE_BUTT, VOS_TRUE);
            NAS_MMC_SndMsccRfAvailInd(VOS_TRUE);
        }
    }

    /* 进行消息优先级比较,判断当前是否需要打断当前状态机或进行消息的缓存 */
    ulRet = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    /* 需要打断当前状态机或进行消息的缓存,则表示预处理完成 */
    if ( VOS_TRUE == ulRet )
    {
        /* 不向LMM或MM/GMM发送detach 请求,待处理缓存消息时
           再发送detach 请求 */
        return VOS_TRUE;
    }

    /* 根据当前的接入模式发送detach请求,该消息处理完成不再进入状态
       机处理 */
#if   (FEATURE_ON == FEATURE_LTE)
    ulLteOnlyFlg = NAS_MML_IsLteOnlyMode(NAS_MML_GetMsPrioRatList());

    if (( NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
     || (VOS_TRUE                  == ulLteOnlyFlg))
    {
        /* 向LMM发送DETACH请求 */
        NAS_MMC_SndLmmDetachReq(pstTafDetachReq->ulOpID,
                                (VOS_UINT8)pstTafDetachReq->enDetachType,
                                pstTafDetachReq->enDetachReason);
    }
    else
#endif
    {
        if ((NAS_MSCC_PIF_DETACH_REASON_3GPP2_ATTACHED == pstTafDetachReq->enDetachReason)
         || (NAS_MSCC_PIF_DETACH_REASON_BUTT       == pstTafDetachReq->enDetachReason))
        {
            enDetachReason = MMC_MM_PIF_DETACH_REASON_BUTT;
        }
        else
        {
            enDetachReason = (MMC_MM_DETACH_REASON_ENUM_UINT32)pstTafDetachReq->enDetachReason;
        }

        /* 向MM发送DETACH请求 */
        NAS_MMC_SndMmDetachReq(pstTafDetachReq->ulOpID,
                               (MMC_MM_DETACH_TYPE_ENUM_UINT32)pstTafDetachReq->enDetachType,
                               enDetachReason);

        /* 向GMM发送DETACH请求 */
        NAS_MMC_SndGmmDetachReq(pstTafDetachReq->ulOpID,
                               (MMC_GMM_DETACH_TYPE_ENUM_UINT32)pstTafDetachReq->enDetachType,
                               (MMC_GMM_DETACH_REASON_ENUM_UINT32)enDetachReason);
    }

    /* 保存detach请求上下文 */
    NAS_MMC_GetDetachReqCtxAddr()->enDetachReason = (NAS_MMC_DETACH_REASON_ENUM_UINT32)pstTafDetachReq->enDetachReason;
    NAS_MMC_GetDetachReqCtxAddr()->enDetachType   = (NAS_MMC_DETACH_TYPE_ENUM_UINT32)pstTafDetachReq->enDetachType;


    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvMmAttachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCMM_ATTACH_CNF_STRU              *pstMmAttachCnf;


    pstMmAttachCnf = (MMCMM_ATTACH_CNF_STRU*)pstMsg;



    /* 封装消息，向MSCC发送ATTACH结果 */
    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            (NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8)pstMmAttachCnf->ulServiceStatus,
                            pstMmAttachCnf->ulOpid);


#if   (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == NAS_MMC_IsNeedDisableLte_AttachCs())
    {
        /* 当前网络为LTE，则暂时不发送disable LTE消息 */
        if (NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
        {
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);
        }
        else
        {
            /* 如果漫游导致的disable已经通知了GU接入层,此时不需要再重复通知 */
            if (VOS_FALSE == NAS_MML_GetDisableLteRoamFlg())
            {
                NAS_MMC_SndDisableLteNotify(MMC_LMM_DISABLE_LTE_REASON_PS_USIM_INVALID);

                /* 记录disable lte时驻留的网络信息 */
                NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);
        }

        if (VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
        {
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_DETACH);
        }
        else
        {
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_USIM_INVALID);
        }

    }
#endif

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvGmmAttachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCGMM_ATTACH_CNF_STRU             *pstGmmAttachCnf;


    pstGmmAttachCnf = (MMCGMM_ATTACH_CNF_STRU*)pstMsg;



    /* 封装消息，向MSCC发送ATTACH结果 */
    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            (NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8)pstGmmAttachCnf->ulServiceStatus,
                            pstGmmAttachCnf->ulOpid);


    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvMmDetachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCMM_DETACH_CNF_STRU              *pstMmDetachCnf = VOS_NULL_PTR;
#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulDisableLteRoamFlg;
#endif
    NAS_MMC_SERVICE_ENUM_UINT8          enCsServiceStatus;


    VOS_UINT8                           ucSimCsRegStatus;

    pstMmDetachCnf = (MMCMM_DETACH_CNF_STRU*)pstMsg;



    /* 正常服务且CS允许attach标记为FALSE则修改为限制服务上报-否则不报 */
    enCsServiceStatus = NAS_MMC_GetCsServiceStatus();
    if( (VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
     && (NAS_MMC_NORMAL_SERVICE == enCsServiceStatus) )
    {
        NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MMC_LIMITED_SERVICE);
    }


    /* 向MSCC发送DETACH结果 */
    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            (NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8)pstMmDetachCnf->ulServiceStatus,
                            pstMmDetachCnf->ulOpid);


    /* 如果CS不允许注册而且CS卡有效，上报cs注册状态为NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH*/
    ucSimCsRegStatus = NAS_MML_GetSimCsRegStatus();
    if ((VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
     && (VOS_TRUE == ucSimCsRegStatus))
    {
        NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
    }

#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
    {
        /* GU下MO detach 成功需要通知LMM */
        NAS_MMC_SndLmmMoDetachIndActionResultReq(MMC_LMM_MO_DET_CS_ONLY);
    }

    if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_DetachCs())
    {
        ulDisableLteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisableLteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }
#endif

    /* Added by zwx247453 for CHR optimize ,2015-3-13 begin */
#if (FEATURE_ON == FEATURE_PTM)
    NAS_MMC_MoDetachIndRecord(pstMmDetachCnf->ulDetachType);
#endif
    /* Added by zwx247453 for CHR optimize ,2015-3-13 end */

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_IsNeedStartAvailableTimer_PsDetachCnf(VOS_VOID)
{
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;
    VOS_UINT8                           ucIsSvlteSupportFlag;
    VOS_UINT8                           ucCsAttachAllowFlg;
    VOS_UINT8                           ucPsAttachAllowFlg;
    VOS_UINT32                          ulIsUsimStatusValid;
    NAS_MMC_TIMER_STATUS_ENUM_U8        enAvailableTimerStatus;

    enFsmId                = NAS_MMC_GetCurrFsmId();
    ucIsSvlteSupportFlag   = NAS_MML_GetSvlteSupportFlag();
    ucCsAttachAllowFlg     = NAS_MML_GetCsAttachAllowFlg();
    ucPsAttachAllowFlg     = NAS_MML_GetPsAttachAllowFlg();
    ulIsUsimStatusValid    = NAS_MML_IsUsimStausValid();
    enAvailableTimerStatus = NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER);

    /* ps迁移到modem0后，如果l或tds 15s未注册成功，应用保护定时器超时会将ps再迁回modem1，
       modem0 detach ps后如果后续l或tds没有报丢网或系统消息，会导致available定时器未启动ps迁移不回modem0
       mmc 收到detach cnf判断如果svlte 开启、cs ps不允许注册，卡有效，不在搜网状态机且available定时器未在运行
       则启动available定时器 */
    if ((VOS_TRUE == ucIsSvlteSupportFlag)
     && (VOS_FALSE == ucCsAttachAllowFlg)
     && (VOS_FALSE == ucPsAttachAllowFlg)
     && (VOS_TRUE == ulIsUsimStatusValid)
     && (NAS_MMC_FSM_PLMN_SELECTION != enFsmId)
     && (NAS_MMC_FSM_ANYCELL_SEARCH != enFsmId)
     && (NAS_MMC_TIMER_STATUS_RUNING != enAvailableTimerStatus))
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}



VOS_UINT32  NAS_MMC_RcvGmmDetachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCGMM_DETACH_CNF_STRU             *pstGmmDetachCnf;

    NAS_MMC_SERVICE_ENUM_UINT8          enPsServiceStatus;

    VOS_UINT8                           ucSimPsRegStatus;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulDisableLteRoamFlg;

    ulDisableLteRoamFlg = NAS_MML_GetDisableLteRoamFlg();
#endif

    pstGmmDetachCnf = (MMCGMM_DETACH_CNF_STRU*)pstMsg;

    /* 更新PS服务状态 */

    /* 正常服务且pS允许attach标记为FALSE则修改为限制服务上报-否则不报 */
    enPsServiceStatus = NAS_MMC_GetPsServiceStatus();
    if( (VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
     && (NAS_MMC_NORMAL_SERVICE == enPsServiceStatus) )
    {
        NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MMC_LIMITED_SERVICE);
    }


    /* 向MSCC发送DETACH结果 */
    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            (NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8)pstGmmDetachCnf->ulServiceSts,
                            pstGmmDetachCnf->ulOpid);


    /* 如果PS不允许注册而且PS卡有效，上报ps注册状态为NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH*/
    ucSimPsRegStatus = NAS_MML_GetSimPsRegStatus();
    if ((VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
     && (VOS_TRUE == ucSimPsRegStatus))
    {
        NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
    }

#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
    {
        /* GU下MO detach 需要通知LMM */
        NAS_MMC_SndLmmMoDetachIndActionResultReq((MMC_LMM_MO_DETACH_TYPE_ENUM_UINT32)(pstGmmDetachCnf->ulDetachType));
    }

    /* 判断是否需要disable LTE */
    if ( VOS_TRUE == NAS_MMC_IsNeedDisableLte_DetachPs())
    {
        /* 如果漫游导致的disable已经通知了GU接入层,此时不需要再重复通知 */
        if (VOS_FALSE == ulDisableLteRoamFlg)
        {
            /* 向WAS/GAS发送disable LTE通知消息 */
            NAS_MMC_SndAsLteCapabilityStatus(WUEPS_PID_WRR, RRC_NAS_LTE_CAPABILITY_STATUS_DISABLE);
            NAS_MMC_SndAsLteCapabilityStatus(UEPS_PID_GAS, RRC_NAS_LTE_CAPABILITY_STATUS_DISABLE);

            /* 向LMM发送disable LTE消息 */
            NAS_MMC_SndLmmDisableLteNotify(MMC_LMM_DISABLE_LTE_REASON_PS_DETACH);

            /* 记录disable lte时驻留的网络信息 */
            NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);

        NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_DETACH);

        /* 清除detach 请求上下文信息 */
        NAS_MMC_InitDetachReqCtx(NAS_MMC_GetDetachReqCtxAddr());
    }

    /* 收到GMM的Detach Cnf后，如果已经Detach CS,需要Enable Lte */
    if ( VOS_TRUE == NAS_MMC_IsNeedEnableLte_DetachPs())
    {
        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisableLteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }
#endif

    if (VOS_TRUE == NAS_MMC_IsNeedStartAvailableTimer_PsDetachCnf())
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, NAS_MMC_GetNextAvailableTimerValue());
    }

    /* Added by zwx247453 for CHR optimize ,2015-3-13 begin */
#if (FEATURE_ON == FEATURE_PTM)
    NAS_MMC_MoDetachIndRecord(pstGmmDetachCnf->ulDetachType);
#endif
    /* Added by zwx247453 for CHR optimize ,2015-3-13 end */

    return VOS_TRUE;
}


VOS_UINT32 NAS_MMC_RcvGmmNetworkCapabilityInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    GMMMMC_NETWORK_CAPABILITY_INFO_IND_STRU    *pstRcvMsg = VOS_NULL_PTR;
    VOS_UINT32                                              ulState;

    ulState   = NAS_MMC_GetFsmTopState();

    /* BG搜状态机系统消息可能延迟发送，需要在注册状态和服务状态之前通知MMA */
    if ((NAS_MMC_FSM_BG_PLMN_SEARCH                         == NAS_MMC_GetCurrFsmId())
     && ((NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_REG_IND       == ulState)
      || (NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_CONN_REL_IND  == ulState)))
    {
        NAS_MMC_SndMsccSysInfo_BgPlmnSearch();
    }

    pstRcvMsg = (GMMMMC_NETWORK_CAPABILITY_INFO_IND_STRU *)pstMsg;

    NAS_MMC_SndMsccNetworkCapabilityInfoInd((NAS_MSCC_PIF_NW_IMS_VOICE_CAP_ENUM_UINT8)pstRcvMsg->enNwImsVoCap,
                                           (NAS_MSCC_PIF_NW_EMC_BS_CAP_ENUM_UINT8)pstRcvMsg->enNwEmcBsCap,
                                           (NAS_MSCC_PIF_LTE_CS_CAPBILITY_ENUM_UINT8)pstRcvMsg->enLteCsCap);

    return VOS_TRUE;
}



#if   (FEATURE_ON == FEATURE_LTE)
VOS_UINT32  NAS_MMC_RcvGmmTinInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_TIN_TYPE_ENUM_UINT8         enGmmTinType;
    MMCGMM_TIN_TYPE_IND_STRU           *pstTinTypeMsg;
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;

    pstTinTypeMsg   = (MMCGMM_TIN_TYPE_IND_STRU*)pstMsg;

    enGmmTinType    = pstTinTypeMsg->enTinType;


    pstRplmnCfgInfo = NAS_MML_GetRplmnCfg();

    if ( enGmmTinType != pstRplmnCfgInfo->enTinType)
    {
        NAS_MMC_WriteTinInfoNvim(enGmmTinType, pstRplmnCfgInfo->aucLastImsi);
    }

    return VOS_TRUE;
}




VOS_UINT32  NAS_MMC_RcvLmmTinInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_TIN_TYPE_IND_STRU          *pstTinTypeMsg;
    NAS_MML_TIN_TYPE_ENUM_UINT8         enTinType;
    NAS_MML_RPLMN_CFG_INFO_STRU        *pstRplmnCfgInfo = VOS_NULL_PTR;

    pstTinTypeMsg  = (LMM_MMC_TIN_TYPE_IND_STRU*)pstMsg;

    /*消息中TIN为GUTI时*/
    if ( MMC_LMM_TIN_GUTI == pstTinTypeMsg->ulTinType )
    {
        /*本地全局变量中TIN覆盖为GUTI*/
        enTinType = NAS_MML_TIN_TYPE_GUTI;
    }
    else if ( MMC_LMM_TIN_RAT_RELATED_TMSI == pstTinTypeMsg->ulTinType )
    {
        /*消息中TIN为RAT-RELATED-TMSI时*/
        /*本地全局变量中TIN覆盖为RAT-RELATED-TMSI*/
        enTinType = NAS_MML_TIN_TYPE_RAT_RELATED_TMSI;
    }
    else if ( MMC_LMM_TIN_P_TMSI == pstTinTypeMsg->ulTinType )
    {
        /*消息中TIN为PTMSI时*/
        /*本地全局变量中TIN覆盖为PTMSI*/
        enTinType = NAS_MML_TIN_TYPE_PTMSI;
    }
    else
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "Invalid Tin Type");
        return VOS_TRUE;
    }

    pstRplmnCfgInfo     = NAS_MML_GetRplmnCfg();
    if ( enTinType != pstRplmnCfgInfo->enTinType )
    {
        NAS_MMC_WriteTinInfoNvim(enTinType, pstRplmnCfgInfo->aucLastImsi);
    }

    return VOS_TRUE;
}

VOS_UINT32  NAS_MMC_RcvLmmDetachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf     = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;

    VOS_UINT8                           ucSimCsRegStatus;

    ulRet           = VOS_TRUE;

    pstLmmDetachCnf = (LMM_MMC_DETACH_CNF_STRU*)pstMsg;

    /* 封装消息，向MSCC发送DETACH结果 */
    switch (pstLmmDetachCnf->ulReqType)
    {
        case MMC_LMM_MO_DET_PS_ONLY:

            ulRet = NAS_MMC_ProcEpsOnlyDetachCnf_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_MO_DET_CS_ONLY:

            ulRet = NAS_MMC_ProcCsOnlyDetachCnf_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_MO_DET_CS_PS:

            ulRet = NAS_MMC_ProcCsPsDetachCnf_PreProc(pstLmmDetachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvLmmDetachCnf_PreProc:Unexpected detach request type!");
            break;
    }


    if (VOS_TRUE == NAS_MMC_IsNeedSndEplmn_LmmDetachCnf(pstLmmDetachCnf))
    {
        NAS_MMC_ProcEmcPdpRelease_DetachPs();
    }


    /* 如果CS不允许注册而且CS卡有效，上报cs注册状态为NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH*/
    ucSimCsRegStatus = NAS_MML_GetSimCsRegStatus();
    if ((VOS_FALSE == NAS_MML_GetCsAttachAllowFlg())
     && (VOS_TRUE == ucSimCsRegStatus))
    {
        NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
    }


    if (VOS_TRUE == NAS_MMC_IsNeedStartAvailableTimer_PsDetachCnf())
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, NAS_MMC_GetNextAvailableTimerValue());
    }

    /* Added by zwx247453 for CHR optimize ,2015-3-13 begin */
#if (FEATURE_ON == FEATURE_PTM)
    NAS_MMC_MoDetachIndRecord(pstLmmDetachCnf->ulReqType);
#endif
    /* Added by zwx247453 for CHR optimize ,2015-3-13 end */

    return ulRet;
}
/*****************************************************************************
 函 数 名  : NAS_MMC_ProcAttachCnfSucc_PreProc
 功能描述  : 收到LMM的eps attach成功的预处理
 输入参数  : pstLmmAttachCnf消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月11日
    作    者   : l00305157
    修改内容   : Service_State_Optimize_PhaseII 项目修改
*****************************************************************************/
VOS_VOID  NAS_MMC_ProcAttachCnfSucc_PreProc(
    LMM_MMC_ATTACH_CNF_STRU            *pstLmmAttachCnf
)
{
    switch (pstLmmAttachCnf->ulReqType)
    {
        case MMC_LMM_ATT_REQ_TYPE_PS_ONLY:
            NAS_MMC_UpdatePsRegStatePsRegSucc();
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MMC_NORMAL_SERVICE);

            break;

        case MMC_LMM_ATT_REQ_TYPE_CS_ONLY:
             NAS_MMC_UpdateCsRegStateCsRegSucc();
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MMC_NORMAL_SERVICE);

            break;

        case MMC_LMM_ATT_REQ_TYPE_CS_PS:
            NAS_MMC_UpdateCsRegStateCsRegSucc();
            NAS_MMC_UpdatePsRegStatePsRegSucc();
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,NAS_MMC_NORMAL_SERVICE);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcAttachCnfSucc_PreProc:Unexpected detach request type!");

            break;
    }
}

/*****************************************************************************
 函 数 名  : NAS_MMC_ProcAttachCnfAuthRej_PreProc
 功能描述  : 收到LMM的attach 鉴权被拒的预处理
 输入参数  : pstLmmAttachCnf消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月11日
    作    者   : l00305157
    修改内容   : Service_State_Optimize_PhaseII 项目修改
*****************************************************************************/
VOS_VOID  NAS_MMC_ProcAttachCnfAuthRej_PreProc(
    LMM_MMC_ATTACH_CNF_STRU            *pstLmmAttachCnf
)
{
    NAS_MML_SetSimCsRegStatus(VOS_FALSE);
    NAS_MML_SetSimPsRegStatus(VOS_FALSE);

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_ATTACH_AUTH_REJ);

    NAS_MMC_ChangeCsRegState(NAS_MML_REG_REGISTERED_DENIED);
    NAS_MMC_ChangePsRegState(NAS_MML_REG_REGISTERED_DENIED);
    NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,NAS_MMC_LIMITED_SERVICE);
}
/*****************************************************************************
 函 数 名  : NAS_MMC_ProcEpsOnlyAttachCnf_PreProc
 功能描述  : 收到LMM的LMM_MMC_ATTACH_CNF回复的预处理
 输入参数  : pstLmmAttachCnf消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月11日
    作    者   : l00305157
    修改内容   : Service_State_Optimize_PhaseII 项目修改
*****************************************************************************/
VOS_VOID  NAS_MMC_ProcEpsOnlyAttachCnf_PreProc(
    LMM_MMC_ATTACH_CNF_STRU            *pstLmmAttachCnf
)
{

    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enPsMmaServiceStatus;

    /* attach结果处理 */
    switch (pstLmmAttachCnf->ulAttachRslt)
    {
        case MMC_LMM_ATT_RSLT_SUCCESS:

            NAS_MMC_ProcAttachCnfSucc_PreProc(pstLmmAttachCnf);

            break;

        case MMC_LMM_ATT_RSLT_ACCESS_BAR:
        case MMC_LMM_ATT_RSLT_FAILURE:

            /* 不做任何处理 */
            break;

        case MMC_LMM_ATT_RSLT_AUTH_REJ:

            NAS_MMC_ProcAttachCnfAuthRej_PreProc(pstLmmAttachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcEpsOnlyAttachCnf_PreProc:Unexpected detach result!");
            break;
    }

    enPsMmaServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);

    /* 获取当前PS服务状态 */
    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            enPsMmaServiceStatus,
                            pstLmmAttachCnf->ulOpId);
}
/*****************************************************************************
 函 数 名  : NAS_MMC_ProcCsOnlyAttachCnf_PreProc
 功能描述  : 收到LMM的attach成功的预处理
 输入参数  : pstLmmattachCnf消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月11日
    作    者   : l00305157
    修改内容   : Service_State_Optimize_PhaseII 项目修改
*****************************************************************************/
VOS_VOID  NAS_MMC_ProcCsOnlyAttachCnf_PreProc(
    LMM_MMC_ATTACH_CNF_STRU            *pstLmmAttachCnf
)
{
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enCsMmaServiceStatus;

    switch (pstLmmAttachCnf->ulAttachRslt)
    {
        case MMC_LMM_ATT_RSLT_SUCCESS:

        NAS_MMC_ProcAttachCnfSucc_PreProc(pstLmmAttachCnf);

            break;

        case MMC_LMM_ATT_RSLT_ACCESS_BAR:
        case MMC_LMM_ATT_RSLT_FAILURE:

            /* 不做任何处理 */
            break;

        case MMC_LMM_ATT_RSLT_AUTH_REJ:

        NAS_MMC_ProcAttachCnfAuthRej_PreProc(pstLmmAttachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcCsOnlyAttachCnf_PreProc:Unexpected detach result!");
            break;
    }

    enCsMmaServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);

    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            enCsMmaServiceStatus,
                            pstLmmAttachCnf->ulOpId);

    return ;
}
/*****************************************************************************
 函 数 名  : NAS_MMC_ProcCsPsAttachCnf_PreProc
 功能描述  : 收到LMM的Attach成功的预处理
 输入参数  : pstLmmattachCnf消息
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月11日
    作    者   : l00305157
    修改内容   : Service_State_Optimize_PhaseII 项目修改
*****************************************************************************/
VOS_VOID  NAS_MMC_ProcCsPsAttachCnf_PreProc(
    LMM_MMC_ATTACH_CNF_STRU            *pstLmmAttachCnf
)
{
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enCsMmaServiceStatus;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enPsMmaServiceStatus;

    switch (pstLmmAttachCnf->ulAttachRslt)
    {
        case MMC_LMM_ATT_RSLT_SUCCESS:

            NAS_MMC_ProcAttachCnfSucc_PreProc(pstLmmAttachCnf);

            break;

        case MMC_LMM_ATT_RSLT_ACCESS_BAR:
        case MMC_LMM_ATT_RSLT_FAILURE:

            /* 不做任何处理 */
            break;

        case MMC_LMM_ATT_RSLT_AUTH_REJ:

            NAS_MMC_ProcAttachCnfAuthRej_PreProc(pstLmmAttachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcCsPsAttachCnf_PreProc:Unexpected detach result!");
            break;
    }

    enCsMmaServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);
    enPsMmaServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);

    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            enCsMmaServiceStatus,
                            pstLmmAttachCnf->ulOpId);

    NAS_MMC_SndMsccAttachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            enPsMmaServiceStatus,
                            pstLmmAttachCnf->ulOpId);
}

VOS_UINT32  NAS_MMC_RcvLmmAttachCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    LMM_MMC_ATTACH_CNF_STRU             *pstLmmAttachCnf;
    NAS_MMC_FSM_ID_ENUM_UINT32           enFsmId;
    VOS_UINT32                           ulDisableLteRoamFlg;

    VOS_UINT32                           ulNeedEnableLte;
    VOS_UINT32                           ulNeedDisableLte;


    pstLmmAttachCnf = (LMM_MMC_ATTACH_CNF_STRU*)pstMsg;

    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

    /* 封装消息，向MSCC发送ATTACH结果 */

    ulNeedEnableLte  = VOS_FALSE;
    ulNeedDisableLte = VOS_FALSE;


    switch (pstLmmAttachCnf->ulReqType)
    {
        case MMC_LMM_ATT_REQ_TYPE_PS_ONLY:
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin*/
            NAS_MMC_ProcEpsOnlyAttachCnf_PreProc(pstLmmAttachCnf);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end*/

            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_AttachPs();

            break;

        case MMC_LMM_ATT_REQ_TYPE_CS_PS:
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin*/
            NAS_MMC_ProcCsPsAttachCnf_PreProc(pstLmmAttachCnf);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end*/

            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_AttachCsPs();
            ulNeedDisableLte = NAS_MMC_IsNeedDisableLte_AttachCsPs();

            break;

        case MMC_LMM_ATT_REQ_TYPE_CS_ONLY:
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin*/
            NAS_MMC_ProcCsOnlyAttachCnf_PreProc(pstLmmAttachCnf);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end*/

            ulNeedDisableLte = NAS_MMC_IsNeedDisableLte_AttachCs();
            break;

        default:
            /* 异常打印 */
            break;
    }


    /* 判断是否需要enable LTE */
    if (VOS_TRUE == ulNeedEnableLte)
    {
        ulDisableLteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisableLteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
        return VOS_TRUE;
    }

    enFsmId = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    /* 判断是否需要disable LTE */
    if (VOS_TRUE == ulNeedDisableLte)
    {
        /* 记录本地disable LTE的标记,后续L变为从模再通知LMM,WAS,GAS */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);

        if (VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
        {
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_DETACH);
        }
        else
        {
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_USIM_INVALID);
        }

        /* ON PLMN下直接可以处理 */
        if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
        && (NAS_MMC_FSM_L1_MAIN == enFsmId))
        {
           /* 如果当前EPS信令链接存在，则缓存构造的搜网消息 */
           if ( VOS_TRUE == NAS_MML_IsRrcConnExist())
           {
               NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE, NAS_MMC_PLMN_SEARCH_SCENE_DISABLE_LTE);

               return VOS_TRUE;
           }
        }

        /* 进入状态机进行处理 */
        return VOS_FALSE;
    }



    /* L->LWG后，会先DETACH CS/PS，MSCC在收到SYSCFG CNF时候，再下发ATTACH CS/PS，ATTACH失败时需要尝试搜网 */
    if ( VOS_TRUE == NAS_MMC_IsLmmAttachCnfTriggerPlmnSrch(pstLmmAttachCnf) )
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
/* 删除将LMM格式的EPS注册状态，转化为MML的注册状态的接口
   NAS_MMC_CovertLmmRegStatusToMmlRegStatus() */

/* 删除LTE上报服务状态和注册状态的接口   NAS_MMC_RcvLmmRegStatusChangeInd_PreProc()处理函数 */

/* 删除将LMM上报的服务状态转换为MMC的服务状态的接口
   NAS_MMC_ConvertLmmServiceStatusToMmcServiceStatus() */

/* 删除将LMM上报的服务状态转换为MMC上报用的服务状态的接口
   NAS_MMC_ConvertLmmServiceStatusToMmcReportSrvSt() */

/* 删除LTE上报服务状态和注册状态的接口*/
/* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11,end */




VOS_UINT32  NAS_MMC_RcvLmmErrInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 调用OAM的接口直接进行复位  */
    NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_RECEIVE_LMM_ERR_IND);

    return VOS_TRUE;
}

VOS_UINT32  NAS_MMC_RcvLmmSuspendCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_SUSPEND_CNF_STRU           *pstLmmSuspendCnf = VOS_NULL_PTR;
    VOS_UINT8                           ucEmcFlg;
    VOS_UINT8                           ucImsVoiceAvail;

    /* L模时，做如下格式转换 */
    pstLmmSuspendCnf = (LMM_MMC_SUSPEND_CNF_STRU*)pstMsg;

    /* 挂起结果不为成功，进入状态机处理 */
    if ( MMC_LMM_SUCC != pstLmmSuspendCnf->ulRst)
    {
        return VOS_FALSE;
    }

    /* 判断是否需要disable LTE */
    if (NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS == NAS_MML_GetLteCapabilityStatus())
    {
        /* 如果漫游导致的disable已经通知了GU接入层,此时不需要再重复通知 */
        if (VOS_FALSE == NAS_MML_GetDisableLteRoamFlg())
        {
            NAS_MMC_SndDisableLteNotify(NAS_MML_GetDisableLteReason());

            /* 记录disable lte时驻留的网络信息 */
            NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());

            if (MMC_LMM_DISABLE_LTE_REASON_LTE_ROAMING_NOT_ALLOWED == NAS_MML_GetDisableLteReason())
            {
                NAS_MML_SetDisableLteRoamFlg(VOS_TRUE);
            }
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);

        NAS_MML_SetDisableLteReason(NAS_MML_GetDisableLteReason());
    }
    else
    {
        /* 在紧急呼叫不能通过CSFB到GU时,根据rel-11 24.301 4.3.1需要disable LTE.目前定义了一个disable时长为5MINS.
           5mins后重新enable LTE */
        /* 3GPP 24.301 Selection 4.3.1:
           if the UE needs to initiate a CS fallback emergency call but it is unable to perform CS fallback,
           the UE shall attempt to select GERAN or UTRAN radio access technology,
           and a UE with "IMS voice not available" should disable the E-UTRA capability (see subclause 4.5) to allow a potential callback,
           and then progress the CS emergency call establishment;

           NOTE 2: Unable to perform CS fallback or 1xCS fallback means that either the UE was not allowed to attempt CS fallback or 1xCS fallback,
           or CS fallback or 1xCS fallback attempt failed.
        */
        /* Disable LTE时需要判断IMS是否可用，IMS可用时不Disable LTE */
        ucEmcFlg        = NAS_MML_GetCsEmergencyServiceFlg();
        ucImsVoiceAvail = NAS_MML_GetImsVoiceAvailFlg();
        if (((NAS_MML_CSFB_SERVICE_STATUS_MO_EMERGENCY_EXIST == NAS_MML_GetCsfbServiceStatus())
          || (VOS_TRUE == ucEmcFlg))
         && (VOS_FALSE  == ucImsVoiceAvail))

        {
            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);

            NAS_MMC_SndDisableLteNotify(MMC_LMM_DISABLE_LTE_REASON_EMERGENCY_CALL);

            /* 记录disable lte时驻留的网络信息 */
            NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());

            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_EMERGENCY_CALL);

            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER, NAS_MML_GetCsfbEmgCallEnableLteTimerLen());
        }
    }

    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvLmmMmcDetachInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_DETACH_IND_STRU                                *pstDetachMsg = VOS_NULL_PTR;
    NAS_MSCC_PIF_NETWORK_DETACH_TYPE_ENUM_UINT8                  enDetachType;
    NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16                      enCause;

    pstDetachMsg        = (LMM_MMC_DETACH_IND_STRU*)pstMsg;

    enDetachType        = NAS_MSCC_PIF_NETWORK_DETACH_TYPE_BUTT;
    enCause             = NAS_MML_REG_FAIL_CAUSE_BUTT;

    if ( VOS_FALSE == pstDetachMsg->bitOpCnCause )
    {
        enCause = NAS_MML_REG_FAIL_CAUSE_OTHER_CAUSE;
    }
    else
    {
        NAS_MMC_ConvertLmmCauseToMmCause((VOS_UINT8)pstDetachMsg->ucCnCause,
                                                &enCause);
    }

    /* UE本地DETACH，如：去激活所有承载，通过DETACH类型中MMC_LMM_DETACH_LOCAL
    通知MMC，不携带其他值 */
    if (MMC_LMM_DETACH_LOCAL == pstDetachMsg->ulDetachType)
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_LTE_LOCAL_DETACH);

        /* Modified by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
        if (NAS_MMC_NO_SERVICE != NAS_MMC_GetPsServiceStatus())
        {
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS,NAS_MMC_LIMITED_SERVICE);
        }
        else
        {
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS,NAS_MMC_NO_SERVICE);
        }

        if (NAS_MMC_NO_SERVICE != NAS_MMC_GetCsServiceStatus())
        {
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS,NAS_MMC_LIMITED_SERVICE);
        }
        else
        {
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS,NAS_MMC_NO_SERVICE);
        }

        NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
        NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
        /* Modified by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */

        /* 紧急PDN连接存在时:清除连接存在标志，给LMM/GUAS发送EPLMN通知，删除Forb Plmn */
        NAS_MMC_ProcEmcPdpRelease_DetachPs();

        /* 向GMM和MM转发注册结果消息 */
        NAS_MMC_SndMmLmmLocalDetachInd(pstDetachMsg);
        NAS_MMC_SndGmmLmmLocalDetachInd(pstDetachMsg);

        return VOS_TRUE;
    }

    if ( VOS_TRUE == pstDetachMsg->bitOpCnReqType )
    {
        NAS_MMC_ConvertLmmDetachIndTypeToMmcType(pstDetachMsg->ulCnReqType, &enDetachType);

        NAS_MMC_SndMsccDetachInd(NAS_MSCC_PIF_SRVDOMAIN_PS, (VOS_UINT32)enDetachType, (VOS_UINT32)enCause);
    }

    return VOS_FALSE;
}



VOS_UINT32  NAS_MMC_RcvLmmMmcStatusInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_STATUS_IND_STRU            *pstLmmStatusIndMsg = VOS_NULL_PTR;


    pstLmmStatusIndMsg = (LMM_MMC_STATUS_IND_STRU*)pstMsg;

    if ( VOS_FALSE == pstLmmStatusIndMsg->bitOpConnState)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvLmmMmcStatusInd_PreProc:bitOpConnState IS VOS_FALSE");
        return VOS_TRUE;
    }

    NAS_MML_UpdateEpsConnectionStatus(pstLmmStatusIndMsg->ulConnState);


    NAS_MMC_SndGmmLmmStatusInd(pstLmmStatusIndMsg);


    if ( MMC_LMM_CONNECTED_DATA == pstLmmStatusIndMsg->ulConnState )
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_LTE_DATA);

        NAS_MMC_SndMsccPsServiceConnStatusInd(VOS_TRUE, MSCC_MMC_PS_SIGNALING_TYPE_LTE);
    }
    else
    {
        NAS_MMC_SndMsccDataTranAttri(NAS_MSCC_PIF_DATATRAN_ATTRI_LTE_IDLE);
        NAS_MMC_SndMsccPsServiceConnStatusInd(VOS_FALSE, MSCC_MMC_PS_SIGNALING_TYPE_LTE);
    }

    /* 返回未处理完成 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvLmmAreaLostInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_AREA_LOST_IND_STRU         *pstLmmAreaLostMsg   = VOS_NULL_PTR;

    pstLmmAreaLostMsg   = (LMM_MMC_AREA_LOST_IND_STRU *)pstMsg;

    if (MMC_LMM_AREA_LOST_REASON_NO_RF == pstLmmAreaLostMsg->enAreaLostReason)
    {
        /* 处理area lost no rf */
        NAS_MMC_ProcNoRfAreaLost_PreProc(NAS_MML_NET_RAT_TYPE_LTE);

        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_LMM_NO_RF_AREA_LOST);

        return VOS_FALSE;
    }

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_LMM_NORMAL_AREA_LOST);

    return VOS_FALSE;
}
VOS_VOID NAS_MMC_ConvertLteSrvDoaminToMmcSrvDomain(
    LMM_MMC_SRV_DOMAIN_ENUM_UINT8       enLteSimSrvDomain,
    NAS_MSCC_PIF_SRVDOMAIN_ENUM_UINT32           *penSrvDomain
)
{
    switch (enLteSimSrvDomain)
    {
        case LMM_MMC_SRV_DOMAIN_CS:
           *penSrvDomain = NAS_MSCC_PIF_SRVDOMAIN_CS;
            break;

        case LMM_MMC_SRV_DOMAIN_PS:
           *penSrvDomain = NAS_MSCC_PIF_SRVDOMAIN_PS;
            break;

        case LMM_MMC_SRV_DOMAIN_CS_PS:
           *penSrvDomain = NAS_MSCC_PIF_SRVDOMAIN_CS_PS;
            break;

        default:
           *penSrvDomain = NAS_MSCC_PIF_SRVDOMAIN_CS_PS;
            break;
    }

    return;
}



VOS_VOID NAS_MMC_ConvertLteSimAuthFailToMmcSimAuthFail(
    LMM_MMC_SIM_AUTH_FAIL_ENUM_UINT16   enLteSimAuthFailRejCause,
    NAS_MML_SIM_AUTH_FAIL_ENUM_UINT16  *penMmcSimAuthFailRejCause
)
{
    switch (enLteSimAuthFailRejCause)
    {
        case LMM_MMC_SIM_AUTH_FAIL_NULL:
           *penMmcSimAuthFailRejCause = NAS_MML_SIM_AUTH_FAIL_NULL;
            break;

        case LMM_MMC_SIM_AUTH_FAIL_MAC_FAILURE:
           *penMmcSimAuthFailRejCause = NAS_MML_SIM_AUTH_FAIL_MAC_FAILURE;
            break;

        case LMM_MMC_SIM_AUTH_FAIL_SYNC_FAILURE:
           *penMmcSimAuthFailRejCause = NAS_MML_SIM_AUTH_FAIL_SYNC_FAILURE;
            break;

        case LMM_MMC_SIM_AUTH_FAIL_OTHER:
           *penMmcSimAuthFailRejCause = NAS_MML_SIM_AUTH_FAIL_LTE_OTHER_FAILURE;
            break;

        default:
           *penMmcSimAuthFailRejCause = NAS_MML_SIM_AUTH_FAIL_LTE_OTHER_FAILURE;
            break;
    }
}


VOS_UINT32  NAS_MMC_RcvLmmSimAuthFailInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_SIM_AUTH_FAIL_IND_STRU     *pstLmmSimAuthFailMsg   = VOS_NULL_PTR;
    NAS_MSCC_PIF_SRVDOMAIN_ENUM_UINT32            enSrvDomain;
    NAS_MML_SIM_AUTH_FAIL_ENUM_UINT16   enRejCause;

    pstLmmSimAuthFailMsg   = (LMM_MMC_SIM_AUTH_FAIL_IND_STRU *)pstMsg;

    NAS_MMC_ConvertLteSrvDoaminToMmcSrvDomain(pstLmmSimAuthFailMsg->enSrvDomain, &enSrvDomain);

    NAS_MMC_ConvertLteSimAuthFailToMmcSimAuthFail(pstLmmSimAuthFailMsg->enSimAuthFailValue, &enRejCause);

    NAS_MMC_SndMsccUsimAuthFailInd(enSrvDomain, enRejCause);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvLmmTimerStateNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_TIMER_STATE_NOTIFY_STRU    *pstTimerInfoNotify;

    pstTimerInfoNotify = (LMM_MMC_TIMER_STATE_NOTIFY_STRU *)pstMsg;

    if ((VOS_TRUE == pstTimerInfoNotify->bitOpT3412)
     && (LMM_MMC_TIMER_BUTT != pstTimerInfoNotify->enT3412State))
    {
        NAS_MML_SetT3412Status((VOS_UINT8)(pstTimerInfoNotify->enT3412State));
    }

    if ((VOS_TRUE == pstTimerInfoNotify->bitOpT3423)
     && (LMM_MMC_TIMER_BUTT != pstTimerInfoNotify->enT3423State))
    {
        NAS_MML_SetT3423Status((VOS_UINT8)(pstTimerInfoNotify->enT3423State));
    }

    if ((VOS_TRUE == pstTimerInfoNotify->bitOpT3402)
     && (LMM_MMC_TIMER_BUTT != pstTimerInfoNotify->enT3402State))
    {
        NAS_MML_SetT3402Status((VOS_UINT8)(pstTimerInfoNotify->enT3402State));

        return VOS_TRUE;
    }

    /* 向MM和GMM转发定时器状态消息 */
    NAS_MMC_SndGmmLmmTimerStateNotify(pstTimerInfoNotify);
    NAS_MMC_SndMmLmmTimerStateNotify(pstTimerInfoNotify);

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvLmmMmcNotCampOnInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MMC_SetAsCellCampOn(NAS_MMC_AS_CELL_NOT_CAMP_ON);
    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvLmmCellSignReportInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                      *pstMsg
)
{
    LMM_MMC_CELL_SIGN_REPORT_IND_STRU  *pStSignReportIndMsg = VOS_NULL_PTR;
    MSCC_MMC_LMM_CELL_SIGN_INFO_STRU     stLteCellSignInfo;
    MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN   unMeasReportType;

    PS_MEM_SET(((VOS_UINT8*)&unMeasReportType), 0, sizeof(MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN));
    unMeasReportType.ucMeasReportType |= NAS_MSCC_PIF_SCELL_MEAS_TYPE_RXLEV;

    PS_MEM_SET(((VOS_UINT8*)&stLteCellSignInfo), 0, sizeof(MSCC_MMC_LMM_CELL_SIGN_INFO_STRU));

    /*获取LTE 消息的信号上报参数*/
    pStSignReportIndMsg                 = (LMM_MMC_CELL_SIGN_REPORT_IND_STRU*)pstMsg;

    stLteCellSignInfo.sRsrp             = ((NAS_MMC_RSRP_UNVALID == pStSignReportIndMsg->sRsrp)?NAS_MMC_RSRP_UNVALID:pStSignReportIndMsg->sRsrp/NAS_MMC_SIGNAL_DIVISOR);
    stLteCellSignInfo.sRsrq             = ((NAS_MMC_RSRQ_UNVALID == pStSignReportIndMsg->sRsrq)?NAS_MMC_RSRQ_UNVALID:pStSignReportIndMsg->sRsrq/NAS_MMC_SIGNAL_DIVISOR);
    stLteCellSignInfo.sRssi             = ((NAS_MMC_RSSI_INVALID == pStSignReportIndMsg->sRssi)?NAS_MMC_RSSI_INVALID:pStSignReportIndMsg->sRssi/NAS_MMC_SIGNAL_DIVISOR);
    stLteCellSignInfo.sRsd              =  pStSignReportIndMsg->sRsd;

    stLteCellSignInfo.lSINR             = pStSignReportIndMsg->lSinr;
    stLteCellSignInfo.stCQI.usRI        = pStSignReportIndMsg->stCQI.usRI;
    stLteCellSignInfo.stCQI.ausCQI[0]   = pStSignReportIndMsg->stCQI.ausCQI[0];
    stLteCellSignInfo.stCQI.ausCQI[1]   = pStSignReportIndMsg->stCQI.ausCQI[1];


    /*上报信号强度给MMA发送LMM CELL SIGN INFO REPORT IND 消息*/
    NAS_MMC_SndMsccMsgLmmCellSignInfoInd(&unMeasReportType, &stLteCellSignInfo);

    return VOS_TRUE;
}

#endif


VOS_UINT32  NAS_MMC_RcvOmOtaReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    OM_NAS_OTA_REQ_STRUCT              *pstOtaReq;
    VOS_UINT32                          ulErrCode;

    pstOtaReq   = ( OM_NAS_OTA_REQ_STRUCT* )pstMsg;

    /*处理NAS OTA消息的开关*/
    NAS_INFO_LOG2(WUEPS_PID_MMC, "NAS_MMC_RcvOmOtaReq_PreProc: NORMAL: Specified NAS OTA switch is , Old switch is ",
                 (long)pstOtaReq->ulOnOff, NAS_GetNasOtaSwitch());

    /* 判断空口消息配置是否有效 */
    if ( (NAS_OTA_SWITCH_ON != pstOtaReq->ulOnOff )
      && (NAS_OTA_SWITCH_OFF != pstOtaReq->ulOnOff))
    {
        /* 空口消息配置错误 */
        ulErrCode = NAS_OTA_CNF_ERRCODE_YES;
        /* Modified by wx270776 for OM融合, 2015-7-23, begin */
        NAS_MMC_SndOmOtaCnf(ulErrCode, pstOtaReq);
        /* Modified by wx270776 for OM融合, 2015-7-23, end */
        return VOS_TRUE;
    }

    NAS_SetNasOtaSwitch(pstOtaReq->ulOnOff);
    ulErrCode = NAS_OTA_CNF_ERRCODE_NO;

    NAS_INFO_LOG2(WUEPS_PID_MMC, "NAS_MMC_RcvOmMsg_PreProc: NORMAL: Specified NAS OTA switch is , Old switch is ",
                 (long)pstOtaReq->ulOnOff, NAS_GetNasOtaSwitch());

    /* Modified by wx270776 for OM融合, 2015-7-23, begin */
    NAS_MMC_SndOmOtaCnf(ulErrCode, pstOtaReq);
    /* Modified by wx270776 for OM融合, 2015-7-23, end */

    return VOS_TRUE;
}


/*****************************************************************************
 函 数 名  : NAS_MMC_RcvOmInquireReq_PreProc
 功能描述  : 处理来自OM的查询消息
 输入参数  : ulEventType - 消息类型
             pstMsg      - 消息的首地址
 输出参数  : 无
 返 回 值  : VOS_TRUE:函数预处理完成
             VOS_FALSE:函数预处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
 1.日    期   : 2011年7月23日
   作    者   :
   修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvOmInquireReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    ID_NAS_OM_INQUIRE_STRU             *pstOmMsg = VOS_NULL_PTR;

    pstOmMsg = (ID_NAS_OM_INQUIRE_STRU *)pstMsg;

    NAS_INFO_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvOmInquireReq_PreProc:  ");

    NAS_MMC_SndOmInquireCnfMsg(pstOmMsg);

    return VOS_TRUE;

}




/* Modified by wx270776 for OM融合, 2015-7-23, begin */
VOS_VOID NAS_MMC_SndOmConfigTimerReportCnfMsg(
    NAS_OM_CONFIG_TIMER_REPORT_REQ_STRU    *pstTimerReportCfgReq
)
{
    NAS_OM_CONFIG_TIMER_REPORT_CNF_STRU    *pstNasSndOmTimerReportCnf = VOS_NULL_PTR;
    VOS_UINT32                              ulMsgLen;
    VOS_UINT32                              ulRet;

    /* 申请消息 */
    ulMsgLen     = sizeof(NAS_OM_CONFIG_TIMER_REPORT_CNF_STRU) - VOS_MSG_HEAD_LENGTH;

    pstNasSndOmTimerReportCnf = (NAS_OM_CONFIG_TIMER_REPORT_CNF_STRU*)PS_ALLOC_MSG(WUEPS_PID_MMC, ulMsgLen);
    if (VOS_NULL_PTR == pstNasSndOmTimerReportCnf)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_SndOmConfigTimerReportCnf: Message Alloc Failed!");

        return;
    }

    /* 初始化 */
    PS_MEM_SET((VOS_UINT8*)pstNasSndOmTimerReportCnf + VOS_MSG_HEAD_LENGTH, 0x00, ulMsgLen);

    /* 填写消息内容 */
    pstNasSndOmTimerReportCnf->ulReceiverPid    = MSP_PID_DIAG_APP_AGENT;
    pstNasSndOmTimerReportCnf->ulMsgId          = ID_NAS_OM_CONFIG_TIMER_REPORT_CNF;
    pstNasSndOmTimerReportCnf->usOriginalId     = pstTimerReportCfgReq->usOriginalId;
    pstNasSndOmTimerReportCnf->usTerminalId     = pstTimerReportCfgReq->usTerminalId;
    pstNasSndOmTimerReportCnf->ulTimeStamp      = pstTimerReportCfgReq->ulTimeStamp;
    pstNasSndOmTimerReportCnf->ulSN             = pstTimerReportCfgReq->ulSN;

    /* 发送消息 */
    ulRet = PS_SEND_MSG(WUEPS_PID_MMC, pstNasSndOmTimerReportCnf);
    if (VOS_OK != ulRet)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_SndOmConfigTimerReportCnf: WARNING: Send msg fail.");
    }

    return;
}
/* Modified by wx270776 for OM融合, 2015-7-23, end */


VOS_UINT32 NAS_MMC_RcvOmConfigTimerReportReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* Modified by wx270776 for OM融合, 2015-7-23, begin */
    NAS_OM_CONFIG_TIMER_REPORT_REQ_STRU *pstTimerReportCfgReq = VOS_NULL_PTR;
    NAS_TIMER_EVENT_INFO_STRU           *pstTimerEventCfg     = VOS_NULL_PTR;
    VOS_UINT32                           ulTimerEventNum;

    pstTimerEventCfg     = NAS_GetTimerEventReportCfg();

    pstTimerReportCfgReq = (NAS_OM_CONFIG_TIMER_REPORT_REQ_STRU*)pstMsg;
    /* Modified by wx270776 for OM融合, 2015-7-23, end */

    if ( NAS_MMC_REPORT_TIMER_OTA_EVENT_START == pstTimerReportCfgReq->ulCommand )
    {
        ulTimerEventNum = pstTimerReportCfgReq->stTimerMsg.ulItems;

        if(pstTimerReportCfgReq->stTimerMsg.ulItems > NAS_MAX_TIMER_EVENT)
        {
            ulTimerEventNum = NAS_MAX_TIMER_EVENT;
        }
        else
        {
            pstTimerEventCfg->ulItems = ulTimerEventNum;
        }

        PS_MEM_CPY(pstTimerEventCfg->aulTimerMsg, pstTimerReportCfgReq->stTimerMsg.aulTimerMsg, ulTimerEventNum*sizeof(VOS_UINT32));
    }
    else
    {
        pstTimerEventCfg->ulItems = 0;
    }

    /* Modified by wx270776 for OM融合, 2015-7-23, begin */
    NAS_INFO_LOG1(WUEPS_PID_MMC, "NAS_MMC_RcvOmConfigTimerReportReq_PreProc: Updated Timer Events.",
                  pstTimerEventCfg->ulItems);

    /* 回复工具cnf消息 */
    NAS_MMC_SndOmConfigTimerReportCnfMsg(pstTimerReportCfgReq);
    /* Modified by wx270776 for OM融合, 2015-7-23, end */

    return VOS_TRUE;

}
VOS_VOID NAS_MMC_IsNeedUpdateHighPrioPlmnSearchList(
    MSCC_MMC_UE_SUPPORT_FREQ_BAND_STRU   *pstSysCfgSetBand
)
{
    NAS_MMC_PLMN_SELECTION_LIST_INFO_STRU                  *pstPlmnSelectionList = VOS_NULL_PTR;
    NAS_MML_MS_BAND_INFO_STRU                              *pstMsBandInfo        = VOS_NULL_PTR;
    VOS_UINT32                                              i;

    pstMsBandInfo        = NAS_MML_GetMsSupportBand();
    pstPlmnSelectionList = NAS_MMC_GetHighPrioPlmnList();

    /* 如果G的频段发生改变，需要更新高优先级搜索列表中所有接入技术为G的网络状态为未搜索 */
    if (pstMsBandInfo->unGsmBand.ulBand != pstSysCfgSetBand->unGsmBand.ulBand)
    {
        for (i = 0; i < pstPlmnSelectionList->usSearchPlmnNum; i++)
        {
            NAS_MMC_UpdateRatNetStatusInPlmnRatInfo(NAS_MML_NET_RAT_TYPE_GSM,
                                                    NAS_MMC_NET_STATUS_NO_SEARCHED,
                                                    &pstPlmnSelectionList->astPlmnSelectionList[i]);
        }
    }

    /* 如果W的频段发生改变，需要更新高优先级搜索列表中所有接入技术为W的网络状态为未搜索 */
    if (pstMsBandInfo->unWcdmaBand.ulBand != pstSysCfgSetBand->unWcdmaBand.ulBand)
    {
        for (i = 0; i < pstPlmnSelectionList->usSearchPlmnNum; i++)
        {
            NAS_MMC_UpdateRatNetStatusInPlmnRatInfo(NAS_MML_NET_RAT_TYPE_WCDMA,
                                                   NAS_MMC_NET_STATUS_NO_SEARCHED,
                                                   &pstPlmnSelectionList->astPlmnSelectionList[i]);
        }
    }

#if (FEATURE_ON == FEATURE_LTE)

/* 如果L的频段发生改变，需要更新高优先级搜索列表中所有接入技术为L的网络状态为未搜索 */
    if ((pstMsBandInfo->stLteBand.aulLteBand[0] != pstSysCfgSetBand->stLteBand.aulLteBand[0])
     || (pstMsBandInfo->stLteBand.aulLteBand[1] != pstSysCfgSetBand->stLteBand.aulLteBand[1]))
    {
        for (i = 0; i < pstPlmnSelectionList->usSearchPlmnNum; i++)
        {
            NAS_MMC_UpdateRatNetStatusInPlmnRatInfo(NAS_MML_NET_RAT_TYPE_LTE,
                                                   NAS_MMC_NET_STATUS_NO_SEARCHED,
                                                   &pstPlmnSelectionList->astPlmnSelectionList[i]);
        }
    }
#endif

    return;
}



VOS_UINT32  NAS_MMC_RcvTafSysCfgReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_SYS_CFG_SET_REQ_STRU        *pstSysCfgSetParm = VOS_NULL_PTR;
    VOS_UINT32                          ulRet;
    VOS_UINT32                          ulSyscfgNeedPlmnSrchRslt;
    VOS_UINT32                          ulCsServiceExist;

    NAS_MMC_FSM_ID_ENUM_UINT32          enCurrFsmId;

    pstSysCfgSetParm         = (MSCC_MMC_SYS_CFG_SET_REQ_STRU*)pstMsg;

    /* 判断当前SYSCFG设置是否需要导致搜网 */
    ulSyscfgNeedPlmnSrchRslt = NAS_MMC_IsPlmnSelectionNeeded_SysCfg(pstSysCfgSetParm);

    /* 判断当前是否有CS业务存在 */
    ulCsServiceExist         = NAS_MML_GetCsServiceExistFlg();

    /* 如果SYSCFG导致需要搜网且当前有业务存在或者有缓存的业务存在
       那么不进行SYSCFG设置，向MSCC返回SYSCFG 设置失败*/


    /* GU下存在PS业务时，MMC给MM/GMM发送释放请求，连接释放后，可下发SYSCFG，和L的处理相同
       存在CS业务时，MMC向MSCC上报SYSCFG结果为有CS业务存在 */
    if (VOS_TRUE == ulSyscfgNeedPlmnSrchRslt)
    {
        if (VOS_TRUE == ulCsServiceExist)
        {
            NAS_MMC_SndMsccSysCfgCnf(NAS_MSCC_PIF_SYS_CFG_SET_FAILURE_CS_SERV_EXIST);
            return VOS_TRUE;
        }
        else
        {

        }
    }

    enCurrFsmId = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enCurrFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    /* 存在缓存的紧急呼叫且当前处于由于紧急呼叫触发的anycell状态机，syscfg设置无需搜网也要先处理紧急呼叫，syscfg设置失败  */
    if ((VOS_TRUE == NAS_MML_IsExistBufferedEmgCall())
     && (NAS_MMC_FSM_ANYCELL_SEARCH == enCurrFsmId))
    {
        NAS_MMC_SndMsccSysCfgCnf(NAS_MSCC_PIF_SYS_CFG_SET_FAILURE_OTHER);
        return VOS_TRUE;
    }

    /* 如果频段发生改变，需要更新高优先级搜网列表中网络状态 */
    NAS_MMC_IsNeedUpdateHighPrioPlmnSearchList((MSCC_MMC_UE_SUPPORT_FREQ_BAND_STRU *)&pstSysCfgSetParm->stBand);

    /* 判断当前是否需要打断当前状态机 或进行消息的缓存 */
    ulRet = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    /* 需要打断当前状态机或进行消息的缓存,则表示预处理完成 */
    if ( VOS_TRUE == ulRet )
    {
        return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvTafAcqReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 当前状态机标识 */
      NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

      MSCC_MMC_PLMN_ID_STRU                stPlmnId;

      enFsmId = NAS_MMC_GetCurrFsmId();

      /* 在异系统，ON Plmn状态尝试启动高优先级搜网定时器 */
    if ( (NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_OOS       == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_HO        == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_CCO       == enFsmId))
    {

       stPlmnId.ulMcc     = NAS_MML_INVALID_MCC;
       stPlmnId.ulMnc     = NAS_MML_INVALID_MNC;
       NAS_MMC_SndMsccAcqCnf(NAS_MSCC_PIF_ACQ_RESULT_FAILURE, &stPlmnId, 0);

       return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvTafRegReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* 当前状态机标识 */
      NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

      MSCC_MMC_PLMN_ID_STRU                stPlmnId;

      enFsmId = NAS_MMC_GetCurrFsmId();

      /* 在异系统，ON Plmn状态尝试启动高优先级搜网定时器 */
    if ( (NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_OOS       == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_HO        == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_CCO       == enFsmId))
    {

       stPlmnId.ulMcc     = NAS_MML_INVALID_MCC;
       stPlmnId.ulMnc     = NAS_MML_INVALID_MNC;
       NAS_MMC_SndMsccRegCnf(MMC_LMM_CL_REG_STATUS_BUTT, NAS_MSCC_PIF_REG_RESULT_ACQ_FAILURE, &stPlmnId, 0x0);

       return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}





VOS_UINT32  NAS_MMC_RcvMsccUserSpecPlmnSearch_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PLMN_SPECIAL_REQ_STRU      *pstUserSelReqMsg  = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU               *pstUserSpecPlmnId = VOS_NULL_PTR;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurrRatType;
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;
    VOS_UINT32                          ulCsServiceStatusFlg;
    VOS_UINT32                          ulRslt;

    NAS_MML_PLMN_WITH_RAT_STRU          stUserSpecPlmnInfo;

    VOS_UINT32                          ulSpecPlmnSearchFlg;

    pstUserSelReqMsg  = (MSCC_MMC_PLMN_SPECIAL_REQ_STRU*)pstMsg;

    /* 指向用户指定搜网消息中的PlmnId */
    pstUserSpecPlmnId = (NAS_MML_PLMN_ID_STRU*)(&pstUserSelReqMsg->stPlmnId);

    /* 参数检查 */
    if ( VOS_FALSE == NAS_MMC_CheckUserSpecPlmnSearchParamMsg_PreProc(pstMsg) )
    {
        /* 向MSCC发送用户指定搜网拒绝，不更新搜网模式 */
        NAS_MMC_SndMsccUserSpecPlmnSearchRej();
        return VOS_TRUE;
    }

    /* 关机状态如果svlte nv开启则允许cops设置写nv项并更新用户指定plmn信息,给mma回复成功 */
    enFsmId = NAS_MMC_GetCurrFsmId();
    if ((NAS_MMC_L1_STA_NULL == NAS_MMC_GetFsmTopState())
     && (NAS_MMC_FSM_L1_MAIN == enFsmId))
    {
        NAS_MMC_SetPlmnSelectionMode(NAS_MMC_PLMN_SELECTION_MODE_MANUAL);

        /* 将当前重选模式写入NVIM中 */
        NAS_MMC_WritePlmnSelectionModeNvim();

        stUserSpecPlmnInfo.stPlmnId.ulMcc = pstUserSpecPlmnId->ulMcc;
        stUserSpecPlmnInfo.stPlmnId.ulMnc = pstUserSpecPlmnId->ulMnc;
        stUserSpecPlmnInfo.enRat          = pstUserSelReqMsg->enAccessMode;

        NAS_MMC_SetUserSpecPlmnId(&stUserSpecPlmnInfo);

        /* 向MSCC发送用户指定搜网成功 */
        NAS_MMC_SndMsccUserSpecPlmnSearchCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);

        return VOS_TRUE;
    }

    /* 卡不在位或卡无效的处理 */
    if ( VOS_FALSE == NAS_MML_IsUsimStausValid() )
    {
        /* 更新手动搜网参数 */
        NAS_MMC_UpdateUserSpecPlmnSearchInfo_PreProc(pstUserSpecPlmnId,
                                                           pstUserSelReqMsg->enAccessMode);

        /* 向MSCC发送用户指定搜网失败 */
        NAS_MMC_SndMsccUserSpecPlmnSearchCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_FAIL);
         return VOS_TRUE;
    }

    enFsmId = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    /* 关机情况下返回无效值 */
    /* 如果重选状态机中收到用户指定搜，用户指定的网络和当前驻留的网络相同，
       并且当前驻留的网络能提供正常服务，则不打断重选状态机，直接回复搜网成功 */
    if ( ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
       && (NAS_MMC_FSM_L1_MAIN == enFsmId))
      || (NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId) )
    {
        /* 判断用户指定的网络和当前驻留的网络是否相同 */
        ulRslt = NAS_MML_CompareBcchPlmnwithSimPlmn(NAS_MML_GetCurrCampPlmnId(),
                                                    pstUserSpecPlmnId);
        enCurrRatType = NAS_MML_GetCurrNetRatType();
        if ( (VOS_TRUE == ulRslt)
          && (pstUserSelReqMsg->enAccessMode == enCurrRatType) )
        {

            /* 当前处在正常服务状态且未进行过搜网，否则在高优先级指定搜网时
               或者快速指定搜时，被用户指定搜打断，若用户指定的网络和先前驻留的
               网络相同，高优先级搜网状态机退出后，会不进行用户指定搜 */
            ulSpecPlmnSearchFlg = NAS_MMC_GetSpecPlmnSearchState();

            if ( (VOS_FALSE == NAS_MMC_NeedTrigPlmnSrch_UserSpecCurrentPlmn())
              && (NAS_MMC_SPEC_PLMN_SEARCH_RUNNING != ulSpecPlmnSearchFlg) )
            {

                /* 更新手动搜网参数 */
                NAS_MMC_UpdateUserSpecPlmnSearchInfo_PreProc(pstUserSpecPlmnId,
                                                                   pstUserSelReqMsg->enAccessMode);

                /* 设置当前注册状态为成功 */
                NAS_MMC_SetUserSpecPlmnRegisterStatus(VOS_TRUE);

                /* 向MSCC发送用户指定搜网成功 */
                NAS_MMC_SndMsccUserSpecPlmnSearchCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);
                return VOS_TRUE;
            }
        }
    }

    ulCsServiceStatusFlg = NAS_MML_GetCsServiceExistFlg();

    if (VOS_TRUE == ulCsServiceStatusFlg)
    {
        /* 向MSCC发送用户指定搜网失败 */
        /* 向MSCC发送用户指定搜网拒绝，不更新搜网模式 */
        NAS_MMC_SndMsccUserSpecPlmnSearchRej();
        return VOS_TRUE;
    }

    /* 判断当前是否需要打断当前状态机 */
    ulRslt = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    if (VOS_TRUE == ulRslt)
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32  NAS_MMC_RcvUserReselReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PLMN_USER_RESEL_REQ_STRU                      *pUserReselReq       = VOS_NULL_PTR;

    pUserReselReq  = (MSCC_MMC_PLMN_USER_RESEL_REQ_STRU*)pstMsg;

    /* 用户发起手动重选，目前不支持 */
    if (NAS_MSCC_PIF_PLMN_SEL_MODE_AUTO != pUserReselReq->enPlmnSelMode)
    {
        /* 异常打印 */
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvUserReselReq_PreProc:Entered!");
        NAS_MMC_SndMsccUserSpecPlmnSearchCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_REJ);

        return VOS_TRUE;
    }

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_USER_AUTO_SEL);

    /* 当前不是正常服务状态 */
    if (VOS_TRUE != NAS_MMC_IsNormalServiceStatus())
    {
        return NAS_MMC_ProcUserReselReqUnNormalSrv_PreProc(pUserReselReq);
    }
    else
    {
        return NAS_MMC_ProcUserReselReqNormalService_PreProc(pUserReselReq);
    }

}



VOS_UINT32  NAS_MMC_RcvTiAvailTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                                              ulRrcConnExistFlag;

#if   (FEATURE_ON == FEATURE_LTE)
    NAS_MML_SIM_FORBIDPLMN_INFO_STRU                       *pstForbidPlmnInfo = VOS_NULL_PTR;
    VOS_UINT32                                              ulRet;
#endif

    NAS_MMC_PLMN_SELECTION_MODE_ENUM_UINT8                  enSelectionMode;
    NAS_MML_PLMN_WITH_RAT_STRU                             *pstUserSpecPlmnId = VOS_NULL_PTR;
    VOS_UINT32                                              ulForbPlmnFlag;
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;            /* 当前状态机标识 */
    VOS_UINT32                                              ulState;            /* 当前状态机状态 */

    enSelectionMode   = NAS_MMC_GetPlmnSelectionMode();

    pstUserSpecPlmnId = NAS_MMC_GetUserSpecPlmnId();
    ulForbPlmnFlag    = NAS_MML_IsPlmnIdInForbidPlmnList( &(pstUserSpecPlmnId->stPlmnId) );

    /* 获取当前状态机ID及其状态 */
    enFsmId           = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    ulState           = NAS_MMC_GetFsmTopState();


    /* 周期搜网定时器超时时候，如果当前存在NO RF标记信息，停止定时器，直接返回 */
    if (VOS_FALSE == NAS_MML_GetRfAvailFlg())
    {
        /* 重启定时器，不搜网，但是不改变定时器计数器 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, NAS_MMC_GetNextAvailableTimerValue());

        return VOS_TRUE;
    }

    /* 当前为手动模式且用户指定的网络在禁止网络列表中，且当前不为OOC状态，
       则此次搜网没有意义，故不处理AvailTimer超时消息 */
    if ( (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == enSelectionMode)
      && (VOS_TRUE                           == ulForbPlmnFlag)
      && (NAS_MMC_FSM_L1_MAIN                == enFsmId)
      && (NAS_MMC_L1_STA_OOC                 != ulState) )
    {
        return VOS_TRUE;
    }

#if   (FEATURE_ON == FEATURE_LTE)
    /* LTE ONLY 当前为手动模式且用户指定的网络在禁止GPRS列表中，且当前不为OOC状态，
       则此次搜网没有意义，故不处理AvailTimer超时消息 */
    pstForbidPlmnInfo = NAS_MML_GetForbidPlmnInfo();

    ulRet = NAS_MML_IsSimPlmnIdInDestBcchPlmnList(&(pstUserSpecPlmnId->stPlmnId),
                                                  pstForbidPlmnInfo->ucForbGprsPlmnNum,
                                                  pstForbidPlmnInfo->astForbGprsPlmnList);

    if ((VOS_TRUE == NAS_MML_IsLteOnlyMode(NAS_MML_GetMsPrioRatList()))
     && (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == enSelectionMode)
     && (VOS_TRUE == ulRet )
     && (NAS_MMC_FSM_L1_MAIN == enFsmId)
     && (NAS_MMC_L1_STA_OOC != ulState))
    {
        return VOS_TRUE;
    }
#endif

    /* 判断信令链接是否已经完成释放 */
    ulRrcConnExistFlag          = NAS_MML_IsRrcConnExist();

    /* 有连接时，设置CTX中的变量记录需要搜网*/
    if (VOS_TRUE == ulRrcConnExistFlag)
    {
        NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE, NAS_MMC_PLMN_SEARCH_SCENE_AVAILABLE_TIMER_EXPIRED);

        return VOS_TRUE;
    }

    /* 进状态机里触发搜网 */
    return VOS_FALSE;
}

/* Added by c00318887 for 预置频点搜网优化, 2015-8-26, begin */

VOS_UINT32  NAS_MMC_RcvTiPlmnSearchPhaseOneTotalTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_AVAIL_TIMER_CFG_STRU                 *pstAvailTimerCfg        = VOS_NULL_PTR;
    VOS_UINT32                                    ulNormalCount;

    pstAvailTimerCfg                            = NAS_MML_GetAvailTimerCfg();
    ulNormalCount                               = NAS_MMC_GetCurNormalAvailableTimerCount_L1Main();

    NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvTiControlFirstPlmnSearchTimerExpired_PreProc entered.");

    /* 在其他状态机下超时，缓存直到L1 MAIN 再处理 */
    if (NAS_MMC_FSM_L1_MAIN == NAS_MMC_GetCurrFsmId())
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_PLMN_SEARCH_PHASE_ONE_TOTAL_TIMER);
        /* 如果已经在搜网第二阶段，则返回 */
        if (ulNormalCount >= pstAvailTimerCfg->ulFirstSearchTimeCount)
        {
            NAS_WARNING_LOG2(WUEPS_PID_MMC, "NAS_MMC_RcvTiControlFirstPlmnSearchTimerExpired_PreProc WANRING: ue is in plmn search phaseII",
                                ulNormalCount, pstAvailTimerCfg->ulFirstSearchTimeCount);

        }

        /* 将normal available 次数设为第一阶段数目，跳到第二阶段搜网 */
        NAS_MMC_SetAvailableTimerType_OnPlmn(NAS_MMC_AVAILABLE_TIMER_TYPE_NORMAL);
        NAS_MMC_SetCurNormalAvailableTimerCount(pstAvailTimerCfg->ulFirstSearchTimeCount);
        NAS_MMC_ResetCurHistorySearchTimerCount_OnPlmn();
        NAS_MMC_ResetCurNcellSearchTimerCount_OnPlmn();

        /* 如果当前available搜网在运行，则重启定时器等待其超时触发后续动作 */
        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_AVAILABLE_TIMER);
        }

        /* 在选网状态机打断后，则触发AVAILABLE定时器超时，触发后续选网动作 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_AVAILABLE_TIMER, 500);

        return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;

}

/* Added by c00318887 for 预置频点搜网优化, 2015-8-26, end */



VOS_VOID NAS_MMC_RcvWasCellId_PreProc(
    RRC_NAS_CELL_ID_ST                  *pstWasCellId
)
{
    NAS_MML_CAMP_CELL_INFO_STRU         *pstCampCellInfo     = VOS_NULL_PTR;
    NAS_MML_CAMP_PLMN_INFO_STRU         *pstCurrCampPlmnInfo = VOS_NULL_PTR;


    /* 若不在驻留态则不处理信息上报 */
    if (NAS_MMC_AS_CELL_NOT_CAMP_ON == NAS_MMC_GetAsCellCampOn() )
    {
        return;
    }


    /*上报小区个数超过8个认为错误 */
    if (pstWasCellId->ucIDNum > RRC_NAS_AT_CELL_MAX_NUM)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC ,"NAS_MMC_RcvWasAtMsgCnf():WARNING:the NUM is out limit.");
        return;
    }


    pstCampCellInfo     = NAS_MML_GetCampCellInfo();
    pstCurrCampPlmnInfo = NAS_MML_GetCurrCampPlmnInfo();

    /* 当小区不改变，而能力发生改变时，需上报*/
    if ( (pstCampCellInfo->astCellInfo[0].ulCellId == pstWasCellId->aulCellId[0])
       &&(pstCurrCampPlmnInfo->enSysSubMode != pstWasCellId->enSysSubMode))
    {
        /* 保存WRR上报的当前小区空闲态下的系统子模式 */
        pstCurrCampPlmnInfo->enSysSubMode = pstWasCellId->enSysSubMode;

        /*调用小区信息上报函数，上报taf小区信息*/
        NAS_MMC_SndMsccSysInfo();
    }


}




VOS_VOID NAS_MMC_RcvWasCellBler_PreProc(
    RRC_NAS_CELL_TRCH_BLER_STRU_LIST    *pstWasCellBler
)
{
    VOS_UINT32                           i;
    NAS_MML_CAMP_CELL_INFO_STRU         *pstCampCellInfo;
    MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN    unMeasReportType;

    pstCampCellInfo  = NAS_MML_GetCampCellInfo();

    PS_MEM_SET(((VOS_UINT8*)&unMeasReportType), 0, sizeof(MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN));
    unMeasReportType.ucMeasReportType |= NAS_MSCC_PIF_SCELL_MEAS_TYPE_RXQUALITY;

    /*误码率超过8个认为消息无效 */
    if ( pstWasCellBler->ucTrchNum > RRC_NAS_AT_CELL_MAX_NUM)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvWasAtMsgCnf():WARNING:the BLER NUM is out limit.");
        return;
    }


    /*处理上报误码率信息*/
    pstCampCellInfo->ucRssiNum = pstWasCellBler->ucTrchNum;

    for ( i = 0; i < pstWasCellBler->ucTrchNum ; i++ )
    {
        if ( pstWasCellBler->stTrchBlerInfo[i].ucDlTrChBler > NAS_MML_UTRA_BLER_INVALID)
        {
            pstCampCellInfo->astRssiInfo[i].ucChannalQual = NAS_MML_UTRA_BLER_INVALID;
        }
        else
        {
            pstCampCellInfo->astRssiInfo[i].ucChannalQual = pstWasCellBler->stTrchBlerInfo[i].ucDlTrChBler;
        }
    }

    /*处理上报信号强度*/
    NAS_MMC_SndMsccRssiInd(&unMeasReportType);
}

VOS_VOID NAS_MMC_RcvWasCellRssi_PreProc(
    RRC_NAS_CELL_RSCP_STRU_LIST         *pstWasCellRscp
)
{
    VOS_UINT32                           i;
    NAS_MML_CAMP_CELL_INFO_STRU         *pstCampCellInfo;
    MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN    unMeasReportType;

    pstCampCellInfo  = NAS_MML_GetCampCellInfo();

    PS_MEM_SET(((VOS_UINT8*)&unMeasReportType), 0, sizeof(MSCC_MMC_SCELL_MEAS_REPORT_TYPE_UN));
    unMeasReportType.ucMeasReportType |= NAS_MSCC_PIF_SCELL_MEAS_TYPE_RXLEV;

    if ( pstWasCellRscp->usCellNum > RRC_NAS_AT_CELL_MAX_NUM)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC ,"NAS_MMC_RcvWasCellRssi_PreProc():WARNING:RSCP NUM is out limit.");
        return;
    }

    pstCampCellInfo->ucRssiNum  = (VOS_UINT8)pstWasCellRscp->usCellNum;

    for ( i = 0; i < pstWasCellRscp->usCellNum ; i++ )
    {
        if ( (pstWasCellRscp->stCellRscp[i].sCellRSCP > NAS_MML_RSCP_UPPER_LIMIT)
          || (pstWasCellRscp->stCellRscp[i].sCellRSCP < NAS_MML_RSCP_LOWER_LIMIT))
        {
            pstCampCellInfo->astRssiInfo[i].sRscpValue = NAS_MML_UTRA_RSCP_UNVALID;
        }
        else
        {
            pstCampCellInfo->astRssiInfo[i].sRscpValue = pstWasCellRscp->stCellRscp[i].sCellRSCP;
        }

        if ((pstWasCellRscp->stCellRscp[i].sRSSI > NAS_MML_RSSI_UPPER_LIMIT)
         || (pstWasCellRscp->stCellRscp[i].sRSSI < NAS_MML_RSSI_LOWER_LIMIT))
        {
            pstCampCellInfo->astRssiInfo[i].sRssiValue = NAS_MML_RSSI_UNVALID;
        }
        else
        {
            pstCampCellInfo->astRssiInfo[i].sRssiValue  = pstWasCellRscp->stCellRscp[i].sRSSI;
        }
    }

    pstCampCellInfo->usCellDlFreq   = pstWasCellRscp->usCellDlFreq;
    pstCampCellInfo->usCellUlFreq   = pstWasCellRscp->usCellUlFreq;
    pstCampCellInfo->sUeRfPower     = pstWasCellRscp->sUeRfPower;

    for ( i = 0; i < pstWasCellRscp->usCellNum ; i++ )
    {
        if ((pstWasCellRscp->stCellRscp[i].sCellRSCP > NAS_MML_RSCP_UPPER_LIMIT)
            ||(pstWasCellRscp->stCellRscp[i].sCellRSCP < NAS_MML_RSCP_LOWER_LIMIT))
        {
            pstCampCellInfo->astCellInfo[i].sCellRSCP = NAS_MML_UTRA_RSCP_UNVALID;
        }
        else
        {
            pstCampCellInfo->astCellInfo[i].sCellRSCP = pstWasCellRscp->stCellRscp[i].sCellRSCP;
        }
    }

    /*处理上报信号强度*/
    NAS_MMC_SndMsccRssiInd(&unMeasReportType);
}



VOS_VOID  NAS_MMC_UpdateUserSpecPlmnSearchInfo_PreProc(
    NAS_MML_PLMN_ID_STRU               *pstPlmnId,
    VOS_UINT8                           ucAccessMode
)
{
    NAS_MML_PLMN_WITH_RAT_STRU          stUserSpecPlmn;

    /* 保存用户指定的PlmnID及其接入技术 */
    stUserSpecPlmn.stPlmnId.ulMcc = pstPlmnId->ulMcc;
    stUserSpecPlmn.stPlmnId.ulMnc = pstPlmnId->ulMnc;

    stUserSpecPlmn.enRat          = ucAccessMode;
    NAS_MMC_SetUserSpecPlmnId(&stUserSpecPlmn);

    /* 保存指定搜网前驻留的PlmnID及接入技术 */
    NAS_MMC_SaveUserReselPlmnInfo();

    /* 当前模式已是MANUAL，则不更新NV */
    if (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        return;
    }

    /* 设置当前注册状态未注册 */
    NAS_MMC_SetUserSpecPlmnRegisterStatus(VOS_FALSE);

    /* 设置当前搜网模式为手动模式 */
    NAS_MMC_SetPlmnSelectionMode(NAS_MMC_PLMN_SELECTION_MODE_MANUAL);


    /* 如果高优先级搜网定时器在运行过程中，则停止该定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_HPLMN_TIMER))
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_HPLMN_TIMER);

        /* HPLMN TIMER 停止事件上报 */
        NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_STOP,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);
    }

    /* 如果周期高优先级搜网定时器在运行过程中，则停止该定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH))
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH);
    }

    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER))
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER);
        NAS_MMC_ResetCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
        NAS_MMC_InitTdHighRatSearchCount();
    }

    /* 将当前搜网模式写入NVIM中 */
    NAS_MMC_WritePlmnSelectionModeNvim();

    /* 搜网模式变化上报USIM */
    if (VOS_TRUE == NAS_MMC_IsNeedSndStkSearchModeChgEvt())
    {
        NAS_MMC_SndStkSearchModeChgEvt(NAS_MMC_PLMN_SELECTION_MODE_MANUAL);
    }

}


VOS_UINT32 NAS_MMC_CheckUserSpecPlmnSearchParamMsg_PreProc(
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PLMN_SPECIAL_REQ_STRU      *pstUserSelReqMsg   = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU               *pstUserSpecPlmnId  = VOS_NULL_PTR;

    pstUserSelReqMsg = (MSCC_MMC_PLMN_SPECIAL_REQ_STRU*)pstMsg;

    /* 指向用户指定搜网消息中的PlmnId */
    pstUserSpecPlmnId  = (NAS_MML_PLMN_ID_STRU*)&(pstUserSelReqMsg->stPlmnId);

    /* 判断用户指定的Plmn是否有效，无效，则返回VOS_FALSE */
    if (VOS_FALSE == NAS_MML_IsPlmnIdValid(pstUserSpecPlmnId))
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_CheckUserSpecPlmnSearchParamMsg_PlmnSelection():plmn invalid");
        return VOS_FALSE;
    }

    /* 该PLMN ID是否在锁网锁卡的PLMN 列表中 */
    if (VOS_TRUE == NAS_MML_IsBcchPlmnIdInLockPlmnList(pstUserSpecPlmnId))
    {
        return VOS_FALSE;
    }

    /* 指定PLMN 不允许漫游, 返回VOS_FALSE */
    if ( VOS_TRUE != NAS_MML_IsPlmnIdRoamingAllowed(pstUserSpecPlmnId) )
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvTafPlmnListReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    /* 当前的PS业务判断不准确,收到LIST搜网发给RRC判断能否发起,
       CS业务直接回复REJ */
    if (VOS_TRUE == NAS_MML_GetCsServiceExistFlg())
    {
        /* 发送LIST REJ给MSCC */
        NAS_Mmc_SndMsccPlmnListRej();

        return VOS_TRUE;
    }



    /* 周期性尝试内部LIST搜网定时器在运行，则停止 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(
                                       TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST))
    {
        /* 停止TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST */
        NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST);
    }

    /* 进状态机处理，状态机不能处理则进后处理 */
    return VOS_FALSE;
}


VOS_UINT32  NAS_MMC_RcvTiPeriodTryingUserPlmnListExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    /* 当前的PS业务判断不准确,收到LIST搜网发给RRC判断能否发起,
       CS业务直接回复REJ */
    if (VOS_TRUE == NAS_MML_GetCsServiceExistFlg())
    {
        /* 发送LIST REJ给MSCC */
        NAS_Mmc_SndMsccPlmnListRej();

        return VOS_TRUE;
    }



    /* 进状态机处理，状态机不能处理则进后处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvTiPeriodTryingInterPlmnListExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT8                           ucCsSigConnStatusFlg;
    NAS_MMC_AS_CELL_CAMP_ON_ENUM_UINT8  enCampStaus;
    NAS_MMC_FSM_ID_ENUM_UINT32          enCurFsmId;

    VOS_UINT32                          ulSingleDomainNeedPlmnSrch;
#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurRatType;

    enCurRatType                = NAS_MML_GetCurrNetRatType();
#endif

    ulSingleDomainNeedPlmnSrch  = NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_PS);

    /* 如果已经启动了Available定时器进行搜网，不需要发起内部列表搜超时消息，丢弃该消息 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER))
    {
        return VOS_TRUE;
    }

    /* 当前正常服务,丢弃该消息 */
    if ( VOS_TRUE == NAS_MMC_IsNormalServiceStatus())
    {
#if   (FEATURE_ON == FEATURE_LTE)
        if (NAS_MML_NET_RAT_TYPE_LTE == enCurRatType)
        {
            if (VOS_FALSE == NAS_MMC_IsCsPsMode1NeedPeriodSearchGU())
            {
                return VOS_TRUE;
            }
        }
        else
        {
            if ((VOS_FALSE == NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_CS))
             && (VOS_FALSE == ulSingleDomainNeedPlmnSrch))
            {
                return VOS_TRUE;
            }
        }
#else
        if ((VOS_FALSE == NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_CS))
         && (VOS_FALSE == ulSingleDomainNeedPlmnSrch))
        {
            return VOS_TRUE;
        }
#endif
    }

    /* 当前无卡时，丢弃该消息 */
    if (VOS_FALSE == NAS_MML_IsUsimStausValid())
    {
        return VOS_TRUE;
    }

    /* 当前状态机ID */
    enCurFsmId = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enCurFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    /* 子状态机运行的判断 */
    if ( NAS_MMC_FSM_L1_MAIN != enCurFsmId)
    {
        /* 目前只有在搜网状态机运行过程中丢弃该消息 */
        if ((NAS_MMC_FSM_PLMN_SELECTION == enCurFsmId)
          ||(NAS_MMC_FSM_ANYCELL_SEARCH == enCurFsmId))
        {
            return VOS_TRUE;
        }

        /* 其它状态机运行时启动定时器周期性尝试 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST, TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST_LEN);

        return VOS_TRUE;
    }

    enCampStaus                 = NAS_MMC_GetAsCellCampOn();
    ucCsSigConnStatusFlg        = NAS_MML_GetCsSigConnStatusFlg();

    /* 当前的PS业务判断不准确,收到LIST搜网发给RRC判断能否发起,
       CS业务或存在CS信令连接或未驻留时不能立即发起LIST搜重启定时器尝试 */
    if ((VOS_TRUE                    == NAS_MML_GetCsServiceExistFlg())
     || (NAS_MMC_AS_CELL_NOT_CAMP_ON == enCampStaus)
     || (VOS_TRUE                    == ucCsSigConnStatusFlg))
    {
        /* 启动定时器TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST, TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST_LEN);

        return VOS_TRUE;
    }


    /* 进入L1 MAIN处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvMmcInterPlmnListReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT8                           ucCsSigConnStatusFlg;
    NAS_MMC_AS_CELL_CAMP_ON_ENUM_UINT8  enCampStaus;
    VOS_UINT32                          ulSingleDomainNeedPlmnSrch;
#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurRatType;

    enCurRatType            = NAS_MML_GetCurrNetRatType();
#endif

    ulSingleDomainNeedPlmnSrch = NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_PS);

    /* 周期性尝试用户LIST搜网定时器在运行，则不处理内部LIST搜,丢弃该消息 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_USER_PLMN_LIST))
    {
        return VOS_TRUE;
    }

    /* 当前正常服务而且不是CS/PS mode 1驻留LTE周期性搜索GSM或WCDMA网络,
       丢弃该消息 */
    if (VOS_TRUE == NAS_MMC_IsNormalServiceStatus())
    {
#if   (FEATURE_ON == FEATURE_LTE)
        if (NAS_MML_NET_RAT_TYPE_LTE == enCurRatType)
        {
            if (VOS_FALSE == NAS_MMC_IsCsPsMode1NeedPeriodSearchGU())
            {
                return VOS_TRUE;
            }
        }
        else
        {
            if ((VOS_FALSE == NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_CS))
             && (VOS_FALSE == ulSingleDomainNeedPlmnSrch))
            {
                return VOS_TRUE;
            }
        }
#else
        if ((VOS_FALSE == NAS_MMC_IsSingleDomainRegFailNeedPlmnSrch(NAS_MSCC_PIF_SRVDOMAIN_CS))
         && (VOS_FALSE == ulSingleDomainNeedPlmnSrch))
        {
            return VOS_TRUE;
        }
#endif
    }

    /* 当前无卡时，丢弃该消息 */
    if (VOS_FALSE == NAS_MML_IsUsimStausValid())
    {
        return VOS_TRUE;
    }

    enCampStaus                 = NAS_MMC_GetAsCellCampOn();
    ucCsSigConnStatusFlg        = NAS_MML_GetCsSigConnStatusFlg();

    /* 当前的PS业务判断不准确,收到LIST搜网发给RRC判断能否发起,
       CS业务或存在CS信令连接或未驻留时不能立即发起LIST搜重启定时器尝试 */
    if ((VOS_TRUE                    == NAS_MML_GetCsServiceExistFlg())
     || (NAS_MMC_AS_CELL_NOT_CAMP_ON == enCampStaus)
     || (VOS_TRUE                    == ucCsSigConnStatusFlg))
    {
        /* 启动定时器TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST, TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST_LEN);

        return VOS_TRUE;
    }


    /* L1 MAIN上，进状态机里触发搜网 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvTafPlmnListAbortReq_PlmnList_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulCacheMsgIdx;
    VOS_UINT32                          ulExistFlg;
    VOS_UINT32                          ulCacheEventType;
    NAS_MMC_FSM_ID_ENUM_UINT32          ulCurrentFsmId;
    VOS_UINT32                          ulFsmIdInFsmStackFlg;

    /* 用户LIST搜网消息*/
    ulCacheEventType     = NAS_BuildEventType(UEPS_PID_MSCC, ID_MSCC_MMC_PLMN_LIST_REQ);
    ulExistFlg           = NAS_MMC_IsExistCacheMsg(ulCacheEventType, &ulCacheMsgIdx);
    ulCurrentFsmId       = NAS_MMC_GetCurrFsmId();

    if (NAS_FSM_BUTT <= ulCurrentFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }

    ulFsmIdInFsmStackFlg = NAS_MMC_IsFsmIdInFsmStack(NAS_MMC_FSM_PLMN_LIST);

    /* 有用户LIST搜网缓存 */
    if (VOS_TRUE == ulExistFlg )
    {
        /* 清除缓存 */
        NAS_MMC_ClearCacheMsg(ulCacheEventType);

        /* 向MSCC回复ID_MMC_MSCC_PLMN_LIST_ABORT_CNF */
        NAS_MMC_SndMsccPlmnListAbortCnf(VOS_NULL_PTR);

        return VOS_TRUE;
    }

    /* 周期性尝试用户LIST搜网定时器是否在运行 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_USER_PLMN_LIST))
    {
        /* 停止TI_NAS_MMC_PERIOD_TRYING_USER_PLMN_LIST */
        NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_USER_PLMN_LIST);

        /* 向MSCC回复ID_MMC_MSCC_PLMN_LIST_ABORT_CNF */
        NAS_MMC_SndMsccPlmnListAbortCnf(VOS_NULL_PTR);

        return VOS_TRUE;
    }

    /* LIST搜网状态机未运行，为保证AT通道正常，此时也给MSCC回复结果 */
    if ((ulCurrentFsmId != NAS_MMC_FSM_PLMN_LIST)
     && (VOS_FALSE == ulFsmIdInFsmStackFlg))
    {
        /* 异常打印 */
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvTafPlmnListAbortReq_PlmnList_PreProc:ERROR");

        /* 向MSCC回复ID_MMC_MSCC_PLMN_LIST_ABORT_CNF */
        NAS_MMC_SndMsccPlmnListAbortCnf(VOS_NULL_PTR);

        return VOS_TRUE;
    }

    /* 进状态机处理 */
    return VOS_FALSE;
}




VOS_UINT32  NAS_MMC_RcvTafSpecPlmnSearchAbortReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulCacheMsgIdx;
    VOS_UINT32                          ulExistFlg;
    VOS_UINT32                          ulCacheEventType;
    NAS_MMC_FSM_ID_ENUM_UINT32          ulCurrentFsmId;
    VOS_UINT32                          ulFsmIdInFsmStackFlg;

    /* 用户指定搜网消息*/
    ulCacheEventType     = NAS_BuildEventType(UEPS_PID_MSCC, ID_MSCC_MMC_PLMN_SPECIAL_REQ);
    ulExistFlg           = NAS_MMC_IsExistCacheMsg(ulCacheEventType, &ulCacheMsgIdx);
    ulCurrentFsmId       = NAS_MMC_GetCurrFsmId();
    ulFsmIdInFsmStackFlg = NAS_MMC_IsFsmIdInFsmStack(NAS_MMC_FSM_PLMN_SELECTION);

    if (NAS_FSM_BUTT <= ulCurrentFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }

    /* 有用户指定搜网缓存 */
    if (VOS_TRUE == ulExistFlg )
    {
        /* 清除缓存 */
        NAS_MMC_ClearCacheMsg(ulCacheEventType);

        /* 向MSCC回复TAFMMC_SPEC_PLMN_SEARCH_ABORT_CNF */
        NAS_MMC_SndMsccSpecPlmnSearchAbortCnf();
        return VOS_TRUE;
    }

    /* 指定搜网状态机未在运行，为保证AT通道正常，此时也给MSCC回复结果 */
    if ((ulCurrentFsmId != NAS_MMC_FSM_PLMN_SELECTION)
     && (VOS_FALSE == ulFsmIdInFsmStackFlg))
    {
        /* 异常打印 */
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvTafSpecPlmnSearchAbortReq_PreProc:ERROR");

        /* 向MSCC回复TAFMMC_SPEC_PLMN_SEARCH_ABORT_CNF */
        NAS_MMC_SndMsccSpecPlmnSearchAbortCnf();
        return VOS_TRUE;
    }

    /* 进状态机处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvMsccStopGetGeoReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulCacheMsgIdx;
    VOS_UINT32                          ulExistFlg;
    VOS_UINT32                          ulCacheEventType;
    NAS_MMC_FSM_ID_ENUM_UINT32          ulCurrentFsmId;
    VOS_UINT32                          ulFsmIdInFsmStackFlg;

    /* 用户获取国家码消息*/
    ulCacheEventType     = NAS_BuildEventType(UEPS_PID_MSCC, ID_MSCC_MMC_GET_GEO_REQ);
    ulExistFlg           = NAS_MMC_IsExistCacheMsg(ulCacheEventType, &ulCacheMsgIdx);
    ulCurrentFsmId       = NAS_MMC_GetCurrFsmId();
    ulFsmIdInFsmStackFlg = NAS_MMC_IsFsmIdInFsmStack(NAS_MMC_FSM_GET_GEO);

    if (NAS_FSM_BUTT <= ulCurrentFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }

    /* 用户获取国家码缓存 */
    if (VOS_TRUE == ulExistFlg )
    {
        /* 清除缓存 */
        NAS_MMC_ClearCacheMsg(ulCacheEventType);

        /* 向MSCC回复ID_MMC_MSCC_STOP_GET_GEO_CNF */
        NAS_MMC_SndMsccStopGetGeoCnf();
        return VOS_TRUE;
    }

    /* 获取国家码状态机未在运行，此时也给MSCC回复结果 */
    if ((ulCurrentFsmId != NAS_MMC_FSM_GET_GEO)
     && (VOS_FALSE == ulFsmIdInFsmStackFlg))
    {
        /* 异常打印 */
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccStopGetGeoReq_PreProc:ERROR");

        /* 向MSCC回复ID_MMC_MSCC_STOP_GET_GEO_CNF */
        NAS_MMC_SndMsccStopGetGeoCnf();

        return VOS_TRUE;
    }

    /* 进状态机处理 */
    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_IsConnExist_BgPlmnSearch_PreProc( VOS_VOID )
{
    NAS_MML_CONN_STATUS_INFO_STRU      *pstConnStatus = VOS_NULL_PTR;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurrRat;

    /* 取得当前的接入模式 */
    enCurrRat     = NAS_MML_GetCurrNetRatType();

    /* 取得当前的链接信息 */
    pstConnStatus = NAS_MML_GetConnStatus();

    /* 存在CS域的信令连接,不允许发起BG搜索 */
    if (VOS_TRUE == pstConnStatus->ucCsSigConnStatusFlg)
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: Cs Signal Conn exit");
    
        return VOS_TRUE;
    }

    /* 存在CS域的业务或缓存的业务,不允许发起BG搜索 */
    if (VOS_TRUE == NAS_MML_GetCsServiceExistFlg())
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: Cs service exit");
    
        return VOS_TRUE;
    }

    /* 存在PS域的缓存的业务,不允许发起BG搜索 */
    if ( VOS_TRUE == pstConnStatus->ucPsServiceBufferFlg)
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: Ps service buffer flg is true");
    
        return VOS_TRUE;
    }

    /* GSM模式下,TBF存在,不允许发起BG搜索 */
    if ((VOS_TRUE == pstConnStatus->ucPsTbfStatusFlg)
     && (NAS_MML_NET_RAT_TYPE_GSM == enCurrRat))
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: ps tbf exit under gsm");
    
        return VOS_TRUE;
    }

#if (FEATURE_ON == FEATURE_LTE)

    /* L下PS域有信令连接或业务存在，不允许发起BG搜索 */
    if ((NAS_MML_NET_RAT_TYPE_LTE == enCurrRat)
     && ((VOS_TRUE == pstConnStatus->ucEpsSigConnStatusFlg)
      || (VOS_TRUE == pstConnStatus->ucEpsServiceConnStatusFlg)))
    {
        NAS_NORMAL_LOG2(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: ucEpsSigConnStatusFlg ucEpsServiceConnStatusFlg",
                            pstConnStatus->ucEpsSigConnStatusFlg, pstConnStatus->ucEpsServiceConnStatusFlg);
    
        return VOS_TRUE;
    }
#endif

    /* PS域触发的TC环回业务存在,不允许发起BG搜网 */
    if ( VOS_TRUE == pstConnStatus->ucPsTcServiceFlg )
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsConnExist_BgPlmnSearch_PreProc: ucPsTcServiceFlg is true");
    
        return VOS_TRUE;
    }

    /* 其他情况认为不存在业务或信令连接,可以发起BG搜索 */
    return VOS_FALSE;

}



VOS_UINT32 NAS_MMC_IsEnableBgPlmnSearch_PreProc( VOS_VOID )
{
    NAS_MML_CONN_STATUS_INFO_STRU      *pstConnStatus = VOS_NULL_PTR;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurrRat;

    /* 取得当前的接入模式 */
    enCurrRat     = NAS_MML_GetCurrNetRatType();

    /* 取得当前的链接信息 */
    pstConnStatus = NAS_MML_GetConnStatus();


    /* 如果已经启动了Available定时器进行搜网，不需要发起背景搜索 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_AVAILABLE_TIMER))
    {
        return VOS_FALSE;
    }

    /* 如果周期性尝试用户列表搜网定时器在运行过程中，不允许发起背景搜索 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_USER_PLMN_LIST))
    {
        return VOS_FALSE;
    }

    /* 如果周期性尝试内部列表搜网定时器在运行过程中，不允许发起背景搜索 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_INTER_PLMN_LIST))
    {
        return VOS_FALSE;
    }

    /* 如果接入层上报not camp on消息，不允许发起背景搜索 */
    if (NAS_MMC_AS_CELL_NOT_CAMP_ON == NAS_MMC_GetAsCellCampOn())
    {
        return VOS_FALSE;
    }

    /* 为了避免反复调用高优先级搜网的处理，在10s周期性尝试定时器运行期间不能立刻发起背景搜索 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH))
    {
        return VOS_FALSE;
    }

    /* 存在信令连接或业务连接不允许发起背景搜网 */
    if (VOS_TRUE == NAS_MMC_IsConnExist_BgPlmnSearch_PreProc())
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_IsEnableBgPlmnSearch_PreProc: connect exit");
        return VOS_FALSE;
    }

    /* 有子状态机在运行,不运行发起背景搜索 */
    if( NAS_MMC_FSM_L1_MAIN != NAS_MMC_GetCurrFsmId())
    {
        return VOS_FALSE;
    }

    /* GSM模式下，RRC链接存在不允许发起背景搜索，
       WCDMA模式下，RRC链接存在P态,允许发起背景搜索，*/
    if ((VOS_TRUE == pstConnStatus->ucRrcStatusFlg)
     && (NAS_MML_NET_RAT_TYPE_WCDMA != enCurrRat))
    {
        return VOS_FALSE;
    }

    /* 其他情况可以发起BG搜索 */
    return VOS_TRUE;

}



VOS_UINT32 NAS_MMC_ProcTiHplmnTimerExpired_PreProc(VOS_VOID)
{
    NAS_MMC_PLMN_SELECTION_LIST_INFO_STRU                  *pstHighPrioPlmnSearchList = VOS_NULL_PTR;
    NAS_MML_BG_SEARCH_CFG_INFO_STRU                        *pstBgSearchCfg            = VOS_NULL_PTR;


    /* 获取高优先级搜网列表信息 */
    pstHighPrioPlmnSearchList = NAS_MMC_GetHighPrioPlmnList();


    /* HPLMN TIMER启动事件上报 */
    NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_EXPIRE,
                    VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);


    /* 需要判断HPLMN Timer定时器此刻是否有效，无效则直接丢弃该消息 */
    if ( VOS_FALSE == NAS_MMC_IsHighPrioPlmnSearchVaild() )
    {
        return VOS_TRUE;
    }

    pstBgSearchCfg            = NAS_MML_GetBgSearchCfg();

    /* 已经驻留在最高优先级网络下则直接返回丢弃该消息 */
    if ( VOS_TRUE == NAS_MMC_IsCampOnHighestPrioPlmn() )
    {
        /* 如果增强的Hplmn搜网,法电定制特性开关打开: 启动Hplmn Timer定时器,
           只要没有驻留在home PLMN上,也需要启动此定时器,第一次启动2Min,后续都以T Min启动*/
        if ((VOS_FALSE == NAS_MMC_IsCampOnHPlmn())
         && (VOS_TRUE == pstBgSearchCfg->ucEnhancedHplmnSrchFlg))
        {
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER,  NAS_MMC_GetHplmnTimerLen());


            /* HPLMN TIMER启动事件上报 */
            NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                            VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

        }
        else
        {
            /* 已经驻留在最高优先级网络上,法电增强HPLMN搜索未激活,下次按首次启动HPLMN定时器 */
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_TRUE);
        }

        return VOS_TRUE;
    }

    /* 无法进行高优先级搜索时,启动10s周期性尝试高优先级搜索定时器 */
    if ( VOS_FALSE == NAS_MMC_IsEnableBgPlmnSearch_PreProc() )
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH, TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH_LEN);

        /* 删除enable L的逻辑,放到快速指定搜网流程中 */

        /* 初始化高优先级搜网列表 */
        NAS_MMC_InitPlmnSelectionList(NAS_MMC_PLMN_SEARCH_SCENE_HIGH_PRIO_PLMN_SEARCH,
                                      VOS_NULL_PTR,
                                      pstHighPrioPlmnSearchList);

        return VOS_TRUE;
    }



    return VOS_FALSE;
}
VOS_UINT32 NAS_MMC_ProcTiTryingHighPrioPlmnSearchExpired_PreProc(VOS_VOID)
{
    NAS_MML_BG_SEARCH_CFG_INFO_STRU    *pstBgSearchCfg  = VOS_NULL_PTR;

    NAS_MML_SIM_STATUS_STRU            *pstSimStatus    = VOS_NULL_PTR;

    pstSimStatus = NAS_MML_GetSimStatus();

    /* 需要判断10s周期性尝试高优先级搜网定时器此刻是否有效，无效则直接丢弃该消息,预处理完成 */

    /* 手动模式下返回无效 */
    if (NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        return VOS_TRUE;
    }

    /* SIM卡不在位或CS且PS域的卡都无效时返回无效 */
    if ( VOS_FALSE == NAS_MML_GetSimPresentStatus() )
    {
        return VOS_TRUE;
    }

    if (( VOS_FALSE == pstSimStatus->ucSimPsRegStatus)
     && ( VOS_FALSE == pstSimStatus->ucSimCsRegStatus))
    {
        return VOS_TRUE;
    }

    pstBgSearchCfg            = NAS_MML_GetBgSearchCfg();

    /* 已经驻留在最高优先级网络下则直接返回丢弃该消息 */
    if ( VOS_TRUE == NAS_MMC_IsCampOnHighestPrioPlmn() )
    {
        /* 如果增强的Hplmn搜网,法电定制特性开关打开: 启动Hplmn Timer定时器,
           只要没有驻留在home PLMN上,也需要启动此定时器,第一次启动2Min,后续都以T Min启动*/
        if ((VOS_FALSE == NAS_MMC_IsCampOnHPlmn())
         && (VOS_TRUE == pstBgSearchCfg->ucEnhancedHplmnSrchFlg))
        {
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER,  NAS_MMC_GetHplmnTimerLen());


            /* HPLMN TIMER启动事件上报 */
            NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                            VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

        }
        else
        {
            /* 已经驻留在最高优先级网络上,法电增强HPLMN搜索未激活,下次按首次启动HPLMN定时器 */
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_TRUE);
        }

        return VOS_TRUE;
    }

    /* 无法进行高优先级搜索时,启动10s周期性尝试高优先级搜索定时器 */
    if ( VOS_FALSE == NAS_MMC_IsEnableBgPlmnSearch_PreProc() )
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH, TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH_LEN);
        return VOS_TRUE;
    }

    return VOS_FALSE;

}
VOS_UINT32 NAS_MMC_RcvTiHplmnTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulRst;

    ulRst = NAS_MMC_ProcTiHplmnTimerExpired_PreProc();
    return ulRst;
}


VOS_UINT32 NAS_MMC_RcvTiTryingHighPrioPlmnSearchExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulRst;

    ulRst = NAS_MMC_ProcTiTryingHighPrioPlmnSearchExpired_PreProc();

    return ulRst;
}



VOS_UINT32  NAS_MMC_RcvStkSteerRoamingInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    VOS_UINT16                          usOplmnNum;
    VOS_UINT8                           ucUpdateUsimForbPlmnFlg;
    VOS_UINT32                          i;
    NAS_MML_SIM_OPERPLMN_INFO_STRU     *pstOperPlmnInfo   = VOS_NULL_PTR;
    NAS_MML_SIM_PLMN_WITH_RAT_STRU     *pstOplmnAddr      = VOS_NULL_PTR;
    STK_NAS_STEERING_OF_ROAMING_STRU   *pstSteerOfRoamMsg = VOS_NULL_PTR;

    pstSteerOfRoamMsg = (STK_NAS_STEERING_OF_ROAMING_STRU *)pstMsg;

    /* SteeringOfRoaming的特性开关关闭不处理该消息 */
    if (VOS_FALSE == NAS_MML_GetStkSteeringOfRoamingSupportFlg())
    {
        return VOS_TRUE;
    }

    if (VOS_TRUE == NAS_MML_IsNvimOplmnAvail())
    {
        return VOS_TRUE;
    }

    /* 获取全局变量的地址 */
    pstOperPlmnInfo               = NAS_MML_GetSimOperPlmnList();
    pstOplmnAddr                  = pstOperPlmnInfo->astOperPlmnInfo;
    ucUpdateUsimForbPlmnFlg       = VOS_FALSE;

    usOplmnNum        = (VOS_UINT16)(pstSteerOfRoamMsg->usOplmnListLen / NAS_MML_PLMN_WITH_RAT_UNIT_LEN);

    if ( usOplmnNum > NAS_MML_MAX_OPERPLMN_NUM )
    {
        usOplmnNum = NAS_MML_MAX_OPERPLMN_NUM;
    }

    /* 将USIM中保存的PLMN信息转换为内存中的信息 */
    NAS_MMC_ConverPlmnInfoFromSim(pstSteerOfRoamMsg->aucOplmnList, usOplmnNum, pstOplmnAddr);

    if ( usOplmnNum > pstOperPlmnInfo->usOperPlmnNum )
    {
        /* 保存OPLMN的个数 */
        pstOperPlmnInfo->usOperPlmnNum = usOplmnNum;
    }

    for (i = 0; i < usOplmnNum; i++)
    {
        /* STK_NAS_STEERING_OF_ROAMING_IND消息中携带的OPLMN需从禁止网络中删除 */
        if (VOS_TRUE == NAS_MML_DelForbPlmn(&pstOplmnAddr[i].stPlmnId))
        {
            ucUpdateUsimForbPlmnFlg = VOS_TRUE;
        }

        NAS_MML_DelForbGprsPlmn(&pstOplmnAddr[i].stPlmnId);
    }

    /* 禁止网络列表发生改变，需要通知USIM */
    if (VOS_TRUE == ucUpdateUsimForbPlmnFlg)
    {
        NAS_MMC_SndUsimUpdateForbPlmnReq();
    }

    /* BG搜网状态机在运行,进入后处理进行消息优先级比较 */
    if (NAS_MMC_FSM_BG_PLMN_SEARCH == NAS_MMC_GetCurrFsmId())
    {
        return VOS_FALSE;
    }

    /* 初始化高优先级搜网列表 */
    NAS_MMC_InitPlmnSelectionList(NAS_MMC_PLMN_SEARCH_SCENE_HIGH_PRIO_PLMN_SEARCH,
                                  VOS_NULL_PTR,
                                  NAS_MMC_GetHighPrioPlmnList());

    /* 如果HPLMN定时器正在运行,停止该定时器,启动1S定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_HPLMN_TIMER))
    {
        /* 停止HPLMN定时器,启动10s周期性尝试高优先级搜索定时器 */
        NAS_MMC_StopTimer(TI_NAS_MMC_HPLMN_TIMER);


        /* HPLMN TIMER停止事件上报 */
        NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_STOP,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);


        /* 当前UTRAN工作模式不为TDD时候，启动周期背景搜 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH, TI_NAS_MMC_STK_STEERING_TRYING_HIGH_PRIO_PLMN_SEARCH_LEN);

        return VOS_TRUE;
    }

    /* 如果10s周期性尝试高优先级搜索定时器正在运行,先停止该定时器,后续启动1S定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH))
    {
        /* 停止10s周期性尝试高优先级搜索定时器 */
        NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH);
    }

    /* 当前UTRAN工作模式不为TDD时候，启动周期背景搜 */
    /* 当前没有BG搜网状态机及定时器在运行,则当超时进行处理,即启动1s超时定时器 */
    (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH,
                       TI_NAS_MMC_STK_STEERING_TRYING_HIGH_PRIO_PLMN_SEARCH_LEN);

    return VOS_TRUE;
}




VOS_UINT32  NAS_MMC_RcvRrMmSuspendCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{

#if ((FEATURE_ON == FEATURE_GCBS) || (FEATURE_ON == FEATURE_WCBS))

    RRMM_SUSPEND_CNF_ST                *pstSuspendCnf = VOS_NULL_PTR;

    pstSuspendCnf = (RRMM_SUSPEND_CNF_ST*)pstMsg;

    if (MMC_AS_SUSPEND_SUCCESS == pstSuspendCnf->ucResult)
    {
        /* CBS上报*/
        NAS_MMC_SndMsccMsgNoNetWorkInd();
    }

#endif

    return VOS_FALSE;
}


/* Deleted by n00355355 for User_Exp_Improve, 2015-9-19, begin */
/* NAS_MMC_RcvRrMmGetTransactionReq_PreProc移到MM处理 */
/* Deleted by n00355355 for User_Exp_Improve, 2015-9-19, end */


#if (FEATURE_ON == FEATURE_LTE)

VOS_UINT32  NAS_MMC_ProcDetachCnfSucc_PreProc(
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf
)
{
    NAS_MMC_FSM_ID_ENUM_UINT32           enFsmId;

    VOS_UINT32                           ulNeedEnableLte;
    VOS_UINT32                           ulNeedDisableLte;

    enFsmId         = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }

    ulNeedEnableLte  = VOS_FALSE;
    ulNeedDisableLte = VOS_FALSE;

    /* 封装消息，向MSCC发送DETACH结果 */
    switch (pstLmmDetachCnf->ulReqType)
    {
        case MMC_LMM_MO_DET_PS_ONLY:

#if (defined(__PS_WIN32_RECUR__)) || ( VOS_WIN32 == VOS_OS_VER )
            /* 北研会设置这个值，这里进行设置是为了保证PC测试通过 */
            NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
#endif

            /* 转发detach结果给GMM */
            NAS_MMC_SndGmmLmmMoDetachInd(pstLmmDetachCnf);

            ulNeedDisableLte = NAS_MMC_IsNeedDisableLte_DetachPs();

            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
            NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS_PS, NAS_MMC_LIMITED_SERVICE);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */
            break;

        case MMC_LMM_MO_DET_CS_ONLY:

            /* 转发detach结果给MM */
            NAS_MMC_SndMmLmmMoDetachInd(pstLmmDetachCnf);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
            NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MMC_LIMITED_SERVICE);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_DetachCs();
            break;

        case MMC_LMM_MO_DET_CS_PS:

#if (defined(__PS_WIN32_RECUR__)) || ( VOS_WIN32 == VOS_OS_VER )
            /* 北研会设置这个值，这里进行设置是为了保证PC测试通过 */
            NAS_MML_SetPsAttachAllowFlg(VOS_FALSE);
            NAS_MML_SetCsAttachAllowFlg(VOS_FALSE);
#endif

            /* 转发detach结果给MM */
            NAS_MMC_SndMmLmmMoDetachInd(pstLmmDetachCnf);

            /* 转发detach结果给GMM */
            NAS_MMC_SndGmmLmmMoDetachInd(pstLmmDetachCnf);

            ulNeedDisableLte = NAS_MMC_IsNeedDisableLte_DetachCsPs();
            ulNeedEnableLte = NAS_MMC_IsNeedEnableLte_DetachCsPs();
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
            NAS_MMC_ChangeCsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);
            NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,NAS_MMC_LIMITED_SERVICE);
            /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcDetachCnfSucc_PreProc:Unexpected detach request type!");

            break;
    }

    if (VOS_TRUE == ulNeedEnableLte)
    {
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }

    /* 当前不满足disable LTE的条件 */
    if (VOS_FALSE == ulNeedDisableLte)
    {
        /* 可以直接返回不需要继续处理 */
        return VOS_TRUE;
    }

    /* 记录本地disable LTE的标记,后续L变为从模再通知LMM,WAS,GAS */
    NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);

    NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_PS_DETACH);

    /* 清除detach 请求上下文信息 */
    NAS_MMC_InitDetachReqCtx(NAS_MMC_GetDetachReqCtxAddr());

    /* ON PLMN下直接可以处理 */
    if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
    && (NAS_MMC_FSM_L1_MAIN == enFsmId))
    {
       /* 如果当前EPS信令链接存在，则缓存构造的搜网消息 */
       if ( VOS_TRUE == NAS_MML_IsRrcConnExist())
       {
           NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE, NAS_MMC_PLMN_SEARCH_SCENE_DISABLE_LTE);

           return VOS_TRUE;
       }

       return VOS_FALSE;
    }

    /* 进入L1或其它状态机处理 */
    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_ProcDetachCnfAuthRej_PreProc(
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf
)
{
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

    enFsmId = NAS_MMC_GetCurrFsmId();
    if (NAS_FSM_BUTT <= enFsmId)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "ERROR:FsmId Error");
    }
    /* 转发detach结果给MM/GMM */
    NAS_MMC_SndMmLmmMoDetachInd(pstLmmDetachCnf);
    NAS_MMC_SndGmmLmmMoDetachInd(pstLmmDetachCnf);

    /* 后面流程中会判断CS卡无效标志，因此需要设置 */
    NAS_MML_SetSimCsRegStatus(VOS_FALSE);
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    NAS_MML_SetSimPsRegStatus(VOS_FALSE);
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */

    NAS_MMC_ChangeCsRegState(NAS_MML_REG_REGISTERED_DENIED);

    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* MMC接收到LMM模块去注册回复结果,鉴权被拒绝，服务和注册状态上报 */
    NAS_MMC_ChangePsRegState(NAS_MML_REG_REGISTERED_DENIED);
    NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS_PS,NAS_MMC_LIMITED_SERVICE);
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */


    /* ON PLMN下直接可以处理 */
    if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
    && (NAS_MMC_FSM_L1_MAIN == enFsmId))
    {
       /* 如果当前EPS信令链接存在，则缓存构造的搜网消息 */
       if ( VOS_TRUE == NAS_MML_IsRrcConnExist())
       {
            NAS_MMC_SetPsRegAdditionalAction_L1Main(NAS_MMC_ADDITIONAL_ACTION_CSPS_ANYCELL);

           return VOS_TRUE;
       }

       return VOS_FALSE;
    }

    /* 进入L1或其它状态机处理 */
    return VOS_FALSE;
}


VOS_UINT32  NAS_MMC_ProcEpsOnlyDetachCnf_PreProc(
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf
)
{

    VOS_UINT32                           ulRet;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enPsMsccServiceStatus;

    ulRet                = VOS_TRUE;

    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

    /* detach结果处理 */
    switch (pstLmmDetachCnf->ulDetachRslt)
    {
        case MMC_LMM_DETACH_RSLT_SUCCESS:

            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_ACCESS_BARED:
        case MMC_LMM_DETACH_RSLT_FAILURE:


            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_AUTH_REJ:

            /* 鉴权被拒的处理 */
            ulRet = NAS_MMC_ProcDetachCnfAuthRej_PreProc(pstLmmDetachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcEpsOnlyDetachCnf_PreProc:Unexpected detach result!");
            break;
    }

    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* 需要先更新状态变量，再上报 */

    enPsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);

    /* 获取当前PS服务状态 */
    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            enPsMsccServiceStatus,
                            pstLmmDetachCnf->ulOpId);
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */
    return ulRet;
}
VOS_UINT32  NAS_MMC_ProcCsOnlyDetachCnf_PreProc(
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf
)
{

    VOS_UINT32                           ulRet;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enCsMsccServiceStatus;


    ulRet   = VOS_TRUE;

    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

    /* detach结果处理 */
    switch (pstLmmDetachCnf->ulDetachRslt)
    {
        case MMC_LMM_DETACH_RSLT_SUCCESS:

            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_ACCESS_BARED:
        case MMC_LMM_DETACH_RSLT_FAILURE:


            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_AUTH_REJ:

            /* 鉴权被拒的处理 */
            ulRet = NAS_MMC_ProcDetachCnfAuthRej_PreProc(pstLmmDetachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcCsOnlyDetachCnf_PreProc:Unexpected detach result!");
            break;
    }
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */

    /* 获取当前CS服务状态 */
    enCsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);

    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            enCsMsccServiceStatus,
                            pstLmmDetachCnf->ulOpId);

    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */
    return ulRet;
}


VOS_UINT32  NAS_MMC_ProcCsPsDetachCnf_PreProc(
    LMM_MMC_DETACH_CNF_STRU            *pstLmmDetachCnf
)
{

    VOS_UINT32                           ulRet;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enCsMsccServiceStatus;
    NAS_MM_COM_SERVICE_STATUS_ENUM_UINT8 enPsMsccServiceStatus;

    ulRet                = VOS_TRUE;

    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* Deleted by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

    /* detach结果处理 */
    switch (pstLmmDetachCnf->ulDetachRslt)
    {
        case MMC_LMM_DETACH_RSLT_SUCCESS:

            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_ACCESS_BARED:
        case MMC_LMM_DETACH_RSLT_FAILURE:


            /* 按照detach成功进行处理 */
            ulRet = NAS_MMC_ProcDetachCnfSucc_PreProc(pstLmmDetachCnf);

            break;

        case MMC_LMM_DETACH_RSLT_AUTH_REJ:

            /* 鉴权被拒的处理 */
            ulRet = NAS_MMC_ProcDetachCnfAuthRej_PreProc(pstLmmDetachCnf);

            break;

        default:

            /* 异常打印 */
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcCsPsDetachCnf_PreProc:Unexpected detach result!");
            break;
    }
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, begin */
    /* 获取当前上报用的CS_PS服务状态 */
    enCsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_CS);
    enPsMsccServiceStatus = NAS_MMC_GetServiceStatusForMscc(NAS_MSCC_PIF_SRVDOMAIN_PS);

    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_CS,
                            enCsMsccServiceStatus,
                            pstLmmDetachCnf->ulOpId);

    NAS_MMC_SndMsccDetachCnf(NAS_MSCC_PIF_SRVDOMAIN_PS,
                            enPsMsccServiceStatus,
                            pstLmmDetachCnf->ulOpId);
    /* Add by l00305157 for Service_State_Optimize_PhaseII, 2015-02-11, end */

    return ulRet;
}


VOS_UINT32  NAS_MMC_RcvLmmServiceRsltInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                                              ulRet;
    LMM_MMC_SERVICE_RESULT_IND_STRU                        *pstSerRsltMsg = VOS_NULL_PTR;
    NAS_MML_REG_FAIL_CAUSE_ENUM_UINT16                      enCause;

    enCause       = NAS_MML_REG_FAIL_CAUSE_BUTT;

    pstSerRsltMsg = (LMM_MMC_SERVICE_RESULT_IND_STRU *)pstMsg;

    NAS_MMC_ConverLmmServiceRsltToMMLCause(pstSerRsltMsg,&enCause);

    NAS_MMC_SndMsccServRejRsltInd(NAS_MSCC_PIF_SRVDOMAIN_PS, (VOS_UINT16)enCause);

    /* 判断当前是否需要打断当前状态机 或进行消息的缓存 */
    ulRet = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    /* 需要打断当前状态机或进行消息的缓存,则表示预处理完成 */
    if ( VOS_TRUE == ulRet )
    {
        return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}



VOS_UINT32  NAS_MMC_RcvLmmEmmInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MM_INFO_IND_STRU               *pstSendMsg    = VOS_NULL_PTR;
    LMM_MMC_EMM_INFO_IND_STRU          *pstLmmEmmInfo = VOS_NULL_PTR;
    NAS_MML_OPERATOR_NAME_INFO_STRU    *pstCurrOperatorName = VOS_NULL_PTR;
    VOS_UINT8                           ucNvWriteFlg;
    NAS_MML_OPERATOR_NAME_INFO_STRU     stOldOperatorName;

    pstCurrOperatorName = NAS_MML_GetOperatorNameInfo();
    ucNvWriteFlg        = VOS_FALSE;
    PS_MEM_CPY(&stOldOperatorName, pstCurrOperatorName, sizeof(NAS_MML_OPERATOR_NAME_INFO_STRU));
    pstLmmEmmInfo = (LMM_MMC_EMM_INFO_IND_STRU *)pstMsg;

    if ((VOS_FALSE == pstLmmEmmInfo->stLteEmmInfo.bitOpDaylightTime)
     &&(VOS_FALSE == pstLmmEmmInfo->stLteEmmInfo.bitOpLocTimeZone)
     &&(VOS_FALSE == pstLmmEmmInfo->stLteEmmInfo.bitOpUniTimeLocTimeZone)
     &&(VOS_FALSE == pstLmmEmmInfo->stLteEmmInfo.bitOpLongName)
     &&(VOS_FALSE == pstLmmEmmInfo->stLteEmmInfo.bitOpShortName))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvLmmEmmInfoInd_PreProc():No userful info");
        return VOS_TRUE;
    }

    /* 申请内存 */
    pstSendMsg = (NAS_MM_INFO_IND_STRU *)PS_MEM_ALLOC(WUEPS_PID_MMC,
                                                sizeof(NAS_MM_INFO_IND_STRU));

    if( VOS_NULL_PTR == pstSendMsg )
    {
       NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvLmmEmmInfoInd_PreProc():ERROR:Memory Alloc Error.");
       return VOS_TRUE;
    }

    PS_MEM_SET(pstSendMsg, 0, sizeof(NAS_MM_INFO_IND_STRU));

    if (VOS_TRUE == pstLmmEmmInfo->stLteEmmInfo.bitOpLocTimeZone)
    {
        pstSendMsg->ucIeFlg |= NAS_MM_INFO_IE_LTZ;
        pstSendMsg->cLocalTimeZone = pstLmmEmmInfo->stLteEmmInfo.cLocTimeZone;
    }

    if (VOS_TRUE == pstLmmEmmInfo->stLteEmmInfo.bitOpUniTimeLocTimeZone)
    {
        pstSendMsg->ucIeFlg |= NAS_MM_INFO_IE_UTLTZ;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucYear    = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucYear;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucMonth   = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucMonth;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucDay     = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucDay;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucHour    = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucHour;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucMinute  = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucMinute;
        pstSendMsg->stUniversalTimeandLocalTimeZone.ucSecond  = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.ucSecond;
        pstSendMsg->stUniversalTimeandLocalTimeZone.cTimeZone = pstLmmEmmInfo->stLteEmmInfo.stTimeZoneAndTime.cTimeZone;
    }

    if (VOS_TRUE == pstLmmEmmInfo->stLteEmmInfo.bitOpDaylightTime)
    {
        pstSendMsg->ucIeFlg |= NAS_MM_INFO_IE_DST;
        pstSendMsg->ucDST = pstLmmEmmInfo->stLteEmmInfo.enDaylightSavingTime;
    }

    if (VOS_TRUE == pstLmmEmmInfo->stLteEmmInfo.bitOpLongName)
    {
        PS_MEM_CPY(pstCurrOperatorName->aucOperatorNameLong, pstLmmEmmInfo->stLteEmmInfo.aucOperatorNameLong, NAS_MML_MAX_OPER_LONG_NAME_LEN);

        ucNvWriteFlg = VOS_TRUE;
    }

    if (VOS_TRUE == pstLmmEmmInfo->stLteEmmInfo.bitOpShortName)
    {
        PS_MEM_CPY(pstCurrOperatorName->aucOperatorNameShort, pstLmmEmmInfo->stLteEmmInfo.aucOperatorNameShort, NAS_MML_MAX_OPER_SHORT_NAME_LEN);

        ucNvWriteFlg = VOS_TRUE;
    }

    if (VOS_TRUE == ucNvWriteFlg)
    {
        /* 如果消息解析后的网络名与之前保存的内容不同,需要写NV */
        if (0 != VOS_MemCmp(pstCurrOperatorName, &stOldOperatorName, sizeof(NAS_MML_OPERATOR_NAME_INFO_STRU)))
        {
            if (NV_OK != NV_Write(en_NV_Item_Network_Name_From_MM_Info,
                                  pstCurrOperatorName,
                                  sizeof(NAS_MML_OPERATOR_NAME_INFO_STRU)))
            {
                NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvLmmEmmInfoInd_PreProc:Write NV fail.");
            }
        }
    }
    if (NAS_MM_INFO_IE_NULL ==pstSendMsg->ucIeFlg)
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstSendMsg);
        return VOS_TRUE;
    }

    NAS_MMC_SndMsccMmInfo(pstSendMsg);

    PS_MEM_FREE(WUEPS_PID_MMC, pstSendMsg);

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvLmmEmcPdpStatusNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_EMC_PDP_STATUS_NOTIFY_STRU *pstEmcPdpStatus = VOS_NULL_PTR;
    NAS_MML_EQUPLMN_INFO_STRU          *pstEquPlmnInfo = VOS_NULL_PTR;
    NAS_MML_EQUPLMN_INFO_STRU           stSndEquPlmnInfo;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enCurrNetRat;

    PS_MEM_SET(&stSndEquPlmnInfo, 0, sizeof(NAS_MML_EQUPLMN_INFO_STRU));

    enCurrNetRat    = NAS_MML_GetCurrNetRatType();
    pstEmcPdpStatus = (LMM_MMC_EMC_PDP_STATUS_NOTIFY_STRU *)pstMsg;

    /* 如果紧急PDP连接激活 */
    if (VOS_TRUE == pstEmcPdpStatus->ucIsEmcPdpActive)
    {
        NAS_MML_SetEmcPdpStatusFlg(VOS_TRUE);
    }
    else
    {

        /* 清除紧急PDN连接标志 */
        NAS_MML_SetEmcPdpStatusFlg(VOS_FALSE);

        /* 卡存在并且当前主模是L模时给LMM发送EPLMN通知，删除Forb Plmn */
        if ( (VOS_TRUE == NAS_MML_GetSimPresentStatus())
          && (NAS_MML_NET_RAT_TYPE_LTE == enCurrNetRat) )
        {
            pstEquPlmnInfo = NAS_MML_GetEquPlmnList();
            PS_MEM_CPY(&stSndEquPlmnInfo, pstEquPlmnInfo, sizeof(NAS_MML_EQUPLMN_INFO_STRU));
            NAS_MMC_BuildSndLmmEquPlmnInfo(&stSndEquPlmnInfo);
            NAS_MMC_SndLmmEquPlmnReq(&stSndEquPlmnInfo);

            NAS_MMC_SndOmEquPlmn();
        }
    }

    return VOS_TRUE;
}


#endif
VOS_UINT32  NAS_MMC_RcvGmmLocalDetachInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
#if (FEATURE_ON == FEATURE_LTE)
    MMCGMM_LOCAL_DETACH_IND_STRU                           *pstLocalDetachInd;
    MMC_LMM_GU_LOCAL_DETACH_TYPE_ENUM_UINT32                ulLocalDetachType;

    pstLocalDetachInd = (MMCGMM_LOCAL_DETACH_IND_STRU *)pstMsg;

    if (NAS_MML_REG_FAIL_CAUSE_NTDTH_IMSI == pstLocalDetachInd->ulDetachCause)
    {
        ulLocalDetachType = MMC_LMM_GU_LOCAL_DET_IMSI_PAGING;

        /* 通知L GU本地imsi paging detach */
        NAS_MMC_SndLmmLocalDetachIndActionResultReq(ulLocalDetachType);
    }
#endif

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_GUT_LOCAL_DETACH);

    NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_PS, NAS_MMC_LIMITED_SERVICE);

    NAS_MMC_ChangePsRegState(NAS_MML_REG_NOT_REGISTERED_NOT_SEARCH);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvRrMmRelInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    RRMM_REL_IND_STRU                  *pstRrMmRelInd = VOS_NULL_PTR;

    VOS_UINT32                          ulCacheEventType;

    pstRrMmRelInd = (RRMM_REL_IND_STRU*)pstMsg;

    if (RRC_RRC_CONN_STATUS_ABSENT == pstRrMmRelInd->ulRrcConnStatus)
    {
        /* 清除RRC连接状态 */
        NAS_MML_SetRrcConnStatusFlg(VOS_FALSE);
    }


    /* 当前在UTRAN FDD模式下，收到WAS的异常释放清除SUSPEND IND缓存消息 */
    if (VOS_TRUE == NAS_MMC_IsCurrentWcdmaMode())
    {
        if (RRC_REL_CAUSE_UTRAN_RELEASE != pstRrMmRelInd->ulRelCause)
        {
            /* 清除缓存的SUSPEND IND 消息，此后W不上报REUME IND消息 */
            ulCacheEventType = NAS_BuildEventType(WUEPS_PID_WRR, RRMM_SUSPEND_IND);
            NAS_MMC_ClearCacheMsg(ulCacheEventType);
        }
    }


    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvMmRrConnInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{

#if (FEATURE_ON == FEATURE_LTE)
    MMCMM_RR_CONN_INFO_IND_STRU        *pstMmCsRrConnInfInd    = VOS_NULL_PTR;  /* 定义原语类型指针                         */
    VOS_UINT8                           ucCsServiceConnStatusFlg;               /* CS域业务连接是否存在,VOS_FALSE:不存在,VOS_TRUE:存在 */
    VOS_UINT8                           ucCsRrConnStatusFlg;                    /* CS域RR连接是否存在,VOS_FALSE:不存在,VOS_TRUE:存在 */
    VOS_UINT8                           ucCsEmergencyConnStatusFlg;             /* CS域紧急业务连接是否存在,VOS_FALSE:不存在,VOS_TRUE:存在 */

    pstMmCsRrConnInfInd                 = (MMCMM_RR_CONN_INFO_IND_STRU *)pstMsg;
    ucCsRrConnStatusFlg                 = VOS_FALSE;
    ucCsServiceConnStatusFlg            = NAS_MML_GetCsServiceConnStatusFlg();
    ucCsEmergencyConnStatusFlg          = VOS_FALSE;

    /* RR链接存在标志 */
    if (MMC_MM_RR_CONN_ESTED            == pstMmCsRrConnInfInd->ulRrConnFlg)
    {
        ucCsRrConnStatusFlg             = VOS_TRUE;
    }

    /* 紧急业务链路存在标志 */
    if ( (VOS_TRUE                      == NAS_MML_GetCsEmergencyServiceFlg())
      && (VOS_TRUE                      == ucCsServiceConnStatusFlg) )
    {
        ucCsEmergencyConnStatusFlg      = VOS_TRUE;
    }

    /* 通知LMM当前CS域的链接状态 */
    NAS_MMC_SndLmmCsConnStatusNotify(ucCsRrConnStatusFlg, ucCsEmergencyConnStatusFlg);

#endif

    /* 继续进状态机处理 */
    return VOS_FALSE;
}




VOS_UINT32  NAS_MMC_RcvRrMmCipherInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_CIPHER_INFO_IND_STRU          *pstCipherInfoInd;

    /* 初始化消息 */
    pstCipherInfoInd = (RRMM_CIPHER_INFO_IND_STRU*)pstMsg;

    /* 直接将收到的WAS的Cipher信息上报给MSCC */
    NAS_MMC_SndMsccWCipherInfoInd(pstCipherInfoInd);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvGmmCipherInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMC_GMM_CIPHER_INFO_IND_STRU       *pstCipherInfoInd;

    /* 初始化消息 */
    pstCipherInfoInd = (MMC_GMM_CIPHER_INFO_IND_STRU*)pstMsg;

    /* 直接将收到的WAS的Cipher信息上报给MSCC */
    NAS_MMC_SndMsccGCipherInfoInd(pstCipherInfoInd);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMmCsRegResultInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMMMC_CS_REG_RESULT_IND_STRU       *pstCsRegRslt = VOS_NULL_PTR;

    pstCsRegRslt = (MMMMC_CS_REG_RESULT_IND_STRU*)pstMsg;

    /* 对于切换流程，可能没有系统消息，做特殊处理 */
    if  ((MM_MMC_LU_RESULT_SUCCESS       == pstCsRegRslt->enLuResult)
      && (NAS_MML_REG_FAIL_CAUSE_INTER_RAT_CHANGE_SUCC == pstCsRegRslt->enRegFailCause))
    {
        NAS_MMC_ProcCsRegFailCause419InterRatChangeSucc();

        return VOS_TRUE;
    }

    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvGmmPsRegResultInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                                              ulDisablteRoamFlg;

    NAS_MML_LTE_UE_OPERATION_MODE_ENUM_UINT8                enLUeMode;
#endif
    GMMMMC_PS_REG_RESULT_IND_STRU                          *pstPsRegRslt = VOS_NULL_PTR;

    pstPsRegRslt         = (GMMMMC_PS_REG_RESULT_IND_STRU*)pstMsg;

    /* 对于切换流程，可能没有系统消息，做特殊处理 */
    if ((GMM_MMC_ACTION_RESULT_SUCCESS  == pstPsRegRslt->enActionResult)
     && (NAS_MML_REG_FAIL_CAUSE_INTER_RAT_CHANGE_SUCC == pstPsRegRslt->enRegFailCause))
    {
        NAS_MMC_ProcPsRegFailCause419InterRatChangeSucc();

        return VOS_TRUE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    enLUeMode            = NAS_MML_GetLteUeOperationMode();

    /* 由于PS detach也会发送该消息，因此如果是PS detach，不需要进行Enable的处理 */
    if (VOS_FALSE == NAS_MML_GetPsAttachAllowFlg())
    {
        return VOS_FALSE;
    }

    if ((VOS_TRUE == NAS_MMC_IsNeedEnableLte_AttachPs())
     && (NAS_MML_LTE_UE_OPERATION_MODE_CS_PS_1 != enLUeMode))
    {
        ulDisablteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisablteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

       /* 更新disable LTE能力标记 */
       NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }
#endif

    return VOS_FALSE;
}


VOS_UINT32  NAS_MMC_RcvTafOmMaintainInfoInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_OM_MAINTAIN_INFO_IND_STRU   *pstOmMaintainInfoInd;

    pstOmMaintainInfoInd = (MSCC_MMC_OM_MAINTAIN_INFO_IND_STRU*)pstMsg;

    /* 更新SDT连接标志 */
    NAS_MML_SetOmConnectFlg(pstOmMaintainInfoInd->ucOmConnectFlg);

    /* 更新PC回放使能标志 */
    NAS_MML_SetOmPcRecurEnableFlg(pstOmMaintainInfoInd->ucOmPcRecurEnableFlg);

    /* 不需进入状态机处理 */
    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_ProcUserReselReqUnNormalSrv_PreProc(
    MSCC_MMC_PLMN_USER_RESEL_REQ_STRU  *pUserReselReq
)
{
    NAS_MMC_PLMN_SEARCH_SCENE_ENUM_UINT32                   enPlmnSearchScene;
    VOS_UINT32                                              ulCsServiceExist;
    VOS_UINT32                                              ulPsServiceExist;

    VOS_UINT32                                              ulIsNeedPlmnSearch;

    NAS_MML_BG_SEARCH_CFG_INFO_STRU                        *pstBgSearchCfg = VOS_NULL_PTR;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();

    /* 判断当前是否有CS业务存在 */
    ulCsServiceExist         = NAS_MML_GetCsServiceExistFlg();

    /* 判断当前是否有PS业务存在 */
    ulPsServiceExist         = NAS_MML_GetPsServiceExistFlg();

    /* 手动改自动模式需要将首次启动hplmn定时器标志置为true */
    if ( NAS_MMC_PLMN_SELECTION_MODE_MANUAL == NAS_MMC_GetPlmnSelectionMode())
    {
        /* 法电定制，增强的Hplmn搜网特性打开时，自动模式修改为手动模式后，
           停止HPLMN定时器，用户再次将手动模式修改为自动模式后，满足HPLMN的启动条件后，
           启动HPLMN定时器，认为不是首次启动HPLMN定时器*/
        if (VOS_TRUE == pstBgSearchCfg->ucEnhancedHplmnSrchFlg)
        {
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);
        }
        else
        {
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_TRUE);
        }
    }

    /* 无业务时，不需要缓存搜网 */
    if ( (VOS_FALSE == ulCsServiceExist)
      && (VOS_FALSE == ulPsServiceExist))
    {
        /* 进状态机里触发搜网 */
        /* 当前为自动选网模式且无需搜网则向MSCC回复搜网成功 */
        ulIsNeedPlmnSearch = NAS_MMC_IsNeedPlmnSearch_UserReselReq();

        if ((NAS_MMC_PLMN_SELECTION_MODE_AUTO == NAS_MMC_GetPlmnSelectionMode())
         && (VOS_FALSE == ulIsNeedPlmnSearch))
        {
            NAS_MMC_SndMsccPlmnReselCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);
            return VOS_TRUE;
        }

        return VOS_FALSE;
    }

    /* 当前为自动选网模式 */
    if ( NAS_MMC_PLMN_SELECTION_MODE_AUTO == NAS_MMC_GetPlmnSelectionMode())
    {
        /* 记录搜网场景 */

        if (VOS_TRUE == NAS_MML_GetUserAutoReselActiveFlg())
        {
            enPlmnSearchScene = NAS_MMC_PLMN_SEARCH_SCENE_USER_AUTO_RESEL;
        }
        else
        {
            enPlmnSearchScene = NAS_MMC_PLMN_SEARCH_SCENE_AVAILABLE_TIMER_EXPIRED;
        }
    }
    else
    {
        /* 更新选网模式 */
        NAS_MMC_SetPlmnSelectionMode(NAS_MMC_PLMN_SELECTION_MODE_AUTO);

        /* 将当前重选模式写入NVIM中 */
        NAS_MMC_WritePlmnSelectionModeNvim();

        /* 模式改变调用STK相关事件 */
        if (VOS_TRUE == NAS_MMC_IsNeedSndStkSearchModeChgEvt())
        {
            NAS_MMC_SndStkSearchModeChgEvt(NAS_MMC_PLMN_SELECTION_MODE_AUTO);
        }

        /* 记录搜网场景 */

        if (VOS_TRUE == NAS_MML_GetUserAutoReselActiveFlg())
        {
            enPlmnSearchScene = NAS_MMC_PLMN_SEARCH_SCENE_USER_AUTO_RESEL;
        }
        else
        {
            enPlmnSearchScene = NAS_MMC_PLMN_SEARCH_SCENE_MANUAL_CHANGE_AUTO_MODE;
        }

    }


    /* 缓存搜网消息 */
    NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE, enPlmnSearchScene);

    /* 向MSCC回复搜网成功 */
    NAS_MMC_SndMsccPlmnReselCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);


    /* 有业务时缓存了搜网不再进入状态机处理 */
    return VOS_TRUE;
}


VOS_UINT32 NAS_MMC_ProcUserReselReqNormalServiceFun(VOS_VOID)
{
    NAS_MML_PLMN_ID_STRU               *pstCurrCampPlmnId = VOS_NULL_PTR;
    VOS_UINT32                          ulCampOnHplmnFlag;
    VOS_UINT32                          ulCsServiceExist;

    /* 取得当前驻留的PLMN ID */
    pstCurrCampPlmnId = NAS_MML_GetCurrCampPlmnId();

    /* 与HPLMN相同，返回已驻留HPLMN上 */
    ulCampOnHplmnFlag = NAS_MML_ComparePlmnIdWithHplmn(pstCurrCampPlmnId);

    /* 判断当前是否有CS业务存在 */
    ulCsServiceExist         = NAS_MML_GetCsServiceExistFlg();

    if ((VOS_FALSE == NAS_MML_GetUserAutoReselActiveFlg())
     || (VOS_TRUE  == ulCampOnHplmnFlag))
    {
        NAS_MMC_SndMsccPlmnReselCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);

        return VOS_TRUE;
    }
    else
    {
        if (VOS_TRUE == ulCsServiceExist)
        {
            /* 缓存消息，回复成功 */
            NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE,
                        NAS_MMC_PLMN_SEARCH_SCENE_USER_AUTO_RESEL);

            NAS_MMC_SndMsccPlmnReselCnf(NAS_MSCC_PIF_USER_PLMN_SEARCH_RESULT_SUCCESS);

            return VOS_TRUE;
        }
        else
        {
            /* 进状态机里触发搜网 */
            return VOS_FALSE;
        }
    }
}
VOS_UINT32  NAS_MMC_ProcUserReselReqNormalService_PreProc(
    MSCC_MMC_PLMN_USER_RESEL_REQ_STRU      *pUserReselReq
)
{
    NAS_MML_BG_SEARCH_CFG_INFO_STRU    *pstBgSearchCfg      = VOS_NULL_PTR;

    pstBgSearchCfg = NAS_MML_GetBgSearchCfg();



    /* 当前已经是自动选网模式 */
    if (NAS_MMC_PLMN_SELECTION_MODE_AUTO == NAS_MMC_GetPlmnSelectionMode())
    {
        return NAS_MMC_ProcUserReselReqNormalServiceFun();
    }

    /* 更新选网模式 */
    NAS_MMC_SetPlmnSelectionMode(NAS_MMC_PLMN_SELECTION_MODE_AUTO);

    /* 将当前重选模式写入NVIM中 */
    NAS_MMC_WritePlmnSelectionModeNvim();

    /* 搜网模式变化上报USIM */
    if (VOS_TRUE == NAS_MMC_IsNeedSndStkSearchModeChgEvt())
    {
        NAS_MMC_SndStkSearchModeChgEvt(NAS_MMC_PLMN_SELECTION_MODE_AUTO);
    }

    /* 手动模式修改为自动模式，当前是正常服务的情况下，需要判断是否需要启动HPLMN定时器 */
    if (VOS_TRUE == NAS_MMC_IsNeedStartHPlmnTimer())
    {
        /* 法电定制，增强的Hplmn搜网特性打开时，自动模式修改为手动模式后，
           停止HPLMN定时器，用户再次将手动模式修改为自动模式后，满足HPLMN的启动条件后，
           启动HPLMN定时器，认为不是首次启动HPLMN定时器*/
        if (VOS_TRUE == pstBgSearchCfg->ucEnhancedHplmnSrchFlg)
        {
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);
        }
        else
        {
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_TRUE);
        }

        /* 启动定时器 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER, NAS_MMC_GetHplmnTimerLen());


        /* HPLMN TIMER启动事件上报 */
        NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                        VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

        NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);

    }

    if (VOS_TRUE == NAS_MMC_IsNeedStartHighPrioRatHPlmnTimer())
    {
        /* 启动定时器 */
        if (VOS_TRUE == NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen()))
        {
            NAS_MMC_AddCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
            NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();
        }
    }


    return NAS_MMC_ProcUserReselReqNormalServiceFun();

}



VOS_VOID NAS_MMC_ProcScanCtrl_PreProc(
    VOS_UINT16                          usEfLen,
    VOS_UINT8                          *pucEf
)
{
    VOS_UINT32                          ulHplmnTimerLen;

    /* 如果搜索控制NV未打开，直接返回 */
    if (VOS_FALSE == NAS_MML_GetScanCtrlEnableFlg())
    {
        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcScanCtrl_PreProc:ERROR: Scan Ctrl NV is deactive.");
        return;
    }

    /* 文件长度不正确，直接返回 */
    if (NAS_MML_HPLMN_PERI_FILE_LEN != usEfLen)
    {
        NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_ProcScanCtrl_PreProc:ERROR: File length is invalid.");
        return;
    }


    /* 时长已在预处理中刷新，直接获取 */
    ulHplmnTimerLen = NAS_MMC_GetHplmnTimerLen();


    /* 如果当前高优先级搜网定时器正在运行，首先停止 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_HPLMN_TIMER))
    {
        /* 停止定时器 */
        NAS_MMC_StopTimer(TI_NAS_MMC_HPLMN_TIMER);

        /* 当前UTRAN工作模式非TDD,且时长非0，重新启动 */
        if (0 != ulHplmnTimerLen)
        {
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER, ulHplmnTimerLen);
        }
    }
    else
    {
        /* 背景搜状态机正在运行，直接返回 */
        if (NAS_MMC_FSM_BG_PLMN_SEARCH == NAS_MMC_GetCurrFsmId())
        {
            return;
        }

        /* 周期性尝试高优先级搜网的定时器正在运行，刷新后的时长为0，需要停止 */
        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH))
        {
            if (0 == ulHplmnTimerLen)
            {
                NAS_MMC_StopTimer(TI_NAS_MMC_PERIOD_TRYING_HIGH_PRIO_PLMN_SEARCH);
            }

            return;
        }

        /* 调用NAS_MMC_IsNeedStartHPlmnTimer判断是否需要启动定时器 */
        if (VOS_TRUE == NAS_MMC_IsNeedStartHPlmnTimer())
        {
            /* 启动定时器 */
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HPLMN_TIMER, NAS_MMC_GetHplmnTimerLen());

            /* HPLMN TIMER启动事件上报 */
            NAS_EventReport(WUEPS_PID_MMC, NAS_OM_EVENT_HPLMN_TIMER_START,
                            VOS_NULL_PTR, NAS_OM_EVENT_NO_PARA);

            /* 将HPLMN启动类型设置为非首次启动 */
            NAS_MMC_SetFirstStartHPlmnTimerFlg(VOS_FALSE);
        }

        if (VOS_TRUE == NAS_MMC_IsNeedStartHighPrioRatHPlmnTimer())
        {
            /* 启动定时器 */
            if (VOS_TRUE == NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen()))
            {
                NAS_MMC_AddCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
                NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();
            }
        }
    }

    return;
}


VOS_UINT32  NAS_MMC_RcvCmServiceRejectInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MMCMM_CM_SERVICE_REJECT_IND_STRU   *pstRcvMsg = VOS_NULL_PTR;
    VOS_UINT32                          ulState;

#if (FEATURE_ON == FEATURE_LTE)
    VOS_UINT32                          ulDisablteRoamFlg;
#endif
    pstRcvMsg = (MMCMM_CM_SERVICE_REJECT_IND_STRU*)pstMsg;

    ulState   = NAS_MMC_GetFsmTopState();

    /* BG搜状态机系统消息可能延迟发送，需要在注册状态和服务状态之前通知MMA */
    if ((NAS_MMC_FSM_BG_PLMN_SEARCH                         == NAS_MMC_GetCurrFsmId())
     && ((NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_REG_IND       == ulState)
      || (NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_CONN_REL_IND  == ulState)))
    {
        NAS_MMC_SndMsccSysInfo_BgPlmnSearch();
    }

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_CM_SERVICE_REJ);

    NAS_MMC_SndMsccServRejRsltInd(NAS_MSCC_PIF_SRVDOMAIN_CS,
                                (VOS_UINT16) pstRcvMsg->ulCause);

    if ( NAS_MML_REG_FAIL_CAUSE_ILLEGAL_ME == pstRcvMsg->ulCause )
    {
        NAS_MMC_ChangeServiceState(NAS_MSCC_PIF_SRVDOMAIN_CS, NAS_MMC_LIMITED_SERVICE);
        NAS_MMC_ChangeCsRegState(NAS_MML_REG_REGISTERED_DENIED);
    }

    /* Added by zwx247453 for CHR optimize, 2015-3-13 Begin */
#if (FEATURE_ON == FEATURE_PTM)
    NAS_MMC_CmServiceRejErrRecord(pstRcvMsg->ulCause, pstRcvMsg->ulServiceStatus);
#endif
    /* Added by zwx247453 for CHR optimize, 2015-3-13 End */

    if ( (NAS_MML_REG_FAIL_CAUSE_ILLEGAL_ME != pstRcvMsg->ulCause)
      && (NAS_MML_REG_FAIL_CAUSE_IMSI_UNKNOWN_IN_VLR != pstRcvMsg->ulCause) )
    {
        return VOS_TRUE;
    }


#if (FEATURE_ON == FEATURE_LTE)
    if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_CmServiceRejectInd((VOS_UINT16)pstRcvMsg->ulCause))
    {
        ulDisablteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisablteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }

#endif

    return VOS_FALSE;
}

/* Added by z00359541 for CS REG FAIL FORB LA, 2015-9-23, begin */
/*****************************************************************************
 函 数 名  : NAS_MMC_RcvCustomizedForbLaTimerExpired_PreProc
 功能描述  : 收到删除禁止LA列表定时器超时消息预处理
 输入参数  : ulEventType - 消息类型
             pstMsg      - 消息内容
 输出参数  : 无
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年9月24日
    作    者   : g00322017
    修改内容   : 新加
*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvCustomizedForbLaTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_FORB_LA_WITH_VALID_PERIOD_LIST_STRU            *pstForbLaWithValidPeriod    = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;

    NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvCustomizedForbLaTimerExpired_PreProc");
    
    pstForbLaWithValidPeriod            = NAS_MML_GetForbLaWithValidPeriodList();
    enFsmId                             = NAS_MMC_GetCurrFsmId();

    /* ON PLMN状态机时不要删除CUSTOMIZED FORB LA，因为在ON PLMN的状态机处理时还会使用，
        会在状态机处理过程中删除  */
    if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState()) 
     && (NAS_MMC_FSM_L1_MAIN == enFsmId))
    {
        return VOS_FALSE;
    }
    
    if (VOS_TRUE == NAS_MML_DeleteExpiredLaFromForbLaWithValidPeriodList(pstForbLaWithValidPeriod))
    {
        /* 禁止LA列表有变化，通知接入层 */
        NAS_MMC_SndRrMmCellSelReq(RRC_NAS_FORBLA_CHANGE);
    }
    
    /* 启动定时器 */
    NAS_MML_StartCustomizedForbLaTimer();
    
    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_RcvForbidLaTimerExpired_PreProc
 功能描述  : 收到forbidden LA定时器超时消息预处理,
             确保在timer超时后， 删除forbid LA列表
 输入参数  : ulEventType - 消息类型
             pstMsg      - 消息内容
 输出参数  : 无
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年9月24日
    作    者   : g00322017
    修改内容   : 新加
*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvForbidLaTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_SIM_FORBIDPLMN_INFO_STRU   *pstForbidPlmnInfo   = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

    enFsmId             = NAS_MMC_GetCurrFsmId();

    /* ON PLMN状态机时不要删除FORB LA，因为在ON PLMN的状态机处理时还会使用，
        会在状态机处理过程中删除  */
    if (   (NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState()) 
        && (NAS_MMC_FSM_L1_MAIN == enFsmId)   )
    {
        return VOS_FALSE;
    }
    pstForbidPlmnInfo   = NAS_MML_GetForbidPlmnInfo();
    pstForbidPlmnInfo->ucForbRegLaNum   = 0;
    pstForbidPlmnInfo->ucForbRoamLaNum   = 0;

    NAS_MMC_SndRrMmCellSelReq(RRC_NAS_FORBLA_CHANGE);

    return VOS_TRUE;
}
/* Added by z00359541 for CS REG FAIL FORB LA, 2015-9-23, end */

VOS_UINT32  NAS_MMC_RcvPeriodDeleteDisabledPlmnWithRatExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                                              ulIsNeedSndEplmn;
    VOS_UINT32                                              ulIsExistNeedDeleteDisabledPlmn;
    NAS_MML_EQUPLMN_INFO_STRU                              *pstEquPlmnInfo = VOS_NULL_PTR;
    NAS_MML_EQUPLMN_INFO_STRU                               stSndEquPlmnInfo;
    NAS_MMC_FSM_ID_ENUM_UINT32                              enCurFsmId;

    ulIsExistNeedDeleteDisabledPlmn = VOS_FALSE;
    ulIsNeedSndEplmn                = VOS_FALSE;
    PS_MEM_SET(&stSndEquPlmnInfo, 0, sizeof(stSndEquPlmnInfo));

    ulIsExistNeedDeleteDisabledPlmn = NAS_MML_DeleteExpireValidPeriodPlmnInDisabledRatPlmnList();

    /* 当前状态机ID */
    enCurFsmId = NAS_MMC_GetCurrFsmId();

    if ((NAS_MMC_L1_STA_ON_PLMN != NAS_MMC_GetFsmTopState())
     || (NAS_MMC_FSM_L1_MAIN != enCurFsmId))
    {
        return VOS_TRUE;
    }

    /* 在L1 MAIN,on plmn需要重新通知当前模等效plmn信息 */
    ulIsNeedSndEplmn = NAS_MMC_IsNeedSndEplmn();

    if ((VOS_TRUE == ulIsExistNeedDeleteDisabledPlmn)
     && (VOS_TRUE == ulIsNeedSndEplmn))
    {
#if   (FEATURE_ON == FEATURE_LTE)
        if (NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
        {
            pstEquPlmnInfo = NAS_MML_GetEquPlmnList();
            PS_MEM_CPY(&stSndEquPlmnInfo, pstEquPlmnInfo, sizeof(stSndEquPlmnInfo));
            NAS_MMC_BuildSndLmmEquPlmnInfo(&stSndEquPlmnInfo);
            NAS_MMC_SndLmmEquPlmnReq(&stSndEquPlmnInfo);
            NAS_MMC_SndOmEquPlmn();
            return VOS_TRUE;
        }
#endif

        pstEquPlmnInfo = NAS_MML_GetEquPlmnList();
        PS_MEM_CPY(&stSndEquPlmnInfo, pstEquPlmnInfo, sizeof(stSndEquPlmnInfo));

        NAS_MMC_BuildEquPlmnInfo(&stSndEquPlmnInfo);
        NAS_MMC_SndGuAsEquPlmnReq(&stSndEquPlmnInfo, NAS_MML_GetCurrNetRatType());
        NAS_MMC_SndOmEquPlmn();
        NAS_MMC_SndMsccEplmnInfoInd(&stSndEquPlmnInfo);
    }

    return VOS_TRUE;
}


#if   (FEATURE_ON == FEATURE_LTE)
VOS_UINT32  NAS_MMC_RcvMmAbortInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    MMMMC_ABORT_IND_STRU   *pstRcvMsg = VOS_NULL_PTR;
    VOS_UINT32              ulDisablteRoamFlg;

    ulDisablteRoamFlg = NAS_MML_GetDisableLteRoamFlg();

    pstRcvMsg = (MMMMC_ABORT_IND_STRU*)pstMsg;

    if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_MmAbortInd(pstRcvMsg->enAbortCause))
    {
        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisablteRoamFlg))
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
        }

        if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
        }

        /* 更新disable LTE能力标记 */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
    }

    return VOS_FALSE;
}


VOS_UINT32  NAS_MMC_RcvLmmT3402LenNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_T3402_LEN_NOTIFY_STRU      *pstT3402LenNotifyMsg = VOS_NULL_PTR;

    pstT3402LenNotifyMsg = (LMM_MMC_T3402_LEN_NOTIFY_STRU*)pstMsg;

    /* 更新T3402定时器时长 */
    NAS_MML_SetT3402Len(pstT3402LenNotifyMsg->ulT3402Len);

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvLmmEutranNotAllowNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_EUTRAN_NOT_ALLOW_NOTIFY_STRU                   *pstEutanNotAllowNotifyMsg = VOS_NULL_PTR;
    NAS_MML_PLMN_WITH_RAT_STRU                              stPlmnWithRat;
    VOS_UINT32                                              i;

    pstEutanNotAllowNotifyMsg = (LMM_MMC_EUTRAN_NOT_ALLOW_NOTIFY_STRU*)pstMsg;

    /* 将当前DISABLE的PLMN列表加入禁止接入技术列表里面 */
    if (pstEutanNotAllowNotifyMsg->ucPlmnNum > MMC_LMM_MAX_EUTRAN_NOT_ALLOW_PLMN_NUM)
    {
        pstEutanNotAllowNotifyMsg->ucPlmnNum = MMC_LMM_MAX_EUTRAN_NOT_ALLOW_PLMN_NUM;
    }

    /* 当前接入技术为LTE,PLMN ID为无效值 */
    stPlmnWithRat.enRat = NAS_MML_NET_RAT_TYPE_LTE;
    stPlmnWithRat.stPlmnId.ulMcc = NAS_MML_INVALID_MCC;
    stPlmnWithRat.stPlmnId.ulMnc = NAS_MML_INVALID_MNC;

    for (i = 0; i < pstEutanNotAllowNotifyMsg->ucPlmnNum; i++)
    {
        /* 将LMM的PLMN转换为GU类型的PLMN */
        NAS_MMC_ConvertLmmPlmnToGUNasFormat(&pstEutanNotAllowNotifyMsg->astPlmnIdList[i], &stPlmnWithRat.stPlmnId);


        /* 将转换后的PLMN加入到禁止接入技术列表中 */
        NAS_MML_AddDisabledRatPlmnWithForbiddenPeriodList(&stPlmnWithRat,
                                                      NAS_MML_ONE_SECOND_TIME_SLICES * pstEutanNotAllowNotifyMsg->ulTimerLen,
                                                      MMC_LMM_DISABLE_LTE_REASON_LMM_NOTIFY_EUTRAN_NOT_ALLOW);
    }

    /* 当前PLMN 4G不支持,如果当前没有通知接入层DISABLE,则需要通知接入层DISABLE LTE */
    if (VOS_TRUE == NAS_MMC_IsEutranNotAllowedNeedDisableL())
    {
        /* 更新disable LTE能力标记,在主模不通知DISABLE LTE */
        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);

        /* 当前LTE不是主模则通知AS去DISABLE LTE,否则等挂起LTE后再通知 */
        if (NAS_MML_NET_RAT_TYPE_LTE != NAS_MML_GetCurrNetRatType())
        {
            NAS_MMC_SndDisableLteNotify(MMC_LMM_DISABLE_LTE_REASON_LMM_NOTIFY_EUTRAN_NOT_ALLOW);

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);
        }

        NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());
        NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_LMM_NOTIFY_EUTRAN_NOT_ALLOW);

        /* 启动ENABLE 定时器 */
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER, pstEutanNotAllowNotifyMsg->ulTimerLen * NAS_MML_ONE_THOUSAND_MILLISECOND);
    }

    /* 进行FORBIDEN PLMN相关信息的LOG打印 */
    NAS_MMC_LogForbiddenPlmnRelatedInfo();

    /* 处理完毕，不需进入后续状态机处理 */
    return VOS_TRUE;
}




VOS_UINT32  NAS_MMC_RcvEnableLteExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                        *pstMsg
)
{
    VOS_UINT32              ulDisablteRoamFlg;

    NAS_MML_PLMN_RAT_PRIO_STRU          *pstPrioRatList = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32           enCurFsmId;

    pstPrioRatList = NAS_MML_GetMsPrioRatList();

    ulDisablteRoamFlg = NAS_MML_GetDisableLteRoamFlg();


    if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_EnableLteTimerExp())
    {
        /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
        if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
         && (VOS_FALSE == ulDisablteRoamFlg))
        {
            /* enable lte需要触发搜网且当前syscfg设置LTE接入技术优先级最高 */
            if ((VOS_TRUE == NAS_MML_GetEnableLteTrigPlmnSearchFlag())
             && (NAS_MML_NET_RAT_TYPE_LTE == pstPrioRatList->aucRatPrio[0]))
            {
                /* 如果存在cs业务则等cs业务结束后再enable lte，如果存在ps业务连接，
                   MMC需要主动释放连接，连接释放后再enable lte */
                if (VOS_TRUE == NAS_MML_GetCsServiceExistFlg())
                {
                    NAS_MMC_SetEnableLteTimerExpireFlag(VOS_TRUE);
                    return VOS_TRUE;
                }

                /* 当前状态机ID */
                enCurFsmId = NAS_MMC_GetCurrFsmId();

                if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
                 && (NAS_MMC_FSM_L1_MAIN == enCurFsmId))
                {
                    /* 启动选网状态机进行网络获取和注册 */
                    NAS_MMC_FSM_SetCurrState(NAS_MMC_L1_STA_INITIAL);

                    /* enable lte后需要触发搜网，发送内部搜网消息 */
                    NAS_MMC_SndInterPlmnSearchReq(NAS_MMC_PLMN_SEARCH_SCENE_ENABLE_LTE_PLMN_SEARCH,VOS_NULL_PTR, 0);
                }
            }

            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);

            if (VOS_TRUE == NAS_MMC_IsNeedRelPsConnEnableLteTimerExpired())
            {
                NAS_MMC_SndGmmRelReq();
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_BUTT);
        }
    }

    return VOS_FALSE;
}




VOS_UINT32 NAS_MMC_RcvMmCsfbAbortInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulRet;

    /* 判断当前是否需要打断当前状态机 或进行消息的缓存 */
    ulRet = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    /* 需要打断当前状态机或进行消息的缓存,则表示预处理完成 */
    if (VOS_TRUE == ulRet)
    {
        return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}

#endif
VOS_UINT32  NAS_MMC_IsNeedPlmnSearchGUSearchState_UserReselReq(
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId,
    VOS_UINT32                          ulUserAutoReselActiveFlg,
    VOS_UINT32                          ulIsCurrSearchingPlmnHplmn,
    VOS_UINT32                          ulCurrState
)
{
    /* 搜网状态机在运行，在GU搜网和挂起状态时需要判断当前正在搜索的网络是否为hplmn，nv 9705打开时，如果正在搜hplmn则无需打断 */
    if ( (NAS_MMC_FSM_PLMN_SELECTION == enFsmId)
      && (VOS_TRUE == ulUserAutoReselActiveFlg)
      && (VOS_TRUE == ulIsCurrSearchingPlmnHplmn)
      && ((NAS_MMC_PLMN_SELECTION_STA_WAIT_WAS_PLMN_SEARCH_CNF  ==  ulCurrState )
       || (NAS_MMC_PLMN_SELECTION_STA_WAIT_GAS_PLMN_SEARCH_CNF  ==  ulCurrState )
       || (NAS_MMC_PLMN_SELECTION_STA_WAIT_WAS_SUSPEND_CNF      ==  ulCurrState )
       || (NAS_MMC_PLMN_SELECTION_STA_WAIT_GAS_SUSPEND_CNF      ==  ulCurrState )))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}
VOS_UINT32 NAS_MMC_IsNeedPlmnSearch_UserReselReq(VOS_VOID)
{
    VOS_UINT32                                              ulCurrState;
    NAS_MML_PLMN_WITH_RAT_STRU                             *pstCurrSearchingPlmn = VOS_NULL_PTR;
    VOS_UINT32                                              ulIsCurrSearchingPlmnHplmn;
    VOS_UINT32                                              ulUserAutoReselActiveFlg;
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;
    VOS_UINT32                                              ulIsUsimStatusValid;
#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_PLMN_ID_STRU                                   *pstCsPsMode1ReCampLtePlmn;
    VOS_UINT32                                              ulIsReCampLtePlmnHplmn;
#endif

    enFsmId                    = NAS_MMC_GetCurrFsmId();
    ulCurrState                = NAS_MMC_GetFsmTopState();
    ulUserAutoReselActiveFlg   = NAS_MML_GetUserAutoReselActiveFlg();
    pstCurrSearchingPlmn       = NAS_MMC_GetCurrSearchingPlmn_PlmnSelection();
    ulIsCurrSearchingPlmnHplmn = NAS_MML_ComparePlmnIdWithHplmn(&pstCurrSearchingPlmn->stPlmnId);
    ulIsUsimStatusValid        = NAS_MML_IsUsimStausValid();

#if (FEATURE_ON == FEATURE_LTE)
    pstCsPsMode1ReCampLtePlmn  = NAS_MMC_GetCsPsMode1ReCampLtePlmn_PlmnSelection();
    ulIsReCampLtePlmnHplmn     = NAS_MML_ComparePlmnIdWithHplmn(pstCsPsMode1ReCampLtePlmn);
#endif


    /* 如果已经是自动模式且在搜网状态机或内部列表搜状态机或背景搜状态机无需再打断当前状态机重新搜网，支持用户重选定制的NV项9074关闭，直接回复ok */
    if ((VOS_FALSE == ulUserAutoReselActiveFlg)
     && ((NAS_MMC_FSM_PLMN_SELECTION == enFsmId)
      || (NAS_MMC_FSM_PLMN_LIST == enFsmId)
      || (NAS_MMC_FSM_BG_PLMN_SEARCH == enFsmId)))
    {
        return VOS_FALSE;
    }



    if ( VOS_FALSE == NAS_MMC_IsNeedPlmnSearchGUSearchState_UserReselReq(
                    enFsmId,ulUserAutoReselActiveFlg,ulIsCurrSearchingPlmnHplmn,ulCurrState) )
    {
        return VOS_FALSE;
    }

#if (FEATURE_ON == FEATURE_LTE)
    /* 搜网状态机在运行，在L搜网和挂起状态时需要判断当前正在搜索的网络是否为hplmn，nv 9705打开时，如果正在搜hplmn则无需打断 */
    if ((NAS_MMC_FSM_PLMN_SELECTION == enFsmId)
     && (VOS_TRUE == ulUserAutoReselActiveFlg))
    {
        if ((VOS_TRUE == ulIsCurrSearchingPlmnHplmn)
         && ((NAS_MMC_PLMN_SELECTION_STA_WAIT_LMM_PLMN_SEARCH_CNF  ==  ulCurrState )
          || (NAS_MMC_PLMN_SELECTION_STA_WAIT_LMM_SUSPEND_CNF ==  ulCurrState)))
        {
            return VOS_FALSE;
        }

        if ((VOS_TRUE ==ulIsReCampLtePlmnHplmn)
         && (NAS_MMC_PLMN_SELECTION_STA_WAIT_LMM_PLMN_SEARCH_CNF_CSPSMODE1_RECAMP_LTE == ulCurrState))
        {
            return VOS_FALSE;
        }

        return VOS_TRUE;
    }
#endif


    /* anycell状态机在运行如果当前卡无效，无需打断重新发起anycell搜 */
    if ((NAS_MMC_FSM_ANYCELL_SEARCH == enFsmId)
     && (VOS_FALSE == ulIsUsimStatusValid))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32 NAS_MMC_IsNeedPlmnSearch_Oos(
    RRMM_LIMIT_SERVICE_CAMP_IND_STRU        *pstRrmmLimitCampOnMsg
)
{
    /* 存在RRC连接,当前的操作为start,并且上报的PLMN个数不为0时,需要发起搜网 */

    if (RRC_RRC_CONN_STATUS_PRESENT != pstRrmmLimitCampOnMsg->enRrcConnStatus)
    {
        return VOS_FALSE;
    }

    if (VOS_FALSE == NAS_MML_IsUsimStausValid())
    {
        return VOS_FALSE;
    }

    if ((0 == pstRrmmLimitCampOnMsg->stPlmnIdList.ulHighPlmnNum)
     && (0 == pstRrmmLimitCampOnMsg->stPlmnIdList.ulLowPlmnNum))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



#if (FEATURE_ON == FEATURE_PTM)
VOS_UINT32 NAS_MMC_RcvAcpuOmErrLogRptReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_CHAR                           *pbuffer   = VOS_NULL_PTR;
    VOS_UINT32                          ulBufUseLen;
    VOS_UINT32                          ulRealLen;
    VOS_UINT32                          ulTotalLen;
    NAS_ERR_LOG_MNTN_EVENT_STRU         stNasErrLogMntnEvent;

    /* RING BUFFER数据长度 */
    ulBufUseLen = NAS_MML_GetErrLogRingBufUseBytes();
    ulTotalLen = ulBufUseLen + sizeof(NAS_ERR_LOG_MNTN_EVENT_STRU);

    pbuffer = (VOS_CHAR *)PS_MEM_ALLOC(WUEPS_PID_MMC, ulTotalLen);
    if (VOS_NULL_PTR == pbuffer)
    {
        /* 发送ID_OM_ERR_LOG_REPORT_CNF内容为空的消息给OM */
        NAS_MMC_SndAcpuOmErrLogRptCnf(VOS_NULL_PTR, 0);
        return VOS_TRUE;
    }

    PS_MEM_SET(pbuffer, 0, ulTotalLen);

    /* 获取RING BUFFER的内容 */
    ulRealLen = NAS_MML_GetErrLogRingBufContent(pbuffer, ulBufUseLen);
    if (ulRealLen != ulBufUseLen)
    {
        /* 发送ID_OM_ERR_LOG_REPORT_CNF内容为空的消息给OM */
        NAS_MMC_SndAcpuOmErrLogRptCnf(VOS_NULL_PTR, 0);
        PS_MEM_FREE(WUEPS_PID_MMC, pbuffer);
        return VOS_TRUE;
    }

    /* 将缓冲区溢出次数信息追加在RingBuf后面 */
    NAS_COMM_BULID_ERRLOG_HEADER_INFO(&stNasErrLogMntnEvent.stHeader,
                                      VOS_GetModemIDFromPid(WUEPS_PID_MMC),
                                      NAS_ERR_LOG_ALM_MNTN,
                                      NAS_GetErrLogAlmLevel(NAS_ERR_LOG_ALM_MNTN),
                                      VOS_GetSlice(),
                                      (sizeof(NAS_ERR_LOG_MNTN_EVENT_STRU) - sizeof(OM_ERR_LOG_HEADER_STRU)));

    stNasErrLogMntnEvent.ulCount = NAS_MML_GetErrlogOverflowCnt();

    PS_MEM_CPY(pbuffer + ulBufUseLen, &stNasErrLogMntnEvent, sizeof(stNasErrLogMntnEvent));

    /* 获取完了后需要将RINGBUFFER清空 */
    NAS_MML_CleanErrLogRingBuf();

    /* 重置溢出计数 */
    NAS_MML_SetErrlogOverflowCnt(0);

    /* 可维可测BUF溢出的勾包 */
    NAS_COM_MntnPutRingbuf(NAS_ERR_LOG_ALM_MNTN,
                           WUEPS_PID_MMC,
                           (VOS_UINT8 *)&stNasErrLogMntnEvent,
                           sizeof(stNasErrLogMntnEvent));

    /* 发送ID_OM_ERR_LOG_REPORT_CNF消息给ACPU OM */
    NAS_MMC_SndAcpuOmErrLogRptCnf(pbuffer, ulTotalLen);

    PS_MEM_FREE(WUEPS_PID_MMC, pbuffer);


    return VOS_TRUE;
}
VOS_UINT32 NAS_MMC_RcvAcpuOmErrLogCtrlInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    OM_ERROR_LOG_CTRL_IND_STRU         *pstRcvMsg = VOS_NULL_PTR;

    pstRcvMsg = (OM_ERROR_LOG_CTRL_IND_STRU*)pstMsg;

    /* 更新ERRLOG控制标识 */
    if ((VOS_FALSE == pstRcvMsg->ucAlmStatus)
     || (VOS_TRUE  == pstRcvMsg->ucAlmStatus))
    {
        NAS_MML_SetErrlogCtrlFlag(pstRcvMsg->ucAlmStatus);
    }

    if ((pstRcvMsg->ucAlmLevel >= NAS_ERR_LOG_CTRL_LEVEL_CRITICAL)
     && (pstRcvMsg->ucAlmLevel <= NAS_ERR_LOG_CTRL_LEVEL_WARNING))
    {
        NAS_MML_SetErrlogAlmLevel(pstRcvMsg->ucAlmLevel);
    }

    return VOS_TRUE;
}
VOS_UINT32 NAS_MMC_RcvAcpuOmFtmCtrlInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    OM_FTM_CTRL_IND_STRU               *pstRcvMsg = VOS_NULL_PTR;

    pstRcvMsg = (OM_FTM_CTRL_IND_STRU*)pstMsg;

    /* 更新工程菜单控制标识 */
    if ((VOS_FALSE == pstRcvMsg->ucActionFlag)
     || (VOS_TRUE  == pstRcvMsg->ucActionFlag))
    {
        NAS_MML_SetFtmCtrlFlag(pstRcvMsg->ucActionFlag);
    }

    /* 如果是关闭指示则什么都不处理 */
    if (VOS_FALSE == pstRcvMsg->ucActionFlag)
    {
        return VOS_TRUE;
    }

    /* 收到打开工程菜单的指示后，需要将当前的TMSI/PTMSI上报给上层应用 */
    NAS_MMC_SndAcpuOmCurTmsi();
    NAS_MMC_SndAcpuOmCurPtmsi();

    return VOS_TRUE;
}
#endif


VOS_UINT32  NAS_MMC_RcvMsccEOPlmnSetReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                                              ulLen;
    VOS_UINT16                                              usNewOPlmnNum;
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;
    VOS_UINT32                                              ucMmcTopState;
    VOS_UINT32                                              ulOffset;
    VOS_UINT32                                              ulOplmnNum;
    MSCC_MMC_EOPLMN_SET_REQ_STRU                            *pstEOPlmnSetMsg     = VOS_NULL_PTR;
    NAS_MML_USER_CFG_OPLMN_INFO_STRU                       *pstUserCfgOPlmnInfo = VOS_NULL_PTR;
    NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU                *pstNvimCfgOPlmnInfo = VOS_NULL_PTR;

    pstEOPlmnSetMsg = (MSCC_MMC_EOPLMN_SET_REQ_STRU *)pstMsg;

    ulLen           = 0;
    usNewOPlmnNum   = 0;

    /*写入NV*/
    pstNvimCfgOPlmnInfo = (NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU*)PS_MEM_ALLOC(
                                                      WUEPS_PID_MMC,
                                                      sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));

    if (VOS_NULL_PTR == pstNvimCfgOPlmnInfo)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccEOPlmnSetReq_PreProc():ERROR: Memory Alloc Error");
        NAS_MMC_SndMsccEOPlmnSetCnf(VOS_ERR);
        return VOS_TRUE;
    }

    PS_MEM_SET(pstNvimCfgOPlmnInfo, 0x00, sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));

    /* 读NV项en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST，失败，直接返回 */
    (VOS_VOID)NV_GetLength(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST, &ulLen);

    if (NV_OK != NV_Read(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST,
                         pstNvimCfgOPlmnInfo, ulLen))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccEOPlmnSetReq_PreProc():WARNING: read en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST Error");
        /* 填充消息 */
        NAS_MMC_SndMsccEOPlmnSetCnf(VOS_ERR);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return VOS_TRUE;
    }

    /* 设置第1组数据(index =0 )时，需要先清空NV中旧的OPLMN数据 */
    if (0 == pstEOPlmnSetMsg->ucIndex)
    {
        pstNvimCfgOPlmnInfo->usOplmnListNum = 0;
        PS_MEM_SET(pstNvimCfgOPlmnInfo->aucOPlmnList, 0x00, NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_EXTEND_LEN);
    }

    /* 设置新的分组数据时，需要检查前面的分组数据是否设置成功，只支持连续设置 */
    if ((NAS_MML_MAX_GROUP_CFG_OPLMN_NUM * pstEOPlmnSetMsg->ucIndex) != pstNvimCfgOPlmnInfo->usOplmnListNum )
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccEOPlmnSetReq_PreProc():WARNING: check old index data Error");
        /* 填充消息 */
        NAS_MMC_SndMsccEOPlmnSetCnf(VOS_ERR);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return VOS_TRUE;
    }

    /* 设置后新的OPLMN个数不能大于256个 */
    usNewOPlmnNum = pstNvimCfgOPlmnInfo->usOplmnListNum + (VOS_UINT16)pstEOPlmnSetMsg->ucOPlmnCount;
    if (NAS_MML_MAX_USER_CFG_OPLMN_NUM < usNewOPlmnNum)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccEOPlmnSetReq_PreProc():WARNING: check max oplmn num Error");
        /* 填充消息 */
        NAS_MMC_SndMsccEOPlmnSetCnf(VOS_ERR);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return VOS_TRUE;
    }


    PS_MEM_CPY(pstNvimCfgOPlmnInfo->aucVersion,
                pstEOPlmnSetMsg->aucVersion,
                NAS_MML_MAX_USER_OPLMN_VERSION_LEN * sizeof(VOS_INT8));

    /* 将分组OPLMN数据保存到对应的分组位置 */
    ulOffset    = NAS_MML_OPLMN_WITH_RAT_UNIT_LEN * pstNvimCfgOPlmnInfo->usOplmnListNum;
    ulOplmnNum  = NAS_MML_OPLMN_WITH_RAT_UNIT_LEN * pstEOPlmnSetMsg->ucOPlmnCount;

    if (NAS_MMC_NVIM_MAX_USER_CFG_OPLMN_DATA_EXTEND_LEN >= (ulOffset + ulOplmnNum))
    {

        PS_MEM_CPY(pstNvimCfgOPlmnInfo->aucOPlmnList + ulOffset,
                    pstEOPlmnSetMsg->aucOPlmnWithRat,
                    ulOplmnNum);
    }

    pstNvimCfgOPlmnInfo->usOplmnListNum = usNewOPlmnNum;

    if (NV_OK != NV_Write(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST,
                          pstNvimCfgOPlmnInfo,
                          sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU)))
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccEOPlmnSetReq_PreProc():ERROR: Write NV Failed");
        NAS_MMC_SndMsccEOPlmnSetCnf(VOS_ERR);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return VOS_TRUE;
    }

    /*NV保存成功后即可通知TAF*/
    NAS_MMC_SndMsccEOPlmnSetCnf(VOS_OK);

    enFsmId         = (VOS_UINT8)NAS_MMC_GetCurrFsmId();
    ucMmcTopState   = (VOS_UINT8)NAS_MMC_GetFsmTopState();

    /*关机状态下返回无效值,不需要通知MMC刷新OPLMN*/
    if ( ( NAS_MMC_FSM_L1_MAIN == enFsmId )
      && ( NAS_MMC_L1_STA_NULL == ucMmcTopState ))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
        return VOS_TRUE;
    }

    /*更新NV全局变量*/
    pstUserCfgOPlmnInfo                 = NAS_MML_GetUserCfgOPlmnInfo();

    pstUserCfgOPlmnInfo->usOplmnListNum = usNewOPlmnNum;
    PS_MEM_CPY(pstUserCfgOPlmnInfo->aucVersion,
                pstEOPlmnSetMsg->aucVersion,
                NAS_MML_MAX_USER_OPLMN_VERSION_LEN * sizeof(VOS_INT8));


    PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);

    /*发送MMC内部OPLMN刷新请求*/
    NAS_MMC_SndInterNvimOPlmnRefreshInd();
    return VOS_TRUE;

}


VOS_UINT32 NAS_MMC_RcvMmcInterNvimOPlmnRefreshInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_USER_CFG_OPLMN_INFO_STRU                        *pstUserCfgOPlmnInfo = VOS_NULL_PTR;
    NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU                 *pstNvimCfgOPlmnInfo = VOS_NULL_PTR;
    NAS_MML_SIM_OPERPLMN_INFO_STRU                          *pstOperPlmnInfo = VOS_NULL_PTR;
    VOS_UINT32                                               ulEfLen;
    VOS_UINT8                                               *pucEf = VOS_NULL_PTR;
    VOS_UINT32                                               ulLen;
    VOS_UINT32                                               ulRet;
    VOS_UINT32                                               ulFileId;

    ulEfLen = 0;
    ulLen   = 0;
    ulRet   = 0;

    /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
    if (NAS_MML_SIM_TYPE_USIM == NAS_MML_GetSimType())
    {
        ulFileId = USIMM_USIM_EFOPLMNWACT_ID;
    }
    else
    {
        ulFileId = USIMM_GSM_EFOPLMNWACT_ID;
    }
    /* Modified by h00313353 for iteration 9, 2015-2-9, end */

    /* 更新MMC中的OPLMN*/
    pstUserCfgOPlmnInfo = NAS_MML_GetUserCfgOPlmnInfo();

    if (VOS_TRUE == NAS_MML_IsNvimOplmnAvail())
    {
        pstNvimCfgOPlmnInfo = (NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU*)PS_MEM_ALLOC(
                                                          WUEPS_PID_MMC,
                                                          sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));

        if (VOS_NULL_PTR == pstNvimCfgOPlmnInfo)
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMmcInterNvimOPlmnRefreshInd_PreProc():ERROR: Memory Alloc Error");
            return VOS_TRUE;
        }

        PS_MEM_SET(pstNvimCfgOPlmnInfo, 0x00, sizeof(NAS_MMC_NVIM_USER_CFG_OPLMN_EXTEND_STRU));

        (VOS_VOID)NV_GetLength(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST, &ulLen);

        /* 读NV项en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST，失败，直接返回 */
        if (NV_OK != NV_Read(en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST,
                             pstNvimCfgOPlmnInfo, ulLen))
        {
            NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMmcInterNvimOPlmnRefreshInd_PreProc():WARNING: read en_NV_Item_USER_CFG_OPLMN_EXTEND_LIST Error");
            PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);
            return VOS_TRUE;
        }

        NAS_MMC_UpdateOPlmnInfoFromNvim(pstNvimCfgOPlmnInfo->aucOPlmnList, pstUserCfgOPlmnInfo->usOplmnListNum);
        PS_MEM_FREE(WUEPS_PID_MMC, pstNvimCfgOPlmnInfo);

    }
    else /* 如果当前的NV无效，则需要把卡内的信息读上来*/
    {
        /* 卡内的信息更新上来*/
        ulRet = NAS_USIMMAPI_IsServiceAvailable(USIM_SVR_OPLMN_SEL_WACT);

        if ( PS_USIM_SERVICE_AVAILIABLE == ulRet)
        {
            /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
            /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
            if (USIMM_API_SUCCESS != NAS_USIMMAPI_GetCachedFile(ulFileId, &ulEfLen, &pucEf, USIMM_GUTL_APP))
            /* Modified by h00313353 for iteration 9, 2015-2-9, end */
            /* Modified by h00313353 for iteration 9, 2015-2-9, end */
            {
                NAS_WARNING_LOG(WUEPS_PID_MMC,"NAS_MMC_RcvMmcInterNvimOPlmnRefreshInd_PreProc():ERROR:READ FILE FAIL!");

                /* 获取全局变量的地址 */
                pstOperPlmnInfo = NAS_MML_GetSimOperPlmnList();

                /* 初始化全局变量中的OPLMN信息 */
                NAS_MML_InitSimOperPlmnInfo(pstOperPlmnInfo);
            }
            else
            {
                /* Modified by h00313353 for iteration 9, 2015-2-9, begin */
                NAS_MMC_SndOmGetCacheFile(ulFileId, ulEfLen, pucEf);

                /* 读取文件成功后，MMC内部做相应处理 */
                NAS_MMC_ProcAgentUsimReadFileCnf_PreProc((VOS_UINT16)ulFileId, (VOS_UINT16)ulEfLen, pucEf);
                /* Modified by h00313353 for iteration 9, 2015-2-9, end */
            }
        }
        else /* 如果卡文件也不可用，则清空数据*/
        {
            /* 获取全局变量的地址 */
            pstOperPlmnInfo = NAS_MML_GetSimOperPlmnList();

            /* 初始化全局变量中的OPLMN信息 */
            NAS_MML_InitSimOperPlmnInfo(pstOperPlmnInfo);

        }
    }

    /* 更新高优先级搜网策略*/
    return NAS_MMC_ProHighPrioPlmnRefreshInd_PreProc();
}
VOS_UINT32 NAS_MMC_RcvMmCmServiceInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCMM_CM_SERVICE_IND_STRU          *pstCmServiceInd = VOS_NULL_PTR;
    VOS_UINT8                           ucCsServiceConnStatusFlag;

    VOS_UINT8                           ucEnableLteTimerExpFlag;
    NAS_MML_PLMN_RAT_PRIO_STRU         *pstPrioRatList = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32          enCurFsmId;
    VOS_UINT32                          ulState;

    pstPrioRatList          = NAS_MML_GetMsPrioRatList();
    ucEnableLteTimerExpFlag = NAS_MMC_GetEnableLteTimerExpireFlag();

    pstCmServiceInd           = (MMCMM_CM_SERVICE_IND_STRU *)pstMsg;
    ucCsServiceConnStatusFlag = NAS_MML_GetCsServiceConnStatusFlg();

    if ((MM_CS_SERV_EXIST == pstCmServiceInd->ulCsServFlg)
     && (VOS_TRUE == ucCsServiceConnStatusFlag))
    {
        NAS_MMC_SndMsccCsServiceConnStatusInd(VOS_TRUE);
        return VOS_FALSE;
    }

    if (MM_CS_SERV_NOT_EXIST == pstCmServiceInd->ulCsServFlg)
    {
        NAS_MMC_SndMsccCsServiceConnStatusInd(VOS_FALSE);

        /* 如果之前enable lte定时器超时标识为true，cs业务结束则需要enable
           lte，触发搜网*/
#if (FEATURE_ON == FEATURE_LTE)
        if (VOS_TRUE == ucEnableLteTimerExpFlag)
        {
            NAS_MMC_SndEnableLteNotify();
            NAS_MMC_ClearDisabledLtePlmnId();
            NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);
            NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_BUTT);

            /* 当前状态机ID */
            enCurFsmId = NAS_MMC_GetCurrFsmId();
            ulState    = NAS_MMC_GetFsmTopState();

            /* enable lte后需要触发搜网且syscfg设置LTE接入技术优先级最高，发送内部搜网消息 */
            if ((VOS_TRUE == NAS_MML_GetEnableLteTrigPlmnSearchFlag())
             && (NAS_MML_NET_RAT_TYPE_LTE == pstPrioRatList->aucRatPrio[0])
             && (NAS_MMC_L1_STA_ON_PLMN == ulState)
             && (NAS_MMC_FSM_L1_MAIN == enCurFsmId))
            {
                /* 启动选网状态机进行网络获取和注册 */
                NAS_MMC_FSM_SetCurrState(NAS_MMC_L1_STA_INITIAL);
                NAS_MMC_SndInterPlmnSearchReq(NAS_MMC_PLMN_SEARCH_SCENE_ENABLE_LTE_PLMN_SEARCH,VOS_NULL_PTR, 0);
            }
        }
#endif
    }

    return VOS_FALSE;
}




VOS_UINT32 NAS_MMC_RcvMmSimAuthFail_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCMM_SIM_AUTH_FAIL_IND_STRU       *pstSimAuthFail;

    pstSimAuthFail  = (MMCMM_SIM_AUTH_FAIL_IND_STRU *)pstMsg;

    NAS_MMC_SndMsccUsimAuthFailInd(pstSimAuthFail->enSrvDomain, pstSimAuthFail->enSimAuthFailValue);

    return VOS_TRUE;
}


VOS_UINT32 NAS_MMC_RcvGmmSimAuthFail_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCGMM_SIM_AUTH_FAIL_IND_STRU      *pstSimAuthFail;

    pstSimAuthFail  = (MMCGMM_SIM_AUTH_FAIL_IND_STRU *)pstMsg;

    NAS_MMC_SndMsccUsimAuthFailInd(pstSimAuthFail->enSrvDomain, pstSimAuthFail->enSimAuthFailValue);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_IsNetScanConflictWithFsmId_PreProc(
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId
)
{
    /* 内部列表搜、BG搜、异系统时候直接回复ERROR */
    if ( (NAS_MMC_FSM_PLMN_LIST             == enFsmId)
      || (NAS_MMC_FSM_BG_PLMN_SEARCH        == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_OOS         == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_HO          == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_CCO         == enFsmId)
      || (NAS_MMC_FSM_INTER_SYS_CELLRESEL   == enFsmId) )
    {
       return VOS_TRUE;
    }

    return VOS_FALSE;

}
VOS_UINT32 NAS_MMC_IsNetRatTypeWcdmaOrGsm(
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enRat
)
{
    /* 当前时WCDMA时候，返回VOS_TRUE */
    if ( (NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
      && (NAS_MML_NET_RAT_TYPE_WCDMA   == enRat) )
    {
        return VOS_TRUE;
    }

    /* 当前是GSM时候，返回VOS_TRUE */
    if ( NAS_MML_NET_RAT_TYPE_GSM == enRat )
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_IsEnableEndNetScanReq_PreProc(
    NAS_MSCC_PIF_NET_SCAN_CAUSE_ENUM_UINT8  *penCause
)
{
    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;
    VOS_UINT32                                              ulCsServiceExistFlg;
    VOS_UINT32                                              ulPsServiceExistFlg;
    NAS_MMC_SPEC_PLMN_SEARCH_STATE_ENUM_UINT8               enSpecPlmnSearchState;

    ulCsServiceExistFlg   = NAS_MML_GetCsServiceExistFlg();
    ulPsServiceExistFlg   = NAS_MML_GetPsServiceExistFlg();
    enSpecPlmnSearchState = NAS_MMC_GetSpecPlmnSearchState();

    /* 接入模式不对，直接返回VOS_TRUE */
    if ( VOS_FALSE == NAS_MMC_IsNetRatTypeWcdmaOrGsm(NAS_MML_GetCurrNetRatType()) )
    {
        *penCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_RAT_TYPE_ERROR;

        return VOS_TRUE;
    }

    /* 存在业务链接，给MSCC模块回复结果失败，原因值冲突 */
    if ( (VOS_TRUE == ulCsServiceExistFlg)
      || (VOS_TRUE == ulPsServiceExistFlg) )
    {
        *penCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_SERVICE_EXIST;

        return VOS_TRUE;
    }

    enFsmId = NAS_MMC_GetCurrFsmId();

    /* 内部列表搜、BG搜、异系统时候直接回复ERROR */
    if ( VOS_TRUE == NAS_MMC_IsNetScanConflictWithFsmId_PreProc(enFsmId) )
    {
        *penCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_CONFLICT;

        return VOS_TRUE;
    }

    /* 当前在搜网状态或者OOC时候，有卡，则认为未驻留，返回ERROR */
    if ( ( (NAS_MMC_L1_STA_OOC  == NAS_MMC_GetFsmTopState())
         && (NAS_MMC_FSM_L1_MAIN == enFsmId) )
      || (NAS_MMC_SPEC_PLMN_SEARCH_RUNNING == enSpecPlmnSearchState) )
    {

        if (VOS_TRUE == NAS_MML_GetSimPresentStatus())
        {
            *penCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_NOT_CAMPED;

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_IsNeedStartPeriodicNetScanTimer_PreProc(VOS_VOID)
{

    NAS_MMC_FSM_ID_ENUM_UINT32                              enFsmId;
    NAS_MMC_SPEC_PLMN_SEARCH_STATE_ENUM_UINT8               enSpecPlmnSearchState;

    enFsmId               = NAS_MMC_GetCurrFsmId();
    enSpecPlmnSearchState = NAS_MMC_GetSpecPlmnSearchState();

    /* 当前在搜网状态或者OOC时候，无卡，启动周期性定时器 */
    if ( ( (NAS_MMC_L1_STA_OOC  == NAS_MMC_GetFsmTopState())
         && (NAS_MMC_FSM_L1_MAIN == enFsmId) )
      || (NAS_MMC_SPEC_PLMN_SEARCH_RUNNING == enSpecPlmnSearchState) )
    {
        if (VOS_FALSE == NAS_MML_GetSimPresentStatus())
        {
            return VOS_TRUE;
        }
    }

    /* 信令链接存在，则启动周期性定时器 */
    if (VOS_TRUE == NAS_MML_IsRrcConnExist())
    {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_VOID NAS_MMC_ProcessNetScanReq_PreProc(
    MSCC_MMC_NET_SCAN_REQ_STRU          *pstNetScanReq
)
{
    NAS_MSCC_PIF_NET_SCAN_CAUSE_ENUM_UINT8   enCause;
    MMC_MSCC_NET_SCAN_CNF_STRU           stNetScanCnf;

    /* 当前是否需要直接给MSCC回复 */
    if ( VOS_TRUE == NAS_MMC_IsEnableEndNetScanReq_PreProc(&enCause) )
    {
        PS_MEM_SET(&stNetScanCnf, 0X00, sizeof(MMC_MSCC_NET_SCAN_CNF_STRU));

        stNetScanCnf.enResult = NAS_MSCC_PIF_NET_SCAN_RESULT_FAILURE;
        stNetScanCnf.enCause  = enCause;
        NAS_MMC_SndMsccNetScanCnf(&stNetScanCnf);

        return;
    }

    /* 当前是否需要启动周期性扫描定时器 */
    if ( VOS_TRUE == NAS_MMC_IsNeedStartPeriodicNetScanTimer_PreProc() )
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER, TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER_LEN);

        return;
    }

    /* 向GUAS发送NETSCAN请求 */
    NAS_MMC_SndAsNetScanReq(NAS_MML_GetCurrNetRatType(), pstNetScanReq);

    return;
}
VOS_UINT32  NAS_MMC_RcvMsccNetScanReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_NET_SCAN_REQ_STRU          *pstNetScanReq       = VOS_NULL_PTR;

    pstNetScanReq = (MSCC_MMC_NET_SCAN_REQ_STRU*)pstMsg;

    /* 保存MSCC模块发送过来的NETSCAN请求，周期性定时器超时后，需使用 */
    NAS_MMC_SetNetScanReq(pstNetScanReq);

    NAS_MMC_ProcessNetScanReq_PreProc(pstNetScanReq);

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvMsccAbortNetScanReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMC_MSCC_ABORT_NET_SCAN_CNF_STRU     stAbortNetScanCnf;

    PS_MEM_SET(&stAbortNetScanCnf, 0x00, sizeof(stAbortNetScanCnf));

    if ( NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER) )
    {
        NAS_MMC_StopTimer(TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER);

        /* 给MSCC发送空的扫描结果 */
        NAS_MMC_SndMsccAbortNetScanCnf(&stAbortNetScanCnf);
    }
    else
    {
        if ( VOS_TRUE == NAS_MMC_IsNetRatTypeWcdmaOrGsm(NAS_MML_GetCurrNetRatType()) )
        {
            /* 给接入层发送停止扫描请求 */
            NAS_MMC_SndAsNetScanStopReq(NAS_MML_GetCurrNetRatType());
        }
        else
        {
            NAS_WARNING_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccAbortNetScanReq_PreProc:rcv unexpected abort net scan req!");
        }
    }

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvPeriodicNetScanExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_NET_SCAN_REQ_STRU          *pstNetScanReq = VOS_NULL_PTR;

    /* 获取已经保存的NETSCAN REQ */
    pstNetScanReq         = NAS_MMC_GetNetScanReq();

    NAS_MMC_ProcessNetScanReq_PreProc(pstNetScanReq);

    return VOS_TRUE;
}
VOS_VOID NAS_MMC_ConvertNetScanRrcBandToNasFormat(
    VOS_UINT32                          ulBand,
    NAS_MML_MS_BAND_INFO_STRU          *pstNasBand
)
{
    MSCC_MMC_NET_SCAN_REQ_STRU          *pstReq              = VOS_NULL_PTR;

    /* 获取保存的NETSCAN请求 */
    pstReq                              = NAS_MMC_GetNetScanReq();

    if ( NAS_MML_NET_RAT_TYPE_GSM == pstReq->ucRat )
    {
        pstNasBand->unGsmBand.ulBand    = ulBand;

        return;
    }

    pstNasBand->unWcdmaBand.ulBand      = ulBand;

    return;
}
VOS_VOID NAS_MMC_ConvertRrcCauseToNasFormat(
    RRC_NAS_NET_SCAN_CAUSE_ENUM_UINT8   enRrcCause,
    NAS_MSCC_PIF_NET_SCAN_CAUSE_ENUM_UINT8  *penMmcCause
)
{
    switch ( enRrcCause )
    {
        case RRC_NAS_NET_SCAN_CAUSE_SIGNAL_EXIST :
            *penMmcCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_SIGNAL_EXIST;
            break;

        case RRC_NAS_NET_SCAN_CAUSE_STATE_NOT_ALLOWED :
            *penMmcCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_STATE_NOT_ALLOWED;
            break;

        case RRC_NAS_NET_SCAN_CAUSE_FREQ_LOCK :
            *penMmcCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_FREQ_LOCK;
            break;

        case RRC_NAS_NET_SCAN_CAUSE_PARA_ERROR :
            *penMmcCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_PARA_ERROR;
            break;

        default:
            *penMmcCause = NAS_MSCC_PIF_NET_SCAN_CAUSE_BUTT;
            break;
    }

    return;
}



VOS_VOID NAS_MMC_ConvertRrcNetScanCnfToNasFormat(
    RRMM_NET_SCAN_CNF_STRU             *pstRrmmNetScanCnfMsg,
    MMC_MSCC_NET_SCAN_CNF_STRU          *pstNetScanCnf
)
{
    VOS_UINT32                          i;

    pstNetScanCnf->enResult             = pstRrmmNetScanCnfMsg->enResult;
    pstNetScanCnf->ucFreqNum            = pstRrmmNetScanCnfMsg->ucFreqNum;

    NAS_MMC_ConvertRrcCauseToNasFormat(pstRrmmNetScanCnfMsg->enCause, &pstNetScanCnf->enCause);

    if ( NAS_MSCC_PIF_NET_SCAN_MAX_FREQ_NUM < pstNetScanCnf->ucFreqNum )
    {
        pstNetScanCnf->ucFreqNum        = NAS_MSCC_PIF_NET_SCAN_MAX_FREQ_NUM;
    }

    for ( i = 0 ; i < pstNetScanCnf->ucFreqNum; i++ )
    {
        pstNetScanCnf->astNetScanInfo[i].usArfcn        = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usArfcn;
        pstNetScanCnf->astNetScanInfo[i].usC1           = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usC1;
        pstNetScanCnf->astNetScanInfo[i].usC2           = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usC2;
        pstNetScanCnf->astNetScanInfo[i].usLac          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usLac;
        pstNetScanCnf->astNetScanInfo[i].ulMcc          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulMcc;
        pstNetScanCnf->astNetScanInfo[i].ulMnc          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulMnc;
        pstNetScanCnf->astNetScanInfo[i].usBsic         = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usBsic;
        pstNetScanCnf->astNetScanInfo[i].sRxlev         = pstRrmmNetScanCnfMsg->astNetScanInfo[i].sRxlev;
        pstNetScanCnf->astNetScanInfo[i].sRssi          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].sRssi;
        pstNetScanCnf->astNetScanInfo[i].ulCellId       = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulCellId;

        NAS_MMC_ConvertNetScanRrcBandToNasFormat(pstRrmmNetScanCnfMsg->astNetScanInfo[i].aulBand[0],
                                          &pstNetScanCnf->astNetScanInfo[i].stBand);
    }

    return;
}




VOS_UINT32  NAS_MMC_RcvRrMmNetScanCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_NET_SCAN_CNF_STRU             *pstRcvRrmmMsg       = VOS_NULL_PTR;
    MMC_MSCC_NET_SCAN_CNF_STRU           stNetScanCnf;

    pstRcvRrmmMsg = (RRMM_NET_SCAN_CNF_STRU *)pstMsg;

    PS_MEM_SET(&stNetScanCnf, 0x00, sizeof(stNetScanCnf));

    /* 存在信令链接的时候需要启动周期尝试定时器 */
    if ( (RRC_NAS_NET_SCAN_RESULT_FAILURE     == pstRcvRrmmMsg->enResult)
      && (RRC_NAS_NET_SCAN_CAUSE_SIGNAL_EXIST == pstRcvRrmmMsg->enCause) )
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER, TI_NAS_MMC_PERIODIC_NET_SCAN_TIMER_LEN);

        return VOS_TRUE;
    }

    NAS_MMC_ConvertRrcNetScanCnfToNasFormat(pstRcvRrmmMsg, &stNetScanCnf);
    NAS_MMC_SndMsccNetScanCnf(&stNetScanCnf);

    return VOS_TRUE;
}
VOS_VOID NAS_MMC_ConvertRrcNetScanStopCnfToNasFormat(
    RRMM_NET_SCAN_STOP_CNF_STRU        *pstRrmmNetScanCnfMsg,
    MMC_MSCC_ABORT_NET_SCAN_CNF_STRU    *pstNetScanCnf
)
{
    VOS_UINT32                          i;

    pstNetScanCnf->ucFreqNum = pstRrmmNetScanCnfMsg->ucFreqNum;

    if ( NAS_MSCC_PIF_NET_SCAN_MAX_FREQ_NUM < pstNetScanCnf->ucFreqNum )
    {
        pstNetScanCnf->ucFreqNum = NAS_MSCC_PIF_NET_SCAN_MAX_FREQ_NUM;
    }

    for ( i = 0 ; i < pstNetScanCnf->ucFreqNum; i++ )
    {
        pstNetScanCnf->astNetScanInfo[i].usArfcn        = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usArfcn;
        pstNetScanCnf->astNetScanInfo[i].usC1           = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usC1;
        pstNetScanCnf->astNetScanInfo[i].usC2           = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usC2;
        pstNetScanCnf->astNetScanInfo[i].usLac          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usLac;
        pstNetScanCnf->astNetScanInfo[i].ulMcc          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulMcc;
        pstNetScanCnf->astNetScanInfo[i].ulMnc          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulMnc;
        pstNetScanCnf->astNetScanInfo[i].usBsic         = pstRrmmNetScanCnfMsg->astNetScanInfo[i].usBsic;
        pstNetScanCnf->astNetScanInfo[i].sRxlev         = pstRrmmNetScanCnfMsg->astNetScanInfo[i].sRxlev;
        pstNetScanCnf->astNetScanInfo[i].sRssi          = pstRrmmNetScanCnfMsg->astNetScanInfo[i].sRssi;
        pstNetScanCnf->astNetScanInfo[i].ulCellId       = pstRrmmNetScanCnfMsg->astNetScanInfo[i].ulCellId;

        NAS_MMC_ConvertNetScanRrcBandToNasFormat(pstRrmmNetScanCnfMsg->astNetScanInfo[i].aulBand[0],
                                          &pstNetScanCnf->astNetScanInfo[i].stBand);
    }

    return;
}



VOS_UINT32  NAS_MMC_RcvRrMmNetScanStopCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_NET_SCAN_STOP_CNF_STRU        *pstRcvRrmmMsg       = VOS_NULL_PTR;
    MMC_MSCC_ABORT_NET_SCAN_CNF_STRU     stAbortNetScanCnf;

    pstRcvRrmmMsg = (RRMM_NET_SCAN_STOP_CNF_STRU *)pstMsg;

    PS_MEM_SET(&stAbortNetScanCnf, 0x00, sizeof(MMC_MSCC_ABORT_NET_SCAN_CNF_STRU));

    /* 将接入层上报的停止扫描结果转换成NAS的结构 */
    NAS_MMC_ConvertRrcNetScanStopCnfToNasFormat(pstRcvRrmmMsg, &stAbortNetScanCnf);

    /* 给MSCC模块转发接入层上报的STOP CNF请求 */
    NAS_MMC_SndMsccAbortNetScanCnf(&stAbortNetScanCnf);

    return VOS_TRUE;
}



VOS_UINT32  NAS_MMC_RcvGasNcellMonitorInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRC_NAS_NCELL_MONITOR_IND_STRU     *pstNcellMonitorInd;

    pstNcellMonitorInd = (RRC_NAS_NCELL_MONITOR_IND_STRU*)pstMsg;

    if ( RRC_NAS_NCELL_STATE_3G == pstNcellMonitorInd->enNcellState)
    {
        NAS_MML_SetRrcUtranNcellExistFlg(VOS_TRUE);
        NAS_MML_SetRrcLteNcellExistFlg(VOS_FALSE);
    }
    else if ( RRC_NAS_NCELL_STATE_4G == pstNcellMonitorInd->enNcellState)
    {
        NAS_MML_SetRrcLteNcellExistFlg(VOS_TRUE);
        NAS_MML_SetRrcUtranNcellExistFlg(VOS_FALSE);
    }
    else if ( RRC_NAS_NCELL_STATE_3G4G == pstNcellMonitorInd->enNcellState)
    {
        NAS_MML_SetRrcUtranNcellExistFlg(VOS_TRUE);
        NAS_MML_SetRrcLteNcellExistFlg(VOS_TRUE);
    }
    else
    {
        NAS_MML_SetRrcUtranNcellExistFlg(VOS_FALSE);
        NAS_MML_SetRrcLteNcellExistFlg(VOS_FALSE);
    }


    return VOS_TRUE;
}



VOS_UINT32 NAS_MMC_RcvMsccImsVoiceCapInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_IMS_VOICE_CAP_NOTIFY_STRU     *pstImsVoiceInd = VOS_NULL_PTR;
#if (FEATURE_ON == FEATURE_LTE)
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;
#endif

    pstImsVoiceInd = (MSCC_MMC_IMS_VOICE_CAP_NOTIFY_STRU *)pstMsg;

    /* 与之前相比,如果改变时通知LMM当前IMS VOICE是否可用 */
    if (NAS_MML_GetImsVoiceAvailFlg() != pstImsVoiceInd->ucAvail)
    {
#if (FEATURE_ON == FEATURE_LTE)
        NAS_MMC_SndLmmImsVoiceCapChangeNtf(pstImsVoiceInd->ucAvail);
#endif

        /* 存储IMS是否可用信息到MML的全局变量中 */
        NAS_MML_SetImsVoiceAvailFlg(pstImsVoiceInd->ucAvail);
    }

#if (FEATURE_ON == FEATURE_LTE)
    NAS_MML_SetPersistentBearerState(pstImsVoiceInd->ucIsExistPersistentBearer);

    /* ON PLMN下直接可以处理 */
    enFsmId = NAS_MMC_GetCurrFsmId();

    if ((NAS_MMC_L1_STA_ON_PLMN == NAS_MMC_GetFsmTopState())
     && (NAS_MMC_FSM_L1_MAIN    == enFsmId))
    {
        /* cs only时和ims不可用处理相同，都需要判断是否Disable LTE */
        if ( (NAS_MML_CS_VOICE_ONLY != NAS_MML_GetVoiceDomainPreference())
          && (VOS_TRUE == pstImsVoiceInd->ucAvail) )
        {
            return VOS_TRUE;
        }

        /* 不满足IMS voice不可用时的Disable L模条件则不做进一步处理 */
        if (VOS_FALSE == NAS_MMC_IsNeedDisableLte_ImsVoiceNotAvail())
        {
            return VOS_TRUE;
        }

        NAS_MML_SetDisableLteReason(MMC_LMM_DISABLE_LTE_REASON_LTE_VOICE_NOT_AVAILABLE);

        /* GU下不需要再次触发搜网,可以直接通知RRC进行Disable L模 */
        if (NAS_MML_NET_RAT_TYPE_LTE != NAS_MML_GetCurrNetRatType())
        {
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS);

            /* 记录disable lte时驻留的网络信息 */
            NAS_MMC_SndDisableLteNotify(MMC_LMM_DISABLE_LTE_REASON_LTE_VOICE_NOT_AVAILABLE);

            NAS_MMC_SetDisabledLtePlmnId(NAS_MML_GetCurrCampPlmnId());

            return VOS_TRUE;
        }

        NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_UNNOTIFY_AS);

        /* 如果当前L下EPS信令链接存在，则缓存构造的搜网消息 */
        if (VOS_TRUE == NAS_MML_IsRrcConnExist())
        {
            NAS_MMC_SetBufferedPlmnSearchInfo(VOS_TRUE, NAS_MMC_PLMN_SEARCH_SCENE_DISABLE_LTE);

            NAS_MMC_SndLmmRelReq();

            return VOS_TRUE;
        }
    }
    return VOS_FALSE;
#else
    return VOS_TRUE;
#endif
}


VOS_UINT32 NAS_MMC_RcvWasAreaLostInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_AREA_LOST_IND_STRU            *pstAreaLost         = VOS_NULL_PTR;

    pstAreaLost         = (RRMM_AREA_LOST_IND_STRU*)pstMsg;

    if (RRC_NAS_NO_RF_AREA_LOST == pstAreaLost->ulRptMode)
    {
        /* 处理area lost no rf */
        NAS_MMC_ProcNoRfAreaLost_PreProc(NAS_MML_NET_RAT_TYPE_WCDMA);

        if (NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
        {
            NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_WAS_NO_RF_AREA_LOST);
        }
        else
        {
            NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_TAS_NO_RF_AREA_LOST);
        }

        return VOS_FALSE;
    }

    if (NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_WAS_NORMAL_AREA_LOST);
    }
    else
    {
        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_TAS_NORMAL_AREA_LOST);
    }

    return VOS_FALSE;
}
VOS_UINT32 NAS_MMC_RcvGasAreaLostInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRMM_AREA_LOST_IND_STRU            *pstAreaLost         = VOS_NULL_PTR;

    pstAreaLost         = (RRMM_AREA_LOST_IND_STRU*)pstMsg;

    if (RRC_NAS_NO_RF_AREA_LOST == pstAreaLost->ulRptMode)
    {
        /* 处理area lost no rf */
        NAS_MMC_ProcNoRfAreaLost_PreProc(NAS_MML_NET_RAT_TYPE_GSM);

        NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_GAS_NO_RF_AREA_LOST);

        return VOS_FALSE;
    }

    NAS_MMC_RecordOosEvent(NAS_ERR_LOG_OOS_EVENT_GAS_NORMAL_AREA_LOST);

    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvRrmPsStatusInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    RRM_PS_STATUS_IND_STRU             *pstRrmPsStatusInd = VOS_NULL_PTR;
    VOS_RATMODE_ENUM_UINT32             enRrmPsRatType;

    pstRrmPsStatusInd   = (RRM_PS_STATUS_IND_STRU*)pstMsg;
    enRrmPsRatType      = pstRrmPsStatusInd->enRatType;

    /* 向RRM发送去注册消息,MMC连续通知不同RAT的NO RF时,RRM通知资源可用也会通知对应的不通RAT的
       RF资源可用 */
    NAS_MMC_DeregisterRrmResourceNtf(enRrmPsRatType, RRM_PS_TASK_TYPE_NAS_SEARCH);

    /* 继续处理，判断是否需要发起搜网 */
    return VOS_FALSE;
}

VOS_UINT32 NAS_MMC_RcvTiHighPrioRatHplmnSrchTimerExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT8                                               ucCurrHighRatHplmnTdCount;
    VOS_UINT8                                               ucNvHighRatHplmnTdThreshold;

    if (VOS_FALSE == NAS_MMC_IsHighPrioRatHplmnSearchVaild())
    {
        return VOS_TRUE;
    }

    ucCurrHighRatHplmnTdCount   = NAS_MMC_GetTdHighRatSearchCount();
    ucNvHighRatHplmnTdThreshold = NAS_MML_GetHighPrioRatHplmnTimerTdThreshold();

    if (VOS_TRUE == NAS_MMC_IsCurrentTdscdmaMode())
    {
        if (ucCurrHighRatHplmnTdCount < ucNvHighRatHplmnTdThreshold)
        {
            (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen());

            NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();

            return VOS_TRUE;
        }
    }

    /* 驻留到非HPLMN上丢弃该消息 */
    if (VOS_FALSE == NAS_MML_ComparePlmnIdWithHplmn(NAS_MML_GetCurrCampPlmnId()))
    {
        return VOS_TRUE;
    }

    /* 驻留到HPLMN且为最高优先级的接入技术复位定时器搜索次数 */
    if (VOS_TRUE == NAS_MMC_IsCampOnHighestPrioRatHplmn())
    {
        NAS_MMC_ResetCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();

        NAS_MMC_InitTdHighRatSearchCount();

        return VOS_TRUE;
    }

    /* 如果接入层上报not camp on消息，不允许发起背景搜索 */
    if (NAS_MMC_AS_CELL_NOT_CAMP_ON == NAS_MMC_GetAsCellCampOn())
    {
        return VOS_FALSE;
    }

    /* 无法进行高优先级搜索时继续启动该定时器尝试 */
    if (VOS_FALSE == NAS_MMC_IsEnableBgPlmnSearch_PreProc())
    {
        (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerRetryLen());
        NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();

        return VOS_TRUE;
    }

    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_RcvGmmServiceRequestResultInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    GMMMMC_SERVICE_REQUEST_RESULT_IND_STRU                 *pstGmmServReqRsltInd = VOS_NULL_PTR;

    pstGmmServReqRsltInd  = (GMMMMC_SERVICE_REQUEST_RESULT_IND_STRU*)pstMsg;

    /* 服务被拒场景需要主动上报 */
    if (GMM_MMC_ACTION_RESULT_FAILURE == pstGmmServReqRsltInd->enActionResult)
    {
        NAS_MMC_SndMsccServRejRsltInd(NAS_MSCC_PIF_SRVDOMAIN_PS,
                                     pstGmmServReqRsltInd->enRegFailCause);
    }

    return VOS_FALSE;
}
VOS_UINT32 NAS_MMC_RcvRrmmSuspendInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    RRMM_SUSPEND_IND_ST            *pstSuspendMsg = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32      enFsmId;

    NAS_MMC_AS_CELL_CAMP_ON_ENUM_UINT8  enAsCellCampOn;
    enAsCellCampOn = NAS_MMC_GetAsCellCampOn();

    pstSuspendMsg = (RRMM_SUSPEND_IND_ST*)pstMsg;

    if ( ( NAS_MML_NET_RAT_TYPE_WCDMA != NAS_MML_GetCurrNetRatType())
      && ( WUEPS_PID_WRR == pstSuspendMsg->MsgHeader.ulSenderPid ))
    {
        /* 当前不是驻留在W模式下，返回挂起失败 */
        NAS_MMC_SndAsSuspendRsp(RRC_NAS_SUSPEND_FAILURE, WUEPS_PID_WRR);

        return VOS_TRUE;
    }

    if ( ( NAS_MML_NET_RAT_TYPE_GSM != NAS_MML_GetCurrNetRatType())
      && ( UEPS_PID_GAS == pstSuspendMsg->MsgHeader.ulSenderPid ))
    {
        /* 当前不是驻留在G模式下，返回挂起失败 */
        NAS_MMC_SndAsSuspendRsp(RRC_NAS_SUSPEND_FAILURE, UEPS_PID_GAS);

        return VOS_TRUE;
    }

    if ( pstSuspendMsg->ucSuspendCause >= MMC_SUSPEND_CAUSE_BUTT )
    {
        /* 输入参数非法 */
        NAS_MMC_SndAsSuspendRsp(RRC_NAS_SUSPEND_FAILURE, pstSuspendMsg->MsgHeader.ulSenderPid);

        return VOS_TRUE;
    }

    /* 如果为当前模式,已经在异系统状态机中,再次收到挂起消息,直接复位 */
    enFsmId = NAS_MMC_GetCurrFsmId();
    if ( ( NAS_MMC_FSM_INTER_SYS_HO == enFsmId )
      || ( NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId )
      || ( NAS_MMC_FSM_INTER_SYS_OOS == enFsmId ) )
    {
        if ( ( NAS_MML_NET_RAT_TYPE_WCDMA == NAS_MML_GetCurrNetRatType())
          && ( WUEPS_PID_WRR == pstSuspendMsg->MsgHeader.ulSenderPid ))
        {
            if ( NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode())
            {
                NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_IN_INTERSYSFSM_RCV_WAS_SUSPEND_IND);
            }
            else
            {
                NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_IN_INTERSYSFSM_RCV_TD_SUSPEND_IND);
            }
        }
        if ( ( NAS_MML_NET_RAT_TYPE_GSM == NAS_MML_GetCurrNetRatType())
          && ( UEPS_PID_GAS == pstSuspendMsg->MsgHeader.ulSenderPid ))
        {
            NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_IN_INTERSYSFSM_RCV_GAS_SUSPEND_IND);
        }

        return VOS_TRUE;
    }


    /* 当前NAS为未驻留状态 */
    if ( (NAS_MMC_SPEC_PLMN_SEARCH_RUNNING  == NAS_MMC_GetSpecPlmnSearchState())
       && (NAS_MMC_AS_CELL_NOT_CAMP_ON      == enAsCellCampOn) )
    {
        /* 输入参数非法 */
        NAS_MMC_SndAsSuspendRsp(RRC_NAS_SUSPEND_FAILURE, pstSuspendMsg->MsgHeader.ulSenderPid);

        return VOS_TRUE;
    }

    /* 进入状态机处理 */
    return VOS_FALSE;
}
VOS_UINT32 NAS_MMC_RcvRrmmResumeInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    RRMM_RESUME_IND_ST                   *pstResumeMsg = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32            enFsmId;
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8       enCurrNetRatType;

    pstResumeMsg = (RRMM_RESUME_IND_ST*)pstMsg;

    /* 如果为当前模式,已经在异系统状态机中,再次收到挂起消息,直接复位 */
    enFsmId = NAS_MMC_GetCurrFsmId();
    if ( ( NAS_MMC_FSM_INTER_SYS_HO != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_CELLRESEL != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_OOS != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_CCO != enFsmId ))
    {
        enCurrNetRatType = NAS_MML_GetCurrNetRatType();
        /* 判断发送id */
        if ( ( ( NAS_MML_NET_RAT_TYPE_GSM == enCurrNetRatType)
            && ( UEPS_PID_GAS != pstResumeMsg->MsgHeader.ulSenderPid ))
         ||  ( ( NAS_MML_NET_RAT_TYPE_WCDMA == enCurrNetRatType)
            && ( WUEPS_PID_WRR != pstResumeMsg->MsgHeader.ulSenderPid ))
         ||    ( NAS_MML_NET_RAT_TYPE_LTE == enCurrNetRatType ))
        {
            if ( UEPS_PID_GAS == pstResumeMsg->MsgHeader.ulSenderPid )
            {
                NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_NOT_IN_INTERSYSFSM_RCV_GAS_RESUME_IND);
            }
            else if ( NAS_UTRANCTRL_UTRAN_MODE_FDD == NAS_UTRANCTRL_GetCurrUtranMode() )
            {
                NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_NOT_IN_INTERSYSFSM_RCV_WAS_RESUME_IND);
            }
            else
            {
                NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_NOT_IN_INTERSYSFSM_RCV_TD_RESUME_IND);
            }
        }

        return VOS_TRUE;
    }

    /* 进入状态机处理 */
    return VOS_FALSE;
}

#if (FEATURE_ON == FEATURE_DSDS)
#if(FEATURE_ON == FEATURE_LTE)

VOS_VOID NAS_MMC_ConvertRrcSessionTypeToLmmFormat(
    RRC_NAS_SESSION_TYPE_ENUM_UINT8     enRrcSessionType,
    MMC_LMM_SESSION_TYPE_ENUM_UINT8    *puclmmSessionType
)
{
    switch (enRrcSessionType)
    {
        case RRC_NAS_SESSION_TYPE_CS_MO_NORMAL_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_CS_MO_NORMAL_CALL;
            break;

        case RRC_NAS_SESSION_TYPE_CS_MO_EMERGENCY_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_CS_MO_EMERGENCY_CALL;
            break;

        case RRC_NAS_SESSION_TYPE_CS_MO_SS:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_CS_MO_SS;
            break;

        case RRC_NAS_SESSION_TYPE_CS_MO_SMS:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_CS_MO_SMS;
            break;

        case RRC_NAS_SESSION_TYPE_PS_CONVERSAT_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_PS_CONVERSAT_CALL;
            break;

        case RRC_NAS_SESSION_TYPE_PS_STREAM_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_PS_STREAM_CALL;
            break;

        case RRC_NAS_SESSION_TYPE_PS_INTERACT_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_PS_INTERACT_CALL;
            break;

         case RRC_NAS_SESSION_TYPE_PS_BACKGROUND_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_PS_BACKGROUND_CALL;
            break;

         case RRC_NAS_SESSION_TYPE_PS_SUBSCRIB_TRAFFIC_CALL:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_PS_SUBSCRIB_TRAFFIC_CALL;
            break;

        default:
            *puclmmSessionType = MMC_LMM_SESSION_TYPE_BUTT;
            break;
    }
    return;
}
#endif


VOS_VOID NAS_MMC_SndBeginSessionNotify(
    RRC_NAS_SESSION_TYPE_ENUM_UINT8     enRrcSessionType
)
{
    TAF_CTRL_STRU                       stCtrl;
    MTC_SESSION_TYPE_ENUM_UINT8         enMtcSessionType;

#if(FEATURE_ON == FEATURE_LTE)
     MMC_LMM_SESSION_TYPE_ENUM_UINT8    enLmmSessionType;
#endif

    enMtcSessionType = MTC_SESSION_TYPE_BUTT;
    PS_MEM_SET(&stCtrl, 0x0, sizeof(stCtrl));

    /* 平台支持W/TD，发送end session */
    if (VOS_TRUE == NAS_MML_IsPlatformSupportUtran())
    {
        NAS_MMC_SndAsBeginSessionNotify(WUEPS_PID_WRR, enRrcSessionType);
    }

    /* 平台支持G，发送end session */
    if (VOS_TRUE == NAS_MML_IsPlatformSupportGsm())
    {
        NAS_MMC_SndAsBeginSessionNotify(UEPS_PID_GAS, enRrcSessionType);
    }

#if(FEATURE_ON == FEATURE_LTE)
    NAS_MMC_ConvertRrcSessionTypeToLmmFormat(enRrcSessionType, &enLmmSessionType);

    /* 平台支持LTE，发送end session */
    if ( VOS_TRUE == NAS_MML_IsPlatformSupportLte() )
    {
        NAS_MMC_SndLmmBeginSessionNotify(enLmmSessionType);
    }
#endif

    if(VOS_OK == NAS_ConvertSessionTypeToMTCFormat(enRrcSessionType, &enMtcSessionType))
    {
        /* 填写CTL头 */
        stCtrl.ulModuleId = WUEPS_PID_MMC;

        MTC_SetBeginSessionInfo(&stCtrl,enMtcSessionType);
    }

    return;
}



VOS_VOID NAS_MMC_SndEndSessionNotify(
    RRC_NAS_SESSION_TYPE_ENUM_UINT8     enRrcSessionType
)
{
    TAF_CTRL_STRU                       stCtrl;
    MTC_SESSION_TYPE_ENUM_UINT8         enMtcSessionType;

#if(FEATURE_ON == FEATURE_LTE)
     MMC_LMM_SESSION_TYPE_ENUM_UINT8    enLmmSessionType;
#endif

    enMtcSessionType = MTC_SESSION_TYPE_BUTT;
    PS_MEM_SET(&stCtrl, 0x0, sizeof(stCtrl));

    /* 平台支持W/TD，发送end session */
    if (VOS_TRUE == NAS_MML_IsPlatformSupportUtran())
    {
        NAS_MMC_SndAsEndSessionNotify(WUEPS_PID_WRR, enRrcSessionType, 0);
    }

    /* 平台支持G，发送end session */
    if (VOS_TRUE == NAS_MML_IsPlatformSupportGsm())
    {
        NAS_MMC_SndAsEndSessionNotify(UEPS_PID_GAS, enRrcSessionType, 0);
    }

#if(FEATURE_ON == FEATURE_LTE)
    NAS_MMC_ConvertRrcSessionTypeToLmmFormat(enRrcSessionType, &enLmmSessionType);

    /* 平台支持LTE，发送end session */
    if ( VOS_TRUE == NAS_MML_IsPlatformSupportLte() )
    {
        NAS_MMC_SndLmmEndSessionNotify(enLmmSessionType);
    }
#endif

    if(VOS_OK == NAS_ConvertSessionTypeToMTCFormat(enRrcSessionType, &enMtcSessionType))
    {
        /* 填写CTL头 */
        stCtrl.ulModuleId = WUEPS_PID_MMC;

        MTC_SetEndSessionInfo(&stCtrl,enMtcSessionType);
    }

    return;
}



VOS_VOID NAS_MMC_ConvertSrvTypeToRrcSessionType(
    NAS_MSCC_PIF_SRV_TYPE_ENUM_UINT8         enMsccMmcSrvType,
    RRC_NAS_SESSION_TYPE_ENUM_UINT8    *pucRrcNasSessionType
)
{
    switch (enMsccMmcSrvType)
    {
        case NAS_MSCC_PIF_SRV_TYPE_CS_MO_NORMAL_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_CS_MO_NORMAL_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_CS_MO_SS:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_CS_MO_SS;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_CS_MO_SMS:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_CS_MO_SMS;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_CS_MO_EMERGENCY_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_CS_MO_EMERGENCY_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_PS_CONVERSAT_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_PS_CONVERSAT_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_PS_STREAM_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_PS_STREAM_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_PS_INTERACT_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_PS_INTERACT_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_PS_BACKGROUND_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_PS_BACKGROUND_CALL;
            break;

        case NAS_MSCC_PIF_SRV_TYPE_PS_SUBSCRIB_TRAFFIC_CALL:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_PS_SUBSCRIB_TRAFFIC_CALL;
            break;

        default:
            *pucRrcNasSessionType = RRC_NAS_SESSION_TYPE_BUTT;
            break;
    }

    return;
}


VOS_UINT32  NAS_MMC_RcvMsccSrvAcqReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;
    VOS_UINT32                          ulState;
    MSCC_MMC_SRV_ACQ_REQ_STRU           *pstSrvAcqReq = VOS_NULL_PTR;

    pstSrvAcqReq = (MSCC_MMC_SRV_ACQ_REQ_STRU*)pstMsg;
    enFsmId      = NAS_MMC_GetCurrFsmId();
    ulState      = NAS_MMC_GetFsmTopState();

    /* 如果是紧急呼叫触发搜网当前处于on plmn则直接回复搜网结果为成功；
       如果是普通业务触发搜网当前处于on plmn且cs ps有一个域可提供正常服务，则回复搜网结果为成功 */
    if ((NAS_MMC_FSM_L1_MAIN           == enFsmId)
     && (NAS_MMC_L1_STA_ON_PLMN        == ulState))
    {
        if (NAS_MSCC_PIF_SRV_TYPE_CS_MO_EMERGENCY_CALL == pstSrvAcqReq->enSrvType)
        {
            NAS_MMC_SndMsccSrvAcqCnf(NAS_MSCC_PIF_SRV_ACQ_RESULT_SUCCESS);
            return VOS_TRUE;
        }

        if (VOS_TRUE == NAS_MMC_IsNormalServiceStatus())
        {
            NAS_MMC_SndMsccSrvAcqCnf(NAS_MSCC_PIF_SRV_ACQ_RESULT_SUCCESS);
            return VOS_TRUE;
        }
    }

    /* 如果当前anycell状态机在运行且不是紧急呼叫直接回复搜网失败结果 */
    if ((NAS_MMC_FSM_ANYCELL_SEARCH == enFsmId)
     && (NAS_MSCC_PIF_SRV_TYPE_CS_MO_EMERGENCY_CALL != pstSrvAcqReq->enSrvType))
    {
        NAS_MMC_SndMsccSrvAcqCnf(NAS_MSCC_PIF_SRV_ACQ_RESULT_FAIL);
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
VOS_UINT32  NAS_MMC_RcvMsccBeginSessionNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_BEGIN_SESSION_NOTIFY_STRU  *pstBeginSessionNotify = VOS_NULL_PTR;
    RRC_NAS_SESSION_TYPE_ENUM_UINT8     enRrcNasSessionType;

    pstBeginSessionNotify = (MSCC_MMC_BEGIN_SESSION_NOTIFY_STRU *)pstMsg;

    NAS_MMC_ConvertSrvTypeToRrcSessionType(pstBeginSessionNotify->enSrvType, &enRrcNasSessionType);

    NAS_MMC_SndBeginSessionNotify(enRrcNasSessionType);

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMsccEndSessionNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_END_SESSION_NOTIFY_STRU    *pstEndSessionNotify = VOS_NULL_PTR;
    RRC_NAS_SESSION_TYPE_ENUM_UINT8     enRrcNasSessionType;

    pstEndSessionNotify = (MSCC_MMC_END_SESSION_NOTIFY_STRU *)pstMsg;

    NAS_MMC_ConvertSrvTypeToRrcSessionType(pstEndSessionNotify->enSrvType,
                                           &enRrcNasSessionType);

    NAS_MMC_SndEndSessionNotify(enRrcNasSessionType);

    return VOS_TRUE;
}

#endif

#if   (FEATURE_ON == FEATURE_LTE)
VOS_UINT32 NAS_MMC_RcvLmmSuspendInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    LMM_MMC_SUSPEND_IND_STRU           *pstSuspendMsg = VOS_NULL_PTR;
    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

    NAS_MMC_AS_CELL_CAMP_ON_ENUM_UINT8  enAsCellCampOn;
    enAsCellCampOn = NAS_MMC_GetAsCellCampOn();

    pstSuspendMsg = (LMM_MMC_SUSPEND_IND_STRU*)pstMsg;

    if ( NAS_MML_NET_RAT_TYPE_LTE != NAS_MML_GetCurrNetRatType())
    {
        /* 当前不是驻留在L模式下，返回挂起失败 */
        NAS_MMC_SndLmmSuspendRsp(MMC_LMM_FAIL);

        return VOS_TRUE;
    }

    if ( pstSuspendMsg->ulSysChngType >= MMC_LMM_SUS_TYPE_BUTT )
    {
        /* 输入参数非法 */
        NAS_MMC_SndLmmSuspendRsp(MMC_LMM_FAIL);

        return VOS_TRUE;
    }

    /* 如果为当前模式,已经在异系统状态机中,再次收到挂起消息,直接复位 */
    enFsmId = NAS_MMC_GetCurrFsmId();
    if ( ( NAS_MMC_FSM_INTER_SYS_HO == enFsmId )
      || ( NAS_MMC_FSM_INTER_SYS_CELLRESEL == enFsmId )
      || ( NAS_MMC_FSM_INTER_SYS_OOS == enFsmId ) )
    {
        if ( NAS_MML_NET_RAT_TYPE_LTE == NAS_MML_GetCurrNetRatType())
        {
            NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_IN_INTERSYSFSM_RCV_LMM_SUSPEND_IND);
        }

        return VOS_TRUE;
    }


    /* 当前NAS为未驻留状态，在搜网和SUSPEND IND对冲场景下不能触发SUSPEND */
    if ( (NAS_MMC_SPEC_PLMN_SEARCH_RUNNING  == NAS_MMC_GetSpecPlmnSearchState())
       && (NAS_MMC_AS_CELL_NOT_CAMP_ON      == enAsCellCampOn) )
    {
        /* 状态不正确 */
        NAS_MMC_SndLmmSuspendRsp(MMC_LMM_FAIL);

        return VOS_TRUE;
    }

    /* 进入状态机处理 */
    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_RcvLmmResumeInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{

    NAS_MMC_FSM_ID_ENUM_UINT32          enFsmId;

/* MOdify by l00324781 for CDMA Iteration 12, 2015-6-9, begin */
    NAS_MMC_L1_STA_ENUM_UINT32          enCurrState;

    /* 如果为当前模式,不在异系统状态机中,再次收到其他模式的恢复消息,直接复位 */
    enFsmId = NAS_MMC_GetCurrFsmId();
    enCurrState = NAS_MMC_GetFsmTopState();

    /* 当收到LMM_MMC_RESUME_IND消息，当前状态为DEACT，状态机ID为NAS_MMC_FSM_L1_MAIN，说明LTE当前处于从模
       并且没有任何动作在执行，处理该消息
    */
    if ((NAS_MMC_L1_STA_DEACT == enCurrState)
         && (NAS_MMC_FSM_L1_MAIN == enFsmId))
    {
        return VOS_FALSE;
    }
/* MOdify by l00324781 for CDMA Iteration 12, 2015-6-9, end */

    if ( ( NAS_MMC_FSM_INTER_SYS_HO != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_CELLRESEL != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_OOS != enFsmId )
      && ( NAS_MMC_FSM_INTER_SYS_CCO != enFsmId ))
    {
        if ( NAS_MML_NET_RAT_TYPE_LTE != NAS_MML_GetCurrNetRatType())
        {
            NAS_MML_SoftReBoot(NAS_MML_REBOOT_SCENE_MMC_NOT_IN_INTERSYSFSM_RCV_LMM_RESUME_IND);
        }

        return VOS_TRUE;
    }

    /* 进入状态机处理 */
    return VOS_FALSE;
}


VOS_UINT32 NAS_MMC_RcvLmmInfoChangeNotifyInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_INFO_CHANGE_NOTIFY_STRU                        *pstRcvMsg = VOS_NULL_PTR;
    MMCGMM_EMERGENCY_NUM_LIST_IND_STRU                      stEmergencyNumList;
    NAS_MSCC_PIF_NW_EMC_BS_CAP_ENUM_UINT8                   enNwEmcBS;
    NAS_MSCC_PIF_NW_IMS_VOICE_CAP_ENUM_UINT8                enNwImsVoPS;
    NAS_MSCC_PIF_LTE_CS_CAPBILITY_ENUM_UINT8                enLteCsCap;
    VOS_UINT32                                              ulState;

    ulState   = NAS_MMC_GetFsmTopState();

    /* BG搜状态机系统消息可能延迟发送，需要在注册状态和服务状态之前通知MMA */
    if ((NAS_MMC_FSM_BG_PLMN_SEARCH                         == NAS_MMC_GetCurrFsmId())
     && ((NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_REG_IND       == ulState)
      || (NAS_MMC_BG_PLMN_SEARCH_STA_WAIT_EPS_CONN_REL_IND  == ulState)))
    {
        NAS_MMC_SndMsccSysInfo_BgPlmnSearch();
    }

    pstRcvMsg = (LMM_MMC_INFO_CHANGE_NOTIFY_STRU *)pstMsg;

    PS_MEM_SET(((VOS_UINT8*)&stEmergencyNumList) + VOS_MSG_HEAD_LENGTH, 0, sizeof(MMCGMM_EMERGENCY_NUM_LIST_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    /* update LTE CS capability */
    NAS_MML_SetAdditionUpdateRslt((NAS_MML_ADDITION_UPDATE_RSLT_INFO_ENUM_UINT8)pstRcvMsg->enLteCsCap);

    enNwEmcBS = (NAS_MSCC_PIF_NW_EMC_BS_CAP_ENUM_UINT8)pstRcvMsg->enNwEmcBS;

    enNwImsVoPS = (NAS_MSCC_PIF_NW_IMS_VOICE_CAP_ENUM_UINT8)pstRcvMsg->enNwImsVoPS;

    NAS_MML_SetLteNwImsVoiceSupportFlg((NAS_MML_NW_IMS_VOICE_CAP_ENUM_UINT8)enNwImsVoPS);

    switch (pstRcvMsg->enLteCsCap)
    {
        case LMM_MMC_LTE_CS_CAPBILITY_NO_ADDITION_INFO:
            enLteCsCap = NAS_MSCC_PIF_LTE_CS_CAPBILITY_NO_ADDITION_INFO;
            break;

        case LMM_MMC_LTE_CS_CAPBILITY_CSFB_NOT_PREFER:
            enLteCsCap = NAS_MSCC_PIF_LTE_CS_CAPBILITY_CSFB_NOT_PREFER;
            break;

        case LMM_MMC_LTE_CS_CAPBILITY_SMS_ONLY:
            enLteCsCap = NAS_MSCC_PIF_LTE_CS_CAPBILITY_SMS_ONLY;
            break;

        case LMM_MMC_LTE_CS_CAPBILITY_NOT_ATTACHED:
        default:
            enLteCsCap = NAS_MSCC_PIF_LTE_CS_CAPBILITY_NOT_SUPPORTED;
            break;
    }

    NAS_MMC_SndMsccNetworkCapabilityInfoInd(enNwImsVoPS, enNwEmcBS, enLteCsCap);

    /* 给GMM发送紧急呼列表 */
    stEmergencyNumList.ucOpEmcNumList       = VOS_TRUE;
    stEmergencyNumList.ucEmergencyNumAmount = pstRcvMsg->ucEmergencyNumAmount;
    PS_MEM_CPY(stEmergencyNumList.astEmergencyNumList, pstRcvMsg->astEmergencyNumList,
                sizeof(LMM_MMC_EMERGENCY_NUM_STRU) * LMM_MMC_EMERGENCY_NUM_LIST_MAX_RECORDS);

    NAS_MMC_SndGmmEmergencyNumList(&stEmergencyNumList);

    return VOS_TRUE;
}

#endif
VOS_UINT32 NAS_MMC_RcvGmmSigStateInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MMCGMM_SIGNALING_STATUS_IND_STRU                       *pstGmmSigStateInd = VOS_NULL_PTR;
    VOS_UINT8                                               ucPdpStatusFlg;


    pstGmmSigStateInd           = (MMCGMM_SIGNALING_STATUS_IND_STRU*)pstMsg;

    /* 获取当前Pdp状态 */
    ucPdpStatusFlg = NAS_MML_GetConnStatus()->ucPdpStatusFlg;

    /* 当前状态不为MMC_GMM_SIGNALING_STATUS_ABSENT且PDP已经激活 */
    if ( (MMC_GMM_SIGNALING_STATUS_ABSENT != pstGmmSigStateInd->ulSignalingSts)
      && (VOS_TRUE == ucPdpStatusFlg) )
    {
        NAS_MMC_SndMsccPsServiceConnStatusInd(VOS_TRUE, MSCC_MMC_PS_SIGNALING_TYPE_GU);
    }

    /* GMM信令连接不存在 */
    if (MMC_GMM_SIGNALING_STATUS_ABSENT == pstGmmSigStateInd->ulSignalingSts)
    {
        NAS_MMC_SndMsccPsServiceConnStatusInd(VOS_FALSE, MSCC_MMC_PS_SIGNALING_TYPE_GU);
    }

    return VOS_FALSE;
}
VOS_VOID NAS_MMC_ProcNoRfAreaLost_PreProc(
    NAS_MML_NET_RAT_TYPE_ENUM_UINT8     enNasRat
)
{
    /* 设置NO RF存在标记信息 */
    NAS_MML_SetCurrRatRfAvailFlg(enNasRat, VOS_FALSE);

    /*向RRM发送注册消息*/
    NAS_MMC_RegisterRrmResourceNtf(enNasRat, RRM_PS_TASK_TYPE_NAS_SEARCH);

    /* 向MSCC通知当前NO RF信息  */
    NAS_MMC_SndMsccRfAvailInd(VOS_FALSE);

    return;
}


#if (FEATURE_ON == FEATURE_IMS)

VOS_UINT32 NAS_MMC_RcvMsccImsSrvInfoNotify_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_IMS_SRV_INFO_NOTIFY_STRU   *pstRcvMsg   = VOS_NULL_PTR;

    pstRcvMsg   = (MSCC_MMC_IMS_SRV_INFO_NOTIFY_STRU *)pstMsg;

    NAS_MML_SetImsCallFlg(pstRcvMsg->ucImsCallFlg);

    return VOS_TRUE;
}
#endif

#if (FEATURE_ON == FEATURE_IMS)
VOS_UINT32 NAS_MMC_RcvMsccImsSwitchStateInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_IMS_SWITCH_STATE_IND_STRU *pstRcvMsg   = VOS_NULL_PTR;
    VOS_UINT32                          ulDisablteRoamFlg;

    pstRcvMsg           = (MSCC_MMC_IMS_SWITCH_STATE_IND_STRU *)pstMsg;
    ulDisablteRoamFlg   = NAS_MML_GetDisableLteRoamFlg();

    /* 更新全局变量 */
    NAS_MML_SetLteImsSupportFlg(pstRcvMsg->enImsSwitch);

    /* IMS开关打开时判断是否需要重新Enable LTE
       IMS开关关闭时，不需要任何操作。是否需要disable LTE，靠原有流程保证。
    */
    if (NAS_MSCC_PIF_IMS_SWITCH_STATE_ON == pstRcvMsg->enImsSwitch)
    {
        if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_ImsSwitchOnOrNotCsOnly())
        {
            /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
            if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
             && (VOS_FALSE == ulDisablteRoamFlg))
            {
                NAS_MMC_SndEnableLteNotify();
                NAS_MMC_ClearDisabledLtePlmnId();
                NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
            }

            /* 停ENABLE LTE定时器 */
            if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
            {
                NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);

            /* 如果需要搜网到状态机里触发搜网 */
            if (VOS_TRUE == NAS_MMC_IsEnableLteTriggerPlmnSearch_ImsSwitchOnOrNotCsOnly())
            {
                return VOS_FALSE;
            }

            /* 不需要搜网时，启动TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER
               漫游的场景不考虑，靠现有回LTE的流程来保证 */
            if (VOS_TRUE == NAS_MMC_IsNeedStartHighPrioRatHPlmnTimer())
            {
                /* 启动定时器 */
                if (VOS_TRUE == NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen()))
                {
                    NAS_MMC_AddCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
                    NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();
                }
            }
        }
    }

    return VOS_TRUE;
}
#endif
VOS_UINT32 NAS_MMC_RcvMsccVoiceDomainChangeInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MSCC_PIF_VOICE_DOMAIN_CHANGE_IND_STRU              *pstRcvMsg   = VOS_NULL_PTR;
    VOS_UINT32                                              ulDisablteRoamFlg;

    ulDisablteRoamFlg   = NAS_MML_GetDisableLteRoamFlg();

    pstRcvMsg = (NAS_MSCC_PIF_VOICE_DOMAIN_CHANGE_IND_STRU *)pstMsg;

    /* 更新全局变量: NAS_MML_VOICE_DOMAIN_PREFERENCE_ENUM_UINT8和MMA_MMC_VOICE_DOMAIN_ENUM_UINT32的取值相同 */
    NAS_MML_SetVoiceDomainPreference((NAS_MML_VOICE_DOMAIN_PREFERENCE_ENUM_UINT8)pstRcvMsg->enVoiceDomain);

    /* 平台不支持LTE时，不需要给LTE发送ID_MMC_LMM_VOICE_DOMAIN_CHANGE_IND */
    if (VOS_FALSE == NAS_MML_IsPlatformSupportLte())
    {
        /* 通知GMM voice domain发生改变，触发GMM RAU流程 */
        NAS_MMC_SndGmmVoiceDomainChangeNotify();

        NAS_NORMAL_LOG(WUEPS_PID_MMC, "NAS_MMC_SndLmmVoiceDomainChangeInd():NORMAL:Platform Not Support LTE");

        /* 返回 */
        return VOS_TRUE;
    }

    /* 通知LMM voice domain发生改变 */
    NAS_MMC_SndLmmVoiceDomainChangeInd((MMC_LMM_VOICE_DOMAIN_ENUM_UINT32)pstRcvMsg->enVoiceDomain);

    /* cs only时判断不再是否需要disable LTE,和wangchen确认，cs only会触发LTE下TAU,
       TAU结束之后不管网络能力是否变化MMA都会通知IMSA service change ind，IMSA会通知MMA ims不可用，mmc收到ims不可用再去判断是否需要Disable LTE
       否则Disable LTE前会先挂起LTE，打断LTE下的TAU流程 */

    /* 非cs only时判断是否需要enable LTE,并作相应的处理 */
    if (NAS_MSCC_PIF_VOICE_DOMAIN_CS_ONLY != pstRcvMsg->enVoiceDomain)
    {
        if (VOS_TRUE == NAS_MMC_IsNeedEnableLte_ImsSwitchOnOrNotCsOnly())
        {
            /* 如果由于禁止LTE国际漫游导致的disable LTE，再未回到国内之前此时也不通知enable LTE */
            if ((NAS_MML_LTE_CAPABILITY_STATUS_DISABLE_NOTIFIED_AS == NAS_MML_GetLteCapabilityStatus())
             && (VOS_FALSE == ulDisablteRoamFlg))
            {
                NAS_MMC_SndEnableLteNotify();
                NAS_MMC_ClearDisabledLtePlmnId();
                NAS_MMC_SetEnableLteTimerExpireFlag(VOS_FALSE);
            }

            /* 停ENABLE LTE定时器 */
            if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER))
            {
                NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_ENABLE_LTE_TIMER);
            }

            /* 更新disable LTE能力标记 */
            NAS_MML_SetLteCapabilityStatus(NAS_MML_LTE_CAPABILITY_STATUS_REENABLE_NOTIFIED_AS);

            /* 如果需要搜网到状态机里触发搜网 */
            if (VOS_TRUE == NAS_MMC_IsEnableLteTriggerPlmnSearch_ImsSwitchOnOrNotCsOnly())
            {
                return VOS_FALSE;
            }

            /* 不需要搜网时，启动TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER
               漫游的场景不考虑，靠现有回LTE的流程来保证 */
            if (VOS_TRUE == NAS_MMC_IsNeedStartHighPrioRatHPlmnTimer())
            {
                /* 启动定时器 */
                if (VOS_TRUE == NAS_MMC_StartTimer(TI_NAS_MMC_HIGH_PRIO_RAT_HPLMN_TIMER, NAS_MMC_GetHighPrioRatHplmnTimerLen()))
                {
                    NAS_MMC_AddCurHighPrioRatHplmnTimerFirstSearchCount_L1Main();
                    NAS_MMC_UpdateHighPrioRatHPlmnTimerTdCount();
                }
            }
        }
    }

    /* 需要去LTE搜网时，不通知GMM voice domain变化，否则搜网流程会和RAU流程冲突
       后续再回来GU下时，如果voice domain和之前发起rau时的voice domain不同，会触发GU下重新做RAU
       其他情况时通知GMM voice domain发生改变，触发GMM RAU流程 */
    NAS_MMC_SndGmmVoiceDomainChangeNotify();

    return VOS_TRUE;
}


VOS_UINT32  NAS_MMC_RcvMsccCFPlmnSetReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_CFPLMN_SET_REQ_STRU       *pstCFPlmnReqMsg  = VOS_NULL_PTR;
    NAS_MML_PLMN_ID_STRU               *pstUserSpecPlmnId = VOS_NULL_PTR;
    VOS_UINT32                          ulChgFlag;
    NAS_MML_SIM_FORBIDPLMN_INFO_STRU   *pstForbidPlmnList = VOS_NULL_PTR;

    pstCFPlmnReqMsg = (MSCC_MMC_CFPLMN_SET_REQ_STRU *)pstMsg;

    /* 指向用户指定的PlmnId */
    pstUserSpecPlmnId = (NAS_MML_PLMN_ID_STRU *)(&pstCFPlmnReqMsg->stPlmnInfo);

    if (NAS_MSCC_PIF_CFPLMN_TYPE_ADD_ONE == pstCFPlmnReqMsg->enCmdType)
    {
        /* 加入forbidden plmn list中的plmn不是hplmn才加入,否则禁止加入 */
        if (VOS_TRUE == NAS_MML_ComparePlmnIdWithHplmn((NAS_MML_PLMN_ID_STRU *)pstUserSpecPlmnId))
        {
            /* 上报设置结果　*/
            NAS_MMC_SndMsccCFPlmnSetCnf(NAS_MSCC_PIF_CFPLMN_OPER_RESULT_FAILURE);
            return VOS_TRUE;
        }

        /* 加入禁止列表 */
        ulChgFlag = NAS_MML_AddForbPlmn((NAS_MML_PLMN_ID_STRU *)pstUserSpecPlmnId);

        if (VOS_TRUE == ulChgFlag)
        {
            NAS_MMC_SndUsimUpdateForbPlmnReq();
        }
    }
    else if ((NAS_MSCC_PIF_CFPLMN_TYPE_DEL_ONE == pstCFPlmnReqMsg->enCmdType)
          || (NAS_MSCC_PIF_CFPLMN_TYPE_DEL_ALL == pstCFPlmnReqMsg->enCmdType))
    {
        ulChgFlag = VOS_FALSE;

        /* 获取需要删除的PLMN */
        pstForbidPlmnList = NAS_MML_GetForbidPlmnInfo();

        /* 如果是删除所有禁止PLMN则直接调用删除所有禁止PLMN公共处理并返回结果 */
        if (NAS_MSCC_PIF_CFPLMN_TYPE_DEL_ALL == pstCFPlmnReqMsg->enCmdType)
        {
            if (0 != pstForbidPlmnList->ucForbPlmnNum)
            {
                pstForbidPlmnList->ucForbPlmnNum = 0;
                ulChgFlag = VOS_TRUE;
            }
        }
        else
        {
            ulChgFlag = NAS_MML_DelForbPlmn(pstUserSpecPlmnId);
        }

        if (VOS_TRUE == ulChgFlag)
        {
            NAS_MMC_SndUsimUpdateForbPlmnReq();
        }
    }
    else
    {
        NAS_MMC_SndMsccCFPlmnSetCnf(NAS_MSCC_PIF_CFPLMN_OPER_RESULT_FAILURE);
        return VOS_TRUE;
    }

    NAS_MMC_SndMsccCFPlmnSetCnf(NAS_MSCC_PIF_CFPLMN_OPER_RESULT_SUCCESS);

    return VOS_TRUE;
}

VOS_UINT32  NAS_MMC_RcvMsccCFPlmnQueryReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_SIM_FORBIDPLMN_INFO_STRU   *pstForbidPlmnList = VOS_NULL_PTR;
    VOS_UINT8                           ucForbPlmnNum;
    NAS_MSCC_PIF_CFPLMN_LIST_STRU       stCFPlmnListInfo;

    PS_MEM_SET(&stCFPlmnListInfo, 0x00, sizeof(NAS_MSCC_PIF_CFPLMN_LIST_STRU));

    /* 获取禁止列表数 */
    pstForbidPlmnList                = NAS_MML_GetForbidPlmnInfo();
    stCFPlmnListInfo.ucPlmnNum       = pstForbidPlmnList->ucForbPlmnNum;

    if (stCFPlmnListInfo.ucPlmnNum > NAS_MSCC_PIF_MAX_CFPLMN_NUM)
    {
        stCFPlmnListInfo.ucPlmnNum = NAS_MSCC_PIF_MAX_CFPLMN_NUM;
    }

    for (ucForbPlmnNum = 0; ((ucForbPlmnNum < stCFPlmnListInfo.ucPlmnNum) && (ucForbPlmnNum < NAS_MML_MAX_FORBPLMN_NUM)); ucForbPlmnNum++)
    {
        stCFPlmnListInfo.astPlmn[ucForbPlmnNum].ulMcc = pstForbidPlmnList->astForbPlmnIdList[ucForbPlmnNum].ulMcc;
        stCFPlmnListInfo.astPlmn[ucForbPlmnNum].ulMnc = pstForbidPlmnList->astForbPlmnIdList[ucForbPlmnNum].ulMnc;
    }

    NAS_MMC_SndMsccCFPlmnQueryCnf(NAS_MSCC_PIF_CFPLMN_OPER_RESULT_SUCCESS, &stCFPlmnListInfo);

    return VOS_TRUE;
}
VOS_UINT32  NAS_MMC_RcvTafSDTConnInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_MML_CONN_STATUS_INFO_STRU      *pstConnStatus = VOS_NULL_PTR;
    VOS_UINT32                          ulRrcConnExistFlg;

    ulRrcConnExistFlg = NAS_MML_IsRrcConnExist();

    pstConnStatus     = NAS_MML_GetConnStatus();

    /* PS域有当前业务存在, 则不发送回放消息 */
    if ((VOS_TRUE == pstConnStatus->ucPdpStatusFlg)
      &&(VOS_TRUE == ulRrcConnExistFlg))
    {
        return VOS_TRUE;
    }

    /* UTRANCTRL模块的回放消息 */
    NAS_UTRANCTRL_SndOutsideContextData();

    NAS_MMC_SndOutsideContextData();
    NAS_MM_SndOutsideContextData();
    NAS_GMM_SndOutsideContextData();

    /* 不需进入状态机处理 */
    return VOS_TRUE;
}

/* Added by y00307564 for CDMA Iteration 8 2015-2-5 begin */
/*****************************************************************************
 函 数 名  : NAS_MMC_FillPrefPlmnFile
 功能描述  : 按USIM卡格式填充优先级列表文件内容
 输入参数  : VOS_UINT16                           usUpdateFile,
             VOS_UINT8                           *pucEfFile,
             NAS_MSCC_PIF_USER_PLMN_LIST_STRU    *pstUPlmnInfo
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_VOID NAS_MMC_FillPrefPlmnFile(
    VOS_UINT16                          usUpdateFile,
    VOS_UINT8                          *pucEfFile,
    NAS_MSCC_PIF_USER_PLMN_LIST_STRU   *pstUPlmnInfo
)
{
    VOS_UINT8                           ucRecordLen;
    VOS_UINT16                          i;
    VOS_UINT16                          usRatMode;

    if (USIMM_GSM_EFPLMNSEL_ID == usUpdateFile)
    {
        ucRecordLen = 3;
    }
    else
    {
        ucRecordLen = 5;
    }

    for (i = 0; i < pstUPlmnInfo->usPlmnNum; i++)
    {
        pucEfFile[i*ucRecordLen]  = (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMcc >> 4));
        pucEfFile[i*ucRecordLen] <<= 4;
        pucEfFile[i*ucRecordLen] |= (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMcc >> 8));

        pucEfFile[(i*ucRecordLen)+1]  = (VOS_UINT8)(0x0f & pstUPlmnInfo->Plmn[i].ulMcc);
        if (0xF0 == (0xf0 & (pstUPlmnInfo->Plmn[i].ulMnc >>4)))
        {
            pucEfFile[(i*ucRecordLen)+1] |= (VOS_UINT8)(0xf0 & (pstUPlmnInfo->Plmn[i].ulMnc >>4));

            pucEfFile[(i*ucRecordLen)+2]  = (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMnc));
            pucEfFile[(i*ucRecordLen)+2] <<= 4;
            pucEfFile[(i*ucRecordLen)+2] |= (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMnc >>4));
        }
        else
        {
            pucEfFile[(i*ucRecordLen)+1] |= (VOS_UINT8)(0xf0 & (pstUPlmnInfo->Plmn[i].ulMnc <<4));

            pucEfFile[(i*ucRecordLen)+2]  = (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMnc >>4));
            pucEfFile[(i*ucRecordLen)+2] <<= 4;
            pucEfFile[(i*ucRecordLen)+2] |= (VOS_UINT8)(0x0f & (pstUPlmnInfo->Plmn[i].ulMnc >>8));
        }

        if (5 == ucRecordLen)
        {
            usRatMode = pstUPlmnInfo->ausPlmnRat[i];
            pucEfFile[(i*ucRecordLen)+3] = (VOS_UINT8)(usRatMode >> 8);
            pucEfFile[(i*ucRecordLen)+4] = (VOS_UINT8)(usRatMode & 0xFF);
        }
    }

    return;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_UpdateUsimPrefPlmn
 功能描述  : 更新USIM pref plmn
 输入参数  : VOS_UINT16                          usClientId,
             VOS_UINT8                           ucOpId,
             TAF_USER_PLMN_LIST_STRU            *pstUPlmnInfo,
             VOS_UINT16                          usUpdateFile
 输出参数  : 无
 返 回 值  : VOS_VOID
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_UpdateUsimPrefPlmn(
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    NAS_MSCC_PIF_USER_PLMN_LIST_STRU   *pstUPlmnInfo,
    VOS_UINT16                          usUpdateFile
)
{
    VOS_UINT8                          *pucEfFile;
    VOS_UINT16                          usEfLen;
    NAS_USIMM_UPDATEFILE_INFO_STRU      stSetFileInfo;

    PS_MEM_SET(&stSetFileInfo, 0x00, sizeof(stSetFileInfo));

    if ((USIMM_USIM_EFPLMNWACT_ID == usUpdateFile)
      || (USIMM_USIM_EFOPLMNWACT_ID == usUpdateFile)
      || (USIMM_USIM_EFHPLMNwACT_ID == usUpdateFile)
      || (USIMM_GSM_EFPLMNWACT_ID == usUpdateFile)
      || (USIMM_GSM_EFOPLMNWACT_ID == usUpdateFile)
      || (USIMM_GSM_EFHPLMNACT_ID == usUpdateFile))
    {
        usEfLen = (VOS_UINT16)(5 * (pstUPlmnInfo->usPlmnNum));
    }
    else
    {
        /*PlmnSel*/
        usEfLen = (VOS_UINT16)(3 * (pstUPlmnInfo->usPlmnNum));
    }

    pucEfFile = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_MMC, usEfLen);
    if (VOS_NULL_PTR == pucEfFile)
    {
        return VOS_FALSE;
    }

    PS_MEM_SET(pucEfFile, 0, usEfLen);

    NAS_MMC_FillPrefPlmnFile(usUpdateFile, pucEfFile, pstUPlmnInfo);

    stSetFileInfo.usEfId       = usUpdateFile;
    stSetFileInfo.ucRecordNum  = 0;
    stSetFileInfo.enAppType    = USIMM_GUTL_APP;
    stSetFileInfo.ucRsv        = 0;
    stSetFileInfo.ulEfLen      = usEfLen;
    stSetFileInfo.pucEfContent = pucEfFile;

    if(USIMM_API_SUCCESS != NAS_USIMMAPI_SetFileReq(WUEPS_PID_MMC, 0, &stSetFileInfo))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pucEfFile);
        return VOS_FALSE;
    }

    /* 启动定时器 */
    (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_WAIT_USIM_SET_FILE_CNF, TI_NAS_MMC_WAIT_USIM_SET_FILE_CNF_LEN);

    PS_MEM_FREE(WUEPS_PID_MMC, pucEfFile);
    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_ModPrefPlmnInfo
 功能描述  : 收到ID_MSCC_MMC_PREF_PLMN_SET_REQ消息的预处理
 输入参数  : MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
 输出参数  : 无
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_ModPrefPlmnInfo(
    MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
)
{
    NAS_MSCC_PIF_PREF_PLMN_OPERATE_STRU                    *pstPrefPlmnOpt           = VOS_NULL_PTR;
    NAS_MSCC_PIF_USER_PLMN_LIST_STRU                       *pstPlmnInfo              = VOS_NULL_PTR;
    VOS_UINT16                                              usMaxIndex;
    VOS_UINT16                                              usUpdateFile;
    VOS_UINT32                                              i;
    MMC_MODIFY_PLMN_INFO_STRU                              *pstPrefPlmnCtx;


    pstPrefPlmnOpt      = &(pstPrefPlmnSetReq->stPrefPlmnOpt);

    pstPlmnInfo = (NAS_MSCC_PIF_USER_PLMN_LIST_STRU *)PS_MEM_ALLOC(WUEPS_PID_MMC, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));
    if (VOS_NULL_PTR == pstPlmnInfo)
    {
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_MEM_ALLOC_FAIL;
    }

    PS_MEM_SET(pstPlmnInfo, 0x00, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));
    if (VOS_FALSE == NAS_MMC_GetPlmnInfo(pstPrefPlmnSetReq->enPrefPlmnParaType, pstPlmnInfo))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_GET_PLMNINFO_FAIL;
    }

    usMaxIndex = pstPlmnInfo->usPlmnNum;

    if ((pstPrefPlmnOpt->ulIndex > usMaxIndex)
     || (0 == pstPrefPlmnOpt->ulIndex))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_PLMN_INDEX_ERR;
    }

    NAS_MMC_PlmnId2NasStyle(&pstPrefPlmnOpt->stPlmn);
    NAS_MMC_PlmnId2Bcd(&pstPrefPlmnOpt->stPlmn);

    /* 判断修改的PLMN ID是否和当前已有的PLMN ID重复,如果重复则不允许设置 */
    for (i = 0; i < usMaxIndex ; i++)
    {
        /* 当前待修改项不进行判断 */
        if (i == (pstPrefPlmnOpt->ulIndex - 1))
        {
            continue;
        }

        /* 与其他的Pref PLMN信息重复时直接返回error  */
        if ((pstPlmnInfo->Plmn[i].ulMcc == pstPrefPlmnOpt->stPlmn.ulMcc)
          && (pstPlmnInfo->Plmn[i].ulMnc == pstPrefPlmnOpt->stPlmn.ulMnc)
          && ((pstPlmnInfo->ausPlmnRat[i] == pstPrefPlmnOpt->usRat)
          || (0 != (pstPlmnInfo->ausPlmnRat[i] & (pstPrefPlmnOpt->usRat)))))
        {

            PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
            return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SAME_PLMNID;
        }
    }

    /* 设置修改后的PLMN ID */
    pstPlmnInfo->Plmn[pstPrefPlmnOpt->ulIndex - 1].ulMcc = pstPrefPlmnOpt->stPlmn.ulMcc;
    pstPlmnInfo->Plmn[pstPrefPlmnOpt->ulIndex - 1].ulMnc = pstPrefPlmnOpt->stPlmn.ulMnc;
    pstPlmnInfo->ausPlmnRat[pstPrefPlmnOpt->ulIndex - 1] = pstPrefPlmnOpt->usRat;

    pstPrefPlmnCtx = NAS_MMC_GetPrefPlmnCtxAddr();
    pstPrefPlmnCtx->stPlmnId.ulMcc = pstPlmnInfo->Plmn[pstPrefPlmnOpt->ulIndex - 1].ulMcc ;
    pstPrefPlmnCtx->stPlmnId.ulMnc = pstPlmnInfo->Plmn[pstPrefPlmnOpt->ulIndex - 1].ulMnc;
    pstPrefPlmnCtx->PlmnRat        = pstPlmnInfo->ausPlmnRat[pstPrefPlmnOpt->ulIndex - 1];
    pstPrefPlmnCtx->usIndex        = (VOS_UINT16)(pstPrefPlmnOpt->ulIndex - 1);

    usUpdateFile = NAS_MMC_GetUpdateFileForPrefPlmn(pstPrefPlmnSetReq->enPrefPlmnParaType);

    if (VOS_TRUE != NAS_MMC_UpdateUsimPrefPlmn(pstPrefPlmnSetReq->stCtrl.usClientId,
                                                pstPrefPlmnSetReq->stCtrl.ucOpId,
                                                pstPlmnInfo,
                                                usUpdateFile))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_UPDATA_USIM_PLMN;
    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

    return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SUCCESS;
}
/*****************************************************************************
 函 数 名  : NAS_MMC_DelPrefPlmnInfo
 功能描述  : 收到ID_MSCC_MMC_PREF_PLMN_SET_REQ消息的预处理
 输入参数  : MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
 输出参数  : 无
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_DelPrefPlmnInfo(
    MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
)
{
    NAS_MSCC_PIF_PREF_PLMN_OPERATE_STRU                    *pstPrefPlmnOpt = VOS_NULL_PTR;
    NAS_MSCC_PIF_USER_PLMN_LIST_STRU                       *pstPlmnInfo    = VOS_NULL_PTR;
    VOS_UINT16                                              usIndex        = 0;
    VOS_UINT16                                              usMaxIndex;
    VOS_UINT16                                              usUpdateFile;
    MMC_MODIFY_PLMN_INFO_STRU                              *pstPrefPlmnCtx;

    pstPrefPlmnOpt      = &(pstPrefPlmnSetReq->stPrefPlmnOpt);

    pstPlmnInfo = (NAS_MSCC_PIF_USER_PLMN_LIST_STRU *)PS_MEM_ALLOC(WUEPS_PID_MMC, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));
    if (VOS_NULL_PTR == pstPlmnInfo)
    {
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_MEM_ALLOC_FAIL;
    }

    PS_MEM_SET(pstPlmnInfo, 0x00, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));

    if (VOS_FALSE == NAS_MMC_GetPlmnInfo(pstPrefPlmnSetReq->enPrefPlmnParaType, pstPlmnInfo))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_GET_PLMNINFO_FAIL;
    }

    usMaxIndex  = pstPlmnInfo->usPlmnNum;

    if (((VOS_UINT16)pstPrefPlmnOpt->ulIndex > usMaxIndex)
     || ((VOS_UINT16)pstPrefPlmnOpt->ulIndex == 0))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_PLMN_INDEX_ERR;
    }

    usIndex = (VOS_UINT16)(pstPrefPlmnOpt->ulIndex - 1);

    /* 设置PLMN ID为无效值 */
    pstPlmnInfo->Plmn[usIndex].ulMcc = NAS_MML_INVALID_MCC;
    pstPlmnInfo->Plmn[usIndex].ulMnc = NAS_MML_INVALID_MNC;
    pstPlmnInfo->ausPlmnRat[usIndex] = NAS_MMC_USIM_NO_RAT;

    pstPrefPlmnCtx = NAS_MMC_GetPrefPlmnCtxAddr();
    pstPrefPlmnCtx->stPlmnId.ulMcc   = pstPlmnInfo->Plmn[usIndex].ulMcc ;
    pstPrefPlmnCtx->stPlmnId.ulMnc   = pstPlmnInfo->Plmn[usIndex].ulMnc;
    pstPrefPlmnCtx->PlmnRat          = pstPlmnInfo->ausPlmnRat[usIndex];
    pstPrefPlmnCtx->usIndex          = usIndex;

    usUpdateFile = NAS_MMC_GetUpdateFileForPrefPlmn(pstPrefPlmnSetReq->enPrefPlmnParaType);

    if (VOS_TRUE != NAS_MMC_UpdateUsimPrefPlmn(pstPrefPlmnSetReq->stCtrl.usClientId,
                                               pstPrefPlmnSetReq->stCtrl.ucOpId,
                                               pstPlmnInfo,
                                               usUpdateFile))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_UPDATA_USIM_PLMN;
    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
    return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SUCCESS;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_AddPrefPlmnInfo
 功能描述  : 收到ID_MSCC_MMC_PREF_PLMN_SET_REQ消息的预处理
 输入参数  : MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
 输出参数  : 无
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_AddPrefPlmnInfo(
    MSCC_MMC_PREF_PLMN_SET_REQ_STRU    *pstPrefPlmnSetReq
)
{
    NAS_MSCC_PIF_PREF_PLMN_OPERATE_STRU                    *pstPrefPlmnOpt           = VOS_NULL_PTR;
    NAS_MSCC_PIF_USER_PLMN_LIST_STRU                       *pstPlmnInfo              = VOS_NULL_PTR;
    VOS_UINT16                                              usMaxIndex;
    VOS_UINT16                                              usUpdateFile;
    VOS_BOOL                                                bFindInvalidPlmn;
    VOS_UINT16                                              usFindIndex;
    VOS_BOOL                                                bFindSamePlmnId;
    VOS_UINT32                                              i;
    MMC_MODIFY_PLMN_INFO_STRU                              *pstPrefPlmnCtx;

    pstPrefPlmnOpt      = &(pstPrefPlmnSetReq->stPrefPlmnOpt);

    pstPlmnInfo = (NAS_MSCC_PIF_USER_PLMN_LIST_STRU *)PS_MEM_ALLOC(WUEPS_PID_MMC, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));
    if (VOS_NULL_PTR == pstPlmnInfo)
    {
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_MEM_ALLOC_FAIL;
    }

    PS_MEM_SET(pstPlmnInfo, 0, sizeof(NAS_MSCC_PIF_USER_PLMN_LIST_STRU));

    if (VOS_FALSE == NAS_MMC_GetPlmnInfo(pstPrefPlmnSetReq->enPrefPlmnParaType, pstPlmnInfo))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_GET_PLMNINFO_FAIL;
    }

    usMaxIndex        = pstPlmnInfo->usPlmnNum;

    bFindInvalidPlmn  = VOS_FALSE;
    usFindIndex       = 0;
    bFindSamePlmnId   = VOS_FALSE;

    NAS_MMC_PlmnId2NasStyle(&pstPrefPlmnOpt->stPlmn);
    NAS_MMC_PlmnId2Bcd(&pstPrefPlmnOpt->stPlmn);

    /* 搜寻空闲的PLMN ID */
    for (i = 0; i < usMaxIndex; i++)
    {
        if (VOS_FALSE == bFindInvalidPlmn)
        {
            if (VOS_FALSE == NAS_MMC_IsPlmnValid(&(pstPlmnInfo->Plmn[i])))
            {
                /* PLMN ID无效 */
                usFindIndex = (VOS_UINT16)i;
                bFindInvalidPlmn = VOS_TRUE;
            }
        }

        /* 增加相同PLMN的接入技术的判断 */
        if ((pstPlmnInfo->Plmn[i].ulMcc == pstPrefPlmnOpt->stPlmn.ulMcc)
          && (pstPlmnInfo->Plmn[i].ulMnc == pstPrefPlmnOpt->stPlmn.ulMnc)
          && ((pstPlmnInfo->ausPlmnRat[i] == pstPrefPlmnOpt->usRat)
          || (0 != (pstPlmnInfo->ausPlmnRat[i] & (pstPrefPlmnOpt->usRat)))))
        {
            bFindSamePlmnId = VOS_TRUE;
            break;
        }
    }

    if ((VOS_FALSE == bFindInvalidPlmn)
      || (VOS_TRUE == bFindSamePlmnId))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_PLMNID_INVAILD;
    }

    /* 设置增加后的PLMN ID值 */
    pstPlmnInfo->Plmn[usFindIndex].ulMcc = pstPrefPlmnOpt->stPlmn.ulMcc;
    pstPlmnInfo->Plmn[usFindIndex].ulMnc = pstPrefPlmnOpt->stPlmn.ulMnc;
    pstPlmnInfo->ausPlmnRat[usFindIndex] = pstPrefPlmnOpt->usRat;

    pstPrefPlmnCtx = NAS_MMC_GetPrefPlmnCtxAddr();
    pstPrefPlmnCtx->stPlmnId.ulMcc       = pstPlmnInfo->Plmn[usFindIndex].ulMcc ;
    pstPrefPlmnCtx->stPlmnId.ulMnc       = pstPlmnInfo->Plmn[usFindIndex].ulMnc;
    pstPrefPlmnCtx->PlmnRat              = pstPlmnInfo->ausPlmnRat[usFindIndex];
    pstPrefPlmnCtx->usIndex              = usFindIndex;

    usUpdateFile = NAS_MMC_GetUpdateFileForPrefPlmn(pstPrefPlmnSetReq->enPrefPlmnParaType);

    if (VOS_TRUE != NAS_MMC_UpdateUsimPrefPlmn(pstPrefPlmnSetReq->stCtrl.usClientId,
                                                pstPrefPlmnSetReq->stCtrl.ucOpId,
                                                pstPlmnInfo,
                                                usUpdateFile))
    {
        PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);
        return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_UPDATA_USIM_PLMN;
    }

    PS_MEM_FREE(WUEPS_PID_MMC, pstPlmnInfo);

    return NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SUCCESS;

}
/*****************************************************************************
 函 数 名  : NAS_MMC_RcvMsccPrefPlmnSetReq_PreProc
 功能描述  : 收到ID_MSCC_MMC_PREF_PLMN_SET_REQ消息的预处理
 输入参数  : pstMsg      - 消息内容
             ulEventType - 消息类型
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvMsccPrefPlmnSetReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PREF_PLMN_SET_REQ_STRU                        *pstPrefPlmnSetReq       = VOS_NULL_PTR;
    NAS_MSCC_PIF_PREF_PLMN_OPERATE_STRU                    *pstPrefPlmnOperate      = VOS_NULL_PTR;
    NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_ENUM_UINT32          enRslt;

    pstPrefPlmnSetReq       = (MSCC_MMC_PREF_PLMN_SET_REQ_STRU *)pstMsg;
    pstPrefPlmnOperate      = &(pstPrefPlmnSetReq->stPrefPlmnOpt);

    if (NAS_MSCC_PREF_PLMN_DEL_ONE == pstPrefPlmnOperate->enPrefPlmnOperType)
    {
        enRslt = NAS_MMC_DelPrefPlmnInfo(pstPrefPlmnSetReq);
    }
    else if (NAS_MSCC_PREF_PLMN_MODIFY_ONE == pstPrefPlmnOperate->enPrefPlmnOperType)
    {
        enRslt = NAS_MMC_ModPrefPlmnInfo(pstPrefPlmnSetReq);
    }
    else if (NAS_MSCC_PREF_PLMN_ADD_ONE == pstPrefPlmnOperate->enPrefPlmnOperType)
    {
        enRslt = NAS_MMC_AddPrefPlmnInfo(pstPrefPlmnSetReq);
    }
    else
    {
        enRslt = NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_FAILURE;
    }

    if (NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SUCCESS != enRslt)
    {
        NAS_MMC_SndMsccPrefPlmnSetCnf(enRslt);
    }

    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_RcvMsccPrefPlmnQueryReq_PreProc
 功能描述  : 收到ID_MSCC_MMC_PREF_PLMN_QUERY_REQ消息的预处理
 输入参数  : pstMsg      - 消息内容
             ulEventType - 消息类型
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvMsccPrefPlmnQueryReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PREF_PLMN_QUERY_REQ_STRU  *pstPrefPlmnQueryReq = VOS_NULL_PTR;
    MMC_MSCC_PREF_PLMN_QUERY_CNF_STRU  *pstPrefPlmnQueryRsp = VOS_NULL_PTR;
    VOS_UINT32                          ulPlmnNum;
    VOS_UINT32                          ulRemainPlmnNum;
    VOS_UINT32                          ulTotal;

    pstPrefPlmnQueryReq = (MSCC_MMC_PREF_PLMN_QUERY_REQ_STRU*)pstMsg;

    /* 设置分组结束标志和分组上报运营商个数: 待发送的运营商数少于一条消息中
        允许发送的运营商数，则设置结束标志并发送所有待发送运营商信息 */
    ulTotal = NAS_MMC_GetSpecificPlmnTblSize(pstPrefPlmnQueryReq->stCpolInfo.enPrefPLMNType);

    if (pstPrefPlmnQueryReq->stCpolInfo.ulFromIndex > ulTotal)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccPrefPlmnQueryReq_PreProc: abnormal request!");

        ulPlmnNum = 0;
    }
    else
    {
        ulRemainPlmnNum = ulTotal - pstPrefPlmnQueryReq->stCpolInfo.ulFromIndex;

        if (ulRemainPlmnNum < pstPrefPlmnQueryReq->stCpolInfo.ulPlmnNum)
        {
            ulPlmnNum   = ulRemainPlmnNum;
        }
        else
        {
            ulPlmnNum   = pstPrefPlmnQueryReq->stCpolInfo.ulPlmnNum;
        }
    }

    pstPrefPlmnQueryRsp = (MMC_MSCC_PREF_PLMN_QUERY_CNF_STRU *)PS_ALLOC_MSG(WUEPS_PID_MMC,
                    sizeof(MMC_MSCC_PREF_PLMN_QUERY_CNF_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstPrefPlmnQueryRsp)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_RcvMsccPrefPlmnQueryReq_PreProc ERROR:Memory Alloc Error for pstPrefPlmnQueryRsp ");
        return VOS_TRUE;
    }

    PS_MEM_SET(((VOS_UINT8*)pstPrefPlmnQueryRsp) + VOS_MSG_HEAD_LENGTH,
                0x00,
                sizeof(MMC_MSCC_PREF_PLMN_QUERY_CNF_STRU) - VOS_MSG_HEAD_LENGTH);

    /* 填充消息头 */
    pstPrefPlmnQueryRsp->stMsgHeader.ulSenderCpuId     = VOS_LOCAL_CPUID;
    pstPrefPlmnQueryRsp->stMsgHeader.ulSenderPid       = WUEPS_PID_MMC;
    pstPrefPlmnQueryRsp->stMsgHeader.ulReceiverCpuId   = VOS_LOCAL_CPUID;
    pstPrefPlmnQueryRsp->stMsgHeader.ulReceiverPid     = UEPS_PID_MSCC;
    pstPrefPlmnQueryRsp->stMsgHeader.ulLength          = sizeof(MMC_MSCC_PREF_PLMN_QUERY_CNF_STRU) - VOS_MSG_HEAD_LENGTH;
    pstPrefPlmnQueryRsp->stMsgHeader.ulMsgName         = ID_MMC_MSCC_PREF_PLMN_QUERY_CNF;

    /* 填充消息内容 */
    pstPrefPlmnQueryRsp->enCmdType                     = pstPrefPlmnQueryReq->enCmdType;
    pstPrefPlmnQueryRsp->ulFromIndex                   = pstPrefPlmnQueryReq->stCpolInfo.ulFromIndex;
    pstPrefPlmnQueryRsp->ulValidPlmnNum                = pstPrefPlmnQueryReq->stCpolInfo.ulValidPlmnNum;
    pstPrefPlmnQueryRsp->stPlmnName.bPlmnSelFlg        = pstPrefPlmnQueryReq->bPlmnSelFlg;

    /* 获取pref plmn */
    if (VOS_TRUE != NAS_MMC_QryPrefPlmnId(pstPrefPlmnQueryReq->stCpolInfo.enPrefPLMNType,
                            pstPrefPlmnQueryReq->stCpolInfo.ulFromIndex,
                            ulPlmnNum,
                            &pstPrefPlmnQueryRsp->stPlmnName))
    {
        pstPrefPlmnQueryRsp->enRslt = NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_FAILURE;
    }
    else
    {
        pstPrefPlmnQueryRsp->usPlmnNum            = (VOS_UINT16)pstPrefPlmnQueryRsp->stPlmnName.ulPlmnNum;
        pstPrefPlmnQueryRsp->stPlmnName.ulPlmnNum = ulPlmnNum;
        pstPrefPlmnQueryRsp->enRslt = NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_SUCCESS;
    }

    /* 发送消息 */
    PS_SEND_MSG(WUEPS_PID_MMC, pstPrefPlmnQueryRsp);

    return VOS_TRUE;
}

/*****************************************************************************
 函 数 名  : NAS_MMC_RcvUsimSetFileCnf_PreProc
 功能描述  : 收到USIMM_UPDATEFILE_CNF消息的预处理
 输入参数  : pstMsg      - 消息内容
             ulEventType - 消息类型
 输出参数  : 无
 返 回 值  : VOS_TRUE:消息处理完成
             VOS_FALSE:消息处理未完成
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   : 2015年02月04日
    作    者   : y00307564
    修改内容   : Iteration 8 新增
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvUsimSetFileCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    USIMM_UPDATEFILE_CNF_STRU          *pUsimSetFileCnf;
    VOS_UINT32                          ulEfId;
    VOS_UINT32                          ulRet;

    pUsimSetFileCnf = (USIMM_UPDATEFILE_CNF_STRU *)pstMsg;

    if (VOS_OK != USIMM_ChangePathToDefFileID(USIMM_GUTL_APP, pUsimSetFileCnf->stFilePath.ulPathLen, pUsimSetFileCnf->stFilePath.acPath, &ulEfId))
    {
        return VOS_TRUE;
    }

    /* 停止定时器 */
    if (NAS_MMC_TIMER_STATUS_RUNING == NAS_MMC_GetTimerStatus(TI_NAS_MMC_WAIT_USIM_SET_FILE_CNF))
    {
        if ((USIMM_GSM_EFPLMNSEL_ID    == ulEfId)
         || (USIMM_USIM_EFPLMNWACT_ID  == ulEfId)
         || (USIMM_GSM_EFPLMNWACT_ID   == ulEfId)
         || (USIMM_USIM_EFOPLMNWACT_ID == ulEfId)
         || (USIMM_GSM_EFOPLMNWACT_ID  == ulEfId)
         || (USIMM_USIM_EFHPLMNwACT_ID == ulEfId)
         || (USIMM_GSM_EFHPLMNACT_ID   == ulEfId))
        {
            NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_USIM_SET_FILE_CNF);

            /* 判断消息有效性 */
            if (VOS_OK != pUsimSetFileCnf->stCmdResult.ulResult)
            {
                ulRet = pUsimSetFileCnf->stCmdResult.ulErrorCode;
            }
            else
            {
                ulRet = VOS_OK;

                /* 更新内存中的变量 */
                NAS_MMC_UpdatePlmnInfo((VOS_UINT16)ulEfId);
            }

            /*向用户报告信息*/
            NAS_MMC_SndMsccPrefPlmnSetCnf(ulRet);

            return VOS_TRUE;
        }
        else
        {
            return VOS_FALSE;
        }
    }
    else
    {
        return VOS_FALSE;
    }
}

/*****************************************************************************
 函 数 名  : NAS_MMC_WaitUsimSetFileExpired_PreProc
 功能描述  : MMC等待Usim Set File超时
 输入参数  : ulEventType - 消息类型
             pstMsg      - 消息的首地址
 输出参数  : 无
 返 回 值  : VOS_TRUE:函数预处理完成
             VOS_FALSE:函数预处理未完成
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年02月08日
    作    者   : y00307564
    修改内容   : 新生成函数
*****************************************************************************/
VOS_UINT32  NAS_MMC_WaitUsimSetFileExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /*向用户报告信息*/
    NAS_MMC_SndMsccPrefPlmnSetCnf(NAS_MSCC_PIF_PREF_PLMN_OPER_RESULT_TIMEOUT);

    return VOS_TRUE;
}
/* Added by y00307564 for CDMA Iteration 8 2015-2-5 end */

/* Added by l00324781 for CDMA Iteration 10, 2015-4-3, begin */

VOS_UINT32  NAS_MMC_RcvMsccPlmnPriClassQueryReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_PLMN_PRI_CLASS_QUERY_REQ_STRU                 *pstPlmnPriClassQryReq;
    NAS_MSCC_PIF_PLMN_PRI_CLASS_LIST_INFO_STRU              stPlmnPriClassListInfo;
    NAS_MML_PLMN_ID_STRU                                    stTmpPlmnId;
    VOS_UINT8                                               i;

    pstPlmnPriClassQryReq = (MSCC_MMC_PLMN_PRI_CLASS_QUERY_REQ_STRU *)pstMsg;

    PS_MEM_SET(&stPlmnPriClassListInfo, 0x0, sizeof(NAS_MSCC_PIF_PLMN_PRI_CLASS_LIST_INFO_STRU));

    stPlmnPriClassListInfo.ucHighPlmnNum = NAS_MML_MIN(pstPlmnPriClassQryReq->stPlmnListInfo.ucHighPlmnNum, NAS_MSCC_PIF_MAX_HIGH_QUALITY_PLMN_NUM);

    for (i = 0; i < stPlmnPriClassListInfo.ucHighPlmnNum; i++)
    {
        stTmpPlmnId.ulMcc = pstPlmnPriClassQryReq->stPlmnListInfo.astHighPlmnID[i].ulMcc;
        stTmpPlmnId.ulMnc = pstPlmnPriClassQryReq->stPlmnListInfo.astHighPlmnID[i].ulMnc;

        stPlmnPriClassListInfo.astHighPlmnPriClassInfo[i].stPlmnId = pstPlmnPriClassQryReq->stPlmnListInfo.astHighPlmnID[i];

        /* first check if it's forbidden plmn */
        if (VOS_TRUE == NAS_MML_IsPlmnIdInForbidPlmnList(&stTmpPlmnId))
        {
            stPlmnPriClassListInfo.astHighPlmnPriClassInfo[i].ucIsForbidden = VOS_TRUE;
        }

        stPlmnPriClassListInfo.astHighPlmnPriClassInfo[i].enPriClass = NAS_MMC_GetPlmnPrioClass(&stTmpPlmnId);
    }

    stPlmnPriClassListInfo.ucLowPlmnNum = NAS_MML_MIN(pstPlmnPriClassQryReq->stPlmnListInfo.ucLowPlmnNum, NAS_MSCC_PIF_MAX_LOW_QUALITY_PLMN_NUM);

    for (i = 0; i < stPlmnPriClassListInfo.ucLowPlmnNum; i++)
    {
        stTmpPlmnId.ulMcc = pstPlmnPriClassQryReq->stPlmnListInfo.astLowPlmnID[i].ulMcc;
        stTmpPlmnId.ulMnc = pstPlmnPriClassQryReq->stPlmnListInfo.astLowPlmnID[i].ulMnc;

        stPlmnPriClassListInfo.astLowPlmnPriClassInfo[i].stPlmnId = pstPlmnPriClassQryReq->stPlmnListInfo.astLowPlmnID[i];

        /* first check if it's forbidden plmn */
        if (VOS_TRUE == NAS_MML_IsPlmnIdInForbidPlmnList(&stTmpPlmnId))
        {
            stPlmnPriClassListInfo.astLowPlmnPriClassInfo[i].ucIsForbidden = VOS_TRUE;
        }

        stPlmnPriClassListInfo.astLowPlmnPriClassInfo[i].enPriClass = NAS_MMC_GetPlmnPrioClass(&stTmpPlmnId);
    }

    stPlmnPriClassListInfo.enRat = pstPlmnPriClassQryReq->stPlmnListInfo.enRat;

    NAS_MMC_SndMsccQueryPlmnPriClassCnf(&stPlmnPriClassListInfo);

    return VOS_TRUE;
}


VOS_UINT32 NAS_MMC_RcvMsccAutoReselSetReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    MSCC_MMC_AUTO_RESEL_SET_REQ_STRU   *pstRcvMsg   = VOS_NULL_PTR;

    pstRcvMsg   = (MSCC_MMC_AUTO_RESEL_SET_REQ_STRU *)pstMsg;

    NAS_MML_SetUserAutoReselActiveFlg(pstRcvMsg->ucActiveFlg);

    return VOS_TRUE;
}

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA) && (FEATURE_ON == FEATURE_LTE)
/*****************************************************************************
Function Name   :   NAS_MMC_RcvMsccBgSearchReq_PreProc
Description     :   Process the message MSCC_MMC_BG_SEARCH_REQ from Mscc,
                    and call function send message MMC_LMM_BG_SEARCH_HRPD_REQ to lmm.
Input parameters:   MSCC_MMC_BG_SEARCH_REQ_STRU *pstMsccMmcBgSearchReq
                    VOS_UINT32 ulEventType
Outout parameters:  None
Return Value    :   VOS_FALSE: Message process success
                    VOS_TRUE: Message process fail
Modify History:
    1)  Date    :   2015-04-03
        Author  :   l00324781
        Modify content :    Create
*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvMsccBgSearchReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulPsServiceExist;
    MSCC_MMC_BG_SEARCH_REQ_STRU        *pstMsccMmcBgSearchReq = VOS_NULL_PTR;

    pstMsccMmcBgSearchReq = (MSCC_MMC_BG_SEARCH_REQ_STRU *)pstMsg;

    /* current, PS service is exist,abort search network, response mscc abort */
    ulPsServiceExist = NAS_MML_GetPsServiceExistFlg();
    if (VOS_TRUE == ulPsServiceExist)
    {
        NAS_MMC_SndMsccBgSearchCnf(MMC_LMM_BG_SRCH_RLT_ABORT, 0, VOS_NULL_PTR);
        return VOS_TRUE;
    }

    /* judge enRatMode,must be hrpd */
    if (VOS_RATMODE_HRPD != pstMsccMmcBgSearchReq->enRatMode)
    {
        NAS_ERROR_LOG(WUEPS_PID_MMC, "pstMsccMmcBgSearchReq->enRatMode is not HRPD");
        NAS_MMC_SndMsccBgSearchCnf(MMC_LMM_BG_SRCH_RLT_FAIL, 0, VOS_NULL_PTR);

        return VOS_TRUE;
    }
    /* send MMC_LMM_BG_SEARCH_HRPD_REQ message to lmm */
    NAS_MMC_SndLmmBgSearchHrpdReq();

    /* send request message, start protect timer */
    (VOS_VOID)NAS_MMC_StartTimer(TI_NAS_MMC_WAIT_LMM_BG_SEARCH_HRPD_CNF,
                                   TI_NAS_MMC_WAIT_LMM_BG_SEARCH_HRPD_CNF_LEN);

    return VOS_TRUE;
}

/*****************************************************************************
Function Name   :   NAS_MMC_RcvLmmBgSearchHrpdCnf_PreProc
Description     :   Process the message LMM_MMC_BG_SEARCH_HRPD_CNF from mscc,
                    and call function send message MMC_MSCC_SEARCH_CNF to mscc.
Input parameters:   LMM_MMC_BG_SEARCH_HRPD_CNF_STRU *pstLmmMmcBgSearchHrpdCnf
                    VOS_UINT32 ulEventType
Outout parameters:  None
Return Value    :   VOS_FALSE: Message process success
                    VOS_TRUE: Message process fail
Modify History:
    1)  Date    :   2015-04-03
        Author  :   l00324781
        Modify content :    Create
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvLmmBgSearchHrpdCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    LMM_MMC_BG_SEARCH_HRPD_CNF_STRU    *pstLmmMmcBgSearchHrpdCnf = VOS_NULL_PTR;

    /* recived response message, stop timer */
    (VOS_VOID)NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_LMM_BG_SEARCH_HRPD_CNF);

    pstLmmMmcBgSearchHrpdCnf = (LMM_MMC_BG_SEARCH_HRPD_CNF_STRU *)pstMsg;

    /* send MMC_MSCC_BG_SEARCH_HRPD_CNF message to mscc */
    NAS_MMC_SndMsccBgSearchCnf(pstLmmMmcBgSearchHrpdCnf->enRlst, pstLmmMmcBgSearchHrpdCnf->ucHrpdSysNum,
        pstLmmMmcBgSearchHrpdCnf->astHrpdSysList);

    return VOS_TRUE;
}

/*****************************************************************************
Function Name   :   NAS_MMC_RcvLmmStopBgSearchHrpdCnf_PreProc
Description     :   Process the message LMM_MMC_STOP_BG_SEARCH_HRPD_CNF from mscc,
                    and call function send message MMC_MSCC_STOP_SEARCH_CNF to mscc.
Input parameters:   LMM_MMC_STOP_BG_SEARCH_HRPD_CNF_STRU *pstLmmMmcStopBgSearchHrpdCnf
                    VOS_UINT32 ulEventType
Outout parameters:  None
Return Value    :   VOS_FALSE: Message process success
                    VOS_TRUE: Message process fail
Modify History:
    1)  Date    :   2015-04-03
        Author  :   l00324781
        Modify content :    Create
*****************************************************************************/
VOS_UINT32 NAS_MMC_RcvLmmStopBgSearchHrpdCnf_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* send MMC_MSCC_STOP_BG_SEARCH_HRPD_CNF message to mscc */
    NAS_MMC_SndMsccStopBgSearchCnf();

    return VOS_TRUE;
}

/*****************************************************************************
Function Name   :   NAS_MMC_RcvMsccStopBgSearchReq_PreProc
Description     :   Process the message MSCC_MMC_STOP_BG_SEARCH_REQ from Mscc,
                    and call function send message MMC_LMM_STOP_BG_SEARCH_HRPD_REQ to lmm.
Input parameters:   MSCC_MMC_STOP_BG_SEARCH_REQ_STRU *pstMsccMmcStopBgSearchReq
                    VOS_UINT32 ulEventType
Outout parameters:  None
Return Value    :   VOS_FALSE: Message process success
                    VOS_TRUE: Message process fails
Modify History:
    1)  Date    :   2015-04-03
        Author  :   l00324781
        Modify content :    Create
*****************************************************************************/
VOS_UINT32  NAS_MMC_RcvMsccStopBgSearchReq_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    /* Stop the Wait LMM Stop BG Search Protect timer */
    NAS_MMC_StopTimer(TI_NAS_MMC_WAIT_LMM_BG_SEARCH_HRPD_CNF);

    /* send MMC_LMM_STOP_BG_SEARCH_HRPD_REQ message to LMM */
    NAS_MMC_SndLmmStopBgSearchHrpdReq();

    /* 向LMM发送stopbgsearchreq消息不再启动保护定时器，如果没有收到cnf消息，
        上层mscc模块会自己超时，进入超时处理
    */


    return VOS_TRUE;
}

/*****************************************************************************
Function Name   :   NAS_MMC_WaitLmmBgSearchHrpdExpired_PreProc
Description     :   Process the timeout message, and response MCC_MSCC_BG_SEARCH_CNF to MSCC,
                    the result is fail.
Input parameters:   struct MsgCB *pstMsg
                    VOS_UINT32 ulEventType
Outout parameters:  None
Return Value    :   VOS_FALSE: Message process sucess
                    VOS_TRUE: Message process fail
Modify History:
    1)  Date    :   2015-04-03
        Author  :   l00324781
        Modify content :    Create
*****************************************************************************/
VOS_UINT32  NAS_MMC_WaitLmmBgSearchHrpdExpired_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    NAS_ERROR_LOG(WUEPS_PID_MMC, "NAS_MMC_WaitLmmBgSearchHrpdExpired_PreProc: timer expired!");

    /* protect timer expired, directly response fail to mscc */
    NAS_MMC_SndMsccBgSearchCnf(MMC_LMM_BG_SRCH_RLT_FAIL, 0, VOS_NULL_PTR);

    return VOS_TRUE;
}
#endif
/* Added by l00324781 for CDMA Iteration 10, 2015-4-3, end */


VOS_UINT32  NAS_MMC_RcvMmPlmnSearchInd_PreProc(
    VOS_UINT32                          ulEventType,
    struct MsgCB                       *pstMsg
)
{
    VOS_UINT32                          ulRet;

    /* 判断当前是否需要打断当前状态机 或进行消息的缓存 */
    ulRet = NAS_MMC_ProcMsgPrio(ulEventType, pstMsg);

    /* 需要打断当前状态机或进行消息的缓存,则表示预处理完成 */
    if (VOS_TRUE == ulRet)
    {
        return VOS_TRUE;
    }

    /* 需要进入后续状态机处理 */
    return VOS_FALSE;
}

/*lint -restore */

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

