////////////////////////////////////////////////////////////////////////////////
//  File Name: CSDClient.cpp
//
//  Functions:
//      客户端对象类.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////

#include "SDClient.h"


extern char		g_acH264FileUrl[1024];
extern int		g_nH264Fps;
extern FEC_REDUN_TYPE g_eRedunMethod;
extern UINT		g_unRedunRatio;
extern UINT		g_unFecMinGroupSize;
extern UINT		g_unFecMaxGroupSize;
extern BOOL		g_bEnableNack;
extern BOOL		g_bSaveRecvData;
extern BOOL		g_bSendNullData;
extern UINT		g_unNullDataBitrateKbps;

#define MAX_PAYLOAD_LEN_AUDIO	2048

//虚拟数据时附带的头部，用于绕开数据合法性检测
static unsigned char 	g_byTestStreamHead[] 	= {0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x28, 0xAC, 0x4C, 0x22, 0x07, 0x80, 0x8B, 0xF7, 0x08, 0x00, 0x00, 0x03, 0x00, 0x08, 0x00, 0x00, 0x03, 0x01, 0xE4, 0x78, 0xC1, 0x90, 0x8C, 0x00, 0x00, 0x00, 0x01, 0x68, 0xEE, 0x32, 0xC8, 0xB0, 0x00, 0x00, 0x00, 0x01, 0x65};

CSDClient::CSDClient()
{
	m_pSendThread = NULL;
	//客户端SDK环境初始化
	SDTerminal_Enviroment_Init("./log", LOG_OUTPUT_LEVEL_INFO);
	m_pTerminal = SDTerminal_Create();
	m_bClosed = TRUE;
	m_pfRecvH264File = NULL;
}

CSDClient::~CSDClient()
{
	SDTerminal_Delete(&m_pTerminal);
	SDTerminal_Enviroment_Free();
	if (m_pfRecvH264File)
	{
		fclose(m_pfRecvH264File);
		m_pfRecvH264File = NULL;
	}
}


BOOL CSDClient::Start(char* strServerIp, UINT unDomainId, UINT unRoomId, UINT unUserId, USER_ONLINE_TYPE eUserType, BYTE bySendPosition, BYTE byRecvPosition)
{
	if ((g_acH264FileUrl[0]) && (g_bSendNullData == FALSE))
	{
		m_H264File.Start(g_acH264FileUrl, g_nH264Fps, TRUE);
	}
	else
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_INFO, "File Video test is disable!");	
	}

	m_bClosed = FALSE;

	if (g_bSaveRecvData)
	{
		m_pfRecvH264File = fopen("recv.h264", "wb");
		if (m_pfRecvH264File == NULL)
		{
			SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Open file for recv bitstream save failed!");
		}
	}


	//设置传输相关参数
	SDTerminal_SetVideoFrameRateForSmoother(m_pTerminal, g_nH264Fps);

	UINT unJitterBuffDelay = 200;
	SDTerminal_SetTransParams(m_pTerminal, unJitterBuffDelay, g_eRedunMethod, g_unRedunRatio, g_unFecMinGroupSize, g_unFecMaxGroupSize, g_bEnableNack);

	//设置回调接口
	SDTerminal_SetSystemStatusNotifyCallback(m_pTerminal, SystemStatusNotifyCallback, this);
	SDTerminal_SetRecvRemoteVideoCallback(m_pTerminal, RecvRemoteVideoCallback, this);
	SDTerminal_SetRecvRemoteAudioCallback(m_pTerminal, RecvRemoteAudioCallback, this);

	if (g_bSendNullData == FALSE)
	{
		//发送文件时，使用外部提供时间戳（来自文件本身），避免时间戳受读文件开销影响
		SDTerminal_SetUseInternalTimeStamp(m_pTerminal, FALSE);
	}

	//登录服务器
	int nRet = SDTerminal_Online(m_pTerminal, strServerIp, unDomainId, unUserId, unRoomId, eUserType);
	if (nRet < 0)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "SDTerminal_Online Failed return:%d!", nRet);
		m_bClosed = TRUE;
		m_H264File.Stop();
		return FALSE;
	}

	//请求向指定位置发送音视频
	nRet = SDTerminal_OnPosition(m_pTerminal, &bySendPosition);
	if (nRet < 0)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "SDTerminal_OnPosition to:%d Failed return:%d!", bySendPosition, nRet);
		m_bClosed = TRUE;
		m_H264File.Stop();
		SDTerminal_Offline(m_pTerminal);
		return FALSE;
	}

	//设置接收掩码
	UINT unMask = 0xFFFFFFFF;
	if (byRecvPosition < MAX_SUPPORT_POSITION_NUM)
	{
		unMask = 0x1 << (byRecvPosition);
	}
	else
	{
		unMask = 0;
	}
	SDTerminal_SetAvDownMasks(m_pTerminal, unMask, unMask);

	//创建发送线程
	char acThreadName[128];
	sprintf(acThreadName, "SendThread");
	m_pSendThread = new CSDThread(acThreadName);

	if (!m_pSendThread->CreateThread(SendThread, SendThreadClose, (void*)this))
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_ERROR, "CreateThread Failed for Send Thread!");
		m_bClosed = TRUE;
		m_H264File.Stop();
		SDTerminal_Offline(m_pTerminal);
		delete m_pSendThread;
		m_pSendThread = NULL;
		return FALSE;
	}
	return TRUE;
}


