////////////////////////////////////////////////////////////////////////////////
//  File Name: CSDClient.h
//
//  Functions:
//      客户端对象类.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////

#if !defined(_SDCLIENT_H)
#define _SDCLIENT_H

#include "SDCommon.h"
#include "SDThread.h"
#include "SDTerminalSdk.h"
#include "SDH264FileParse.h"

#define H264_FILE_FPS_DEF		25


class CSDClient
{
public:
	CSDClient();
	virtual ~CSDClient();

public:
	BOOL Start(char* strServerIp, UINT unDomainId, UINT unRoomId, UINT unUserId, USER_ONLINE_TYPE eUserType, BYTE bySendPosition, BYTE byRecvPosition);
	void Close();
	
private:
	static int SendThread(void *pParam1);
	static int SendThreadClose(void *pParam1);


	// SDK回调接口实现
	//来自底层的状态变更反馈（比如异步登录成功、异步登录失败、启动重连、重连成功、账号被顶下去等）
	static void SystemStatusNotifyCallback(void* pObject, STATUS_CHANGE_NOTIFY unStatus);

	// 收到服务器发来的视频
	static void RecvRemoteVideoCallback(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, VideoFrameInfor* pFrameInfo);

	// 收到服务器发来的音频
	static void RecvRemoteAudioCallback(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, AudioFrameInfor* pFrameInfo);


private:
	void*				 m_pTerminal;

	CSDThread*			 m_pSendThread;
	BOOL				 m_bClosed;
	CSDH264FilePase		 m_H264File;	
	FILE*				 m_pfRecvH264File;

};

#endif // !defined(_SDCLIENT_H)