void CSDClient::Close()
{
	m_bClosed = TRUE;
	if (m_pSendThread)
	{
		m_pSendThread->CloseThread();
		delete m_pSendThread;
		m_pSendThread = NULL;
	}

	SDTerminal_Offline(m_pTerminal);
	m_H264File.Stop();
	if (m_pfRecvH264File)
	{
		fclose(m_pfRecvH264File);
		m_pfRecvH264File = NULL;
	}
}


int CSDClient::SendThreadClose(void* pParam1)
{
	CSDClient* pMain = (CSDClient*) pParam1;

	return 0;
}

#define MAX_FRAME_SIZE	(1920*1080)

int CSDClient::SendThread(void *pObject1)
{
	CSDClient* pClient = (CSDClient*)pObject1;

	BYTE* pFrame = (BYTE*)calloc(MAX_FRAME_SIZE, sizeof(BYTE));
	if (pFrame == NULL)
	{
		SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_ERROR, "malloc frame buff failed!");
		return 0;
	}

	UINT unFrameSizeByte = 0;
	if (g_bSendNullData == TRUE)
	{
		//计算每帧数据的大小
		unFrameSizeByte = ((g_unNullDataBitrateKbps / 8) / g_nH264Fps) * 1000;
		if (unFrameSizeByte > MAX_FRAME_SIZE)
		{
			SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_WARNING, "Null data bitrate:%d is too large!", g_unNullDataBitrateKbps);
			unFrameSizeByte = MAX_FRAME_SIZE;

		}
		
		memcpy(pFrame, g_byTestStreamHead, sizeof(g_byTestStreamHead));
	}

	int nFrameInterval = 1000 / g_nH264Fps;
	UINT unFrameNo = 0;
	long nPrevSendTime = 0;
	UINT unDts = 0;
	UINT unPts = 0;
	long nFirstTime = 0;

	while(pClient->m_bClosed == FALSE)
	{
		long nStartTime = SD_GetTickCount();

		if (g_bSendNullData == FALSE)
		{
			UINT unFrameLen = pClient->m_H264File.ReadH264RawFrame(pFrame, MAX_FRAME_SIZE, &unDts, &unPts);
			if (unFrameLen)
			{
				//SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_INFO, "Read a frme:%d!", unFrameLen);
				if (nFirstTime == 0)
				{
					nFirstTime = nStartTime;
				}
				unDts = (UINT)(nStartTime - nFirstTime);
				SDTerminal_SendVideoData(pClient->m_pTerminal, pFrame, unFrameLen, unDts);
			}
		}
		else
		{
			SDTerminal_SendVideoData(pClient->m_pTerminal, pFrame, unFrameSizeByte, 0);
		}


		unFrameNo++;

		long nCurrTime = SD_GetTickCount();
		long nUsedTime = nCurrTime - nStartTime;

		long nSleepTime = nFrameInterval - nUsedTime;
		if (nSleepTime < 0)
		{
			nSleepTime = 0;
		}
		
		//SDLOG_PRINTF(m_strTag, SD_LOG_LEVEL_INFO, "Send a frme:%d!  send interval:%d", unFrameNo, nCurrTime - nPrevSendTime);
		nPrevSendTime = nCurrTime;
		SD_Sleep(nSleepTime);
	}

	if (pFrame)
	{
		free(pFrame);
	}

	return 0;
}



// SDK回调接口实现
// 【注意事项】
//	1、通知型回调函数中应尽可能快的退出，不进行耗时操作，不调用SDTerminal系列API接口。
//  2、数据型回调函数中允许进行解码处理

//通知型回调：来自底层的状态变更反馈（比如异步登录成功、异步登录失败、启动重连、重连成功、账号被顶下去等）
void CSDClient::SystemStatusNotifyCallback(void* pObject, STATUS_CHANGE_NOTIFY unStatus)
{
	switch (unStatus) 
	{
	case STATUS_NOTIFY_EXIT_AUTH_FAILED:
		// 用户账号(uid)在登录失败：鉴权失败。本回调消息属于严重级别，应通知上层
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "online auth failed, offline...");
		break;
	case STATUS_NOTIFY_EXIT_KICKED:
		// 用户账号(uid)在其他位置登录，被踢出房间。本回调消息属于严重级别，应通知上层
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "kicked by other user, offline...");
		break;
	case STATUS_NOTIFY_RECON_START:
		// 客户端网络掉线，内部开始自动重连。可忽略本消息，若仅做日志提醒
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "starting reconnect");
		break;
	case STATUS_NOTIFY_RECON_SUCCESS:
		// 内部自动重连网络成功。可忽略本消息，若仅做日志提醒
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "reconnect success");
		break;
	}
}

//数据型回调：收到服务器发来的视频
void CSDClient::RecvRemoteVideoCallback(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, VideoFrameInfor* pFrameInfo)
{
	CSDClient* pClient = (CSDClient*)pObject;
	if (pClient->m_pfRecvH264File)
	{
		fwrite(data, sizeof(unsigned char), unLen, pClient->m_pfRecvH264File);
	}
	
}

//数据型回调：收到服务器发来的音频
void CSDClient::RecvRemoteAudioCallback(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, AudioFrameInfor* pFrameInfo)
{
	CSDClient* pClient = (CSDClient*)pObject;
	return;
}

