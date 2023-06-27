////////////////////////////////////////////////////////////////////////////////
//  File Name: main.cpp
//
//  Functions:
//      SDK测试DEMO.
//
//  History:
//  Date        Modified_By  Reason  Description	
//
////////////////////////////////////////////////////////////////////////////////
#include "SDCommon.h"
#include "SDLog.h"
#include "SDIniFile.h"
#include "SDClient.h"
#include "SDConsoleIFace.h"
#include "SDTerminalSdk.h"

#if defined(WIN32) && defined(_DEBUG_)
#include <afx.h>
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>
#endif



#define SECTION_CONFIG					"Config"
#define KEY_NAME_MEDIA_SERVER_IP		"ServerIp"			//远端服务器IP
#define KEY_NAME_ROOM_ID				"RoomId"			//房间号
#define KEY_DOMAIN_ID					"DomainId"			//服务器域ID	
#define KEY_NAME_UP_POSITION			"UpPosition"		//发送位置
#define KEY_NAME_DOWN_POSITION			"DownPosition"		//接收位置
#define KEY_NAME_H264_FILE				"H264FileUrl"		//充当测试数据到H264文件URL	
#define KEY_NAME_H264_FPS				"H264FileFps"		//充当测试数据到H264文件帧率
#define KEY_NAME_RECV_FILE_SAVE			"SaveRecvData"		//是否保存收到的数据到文件
#define KEY_NAME_FEC_REDUN_METHOD		"FecRedunMethod"	//FEC冗余方法
#define KEY_NAME_FEC_REDUN_RATIO		"FecRedunRatio"		//FEC冗余度比例
#define KEY_NAME_FEC_MIN_GROUP_SIZE		"FecMinGroupSize"	//FEC Min Group size
#define KEY_NAME_FEC_MAX_GROUP_SIZE		"FecMaxGroupSize"	//FEC Max Group size
#define KEY_NAME_FEC_ENABLE_NACK		"FecEnableNack"		//是否支持上下行NACK功能
#define KEY_NAME_SEND_NULL_DATA			"SendNullDataEnable"		//是否发送虚拟数据，而非读取码流
#define KEY_NAME_NULL_DATA_BITRATE		"NullDataBitrateKbps"		//发送虚拟数据的码率

CSDClient	g_Client;
char		g_acH264FileUrl[1024];
int			g_nH264Fps					= H264_FILE_FPS_DEF;
FEC_REDUN_TYPE g_eRedunMethod			= FEC_AUTO_REDUN;
UINT		g_unRedunRatio				= 30;
UINT		g_unFecMinGroupSize			= 16;
UINT		g_unFecMaxGroupSize			= 64;
BOOL		g_bEnableNack				= TRUE;
BOOL		g_bSaveRecvData				= TRUE;
BOOL		g_bSendNullData				= FALSE;
UINT		g_unNullDataBitrateKbps		= 2000;

//主函数
int main(int argc, char *argv[])
{
	int nRet = 0;

	//初始化DEMO日志模块
	char strIniFileName[MAX_PATH];
	GetSameExeFile(strIniFileName, ".ini");
	SDLOG_INIT("./log", SD_LOG_LEVEL_INFO, strIniFileName);


	//读取配置
	CSDIniFile *pIniFile = new CSDIniFile;
	pIniFile->ReadIniFile(strIniFileName);

	//服务器IP
	char strServerIp[64];
	memset(strServerIp, 0x0, sizeof(strServerIp));
	pIniFile->SDGetProfileString(SECTION_CONFIG, KEY_NAME_MEDIA_SERVER_IP, strServerIp, 64);
	//用户类型
	USER_ONLINE_TYPE eUserType = USER_ONLINE_TYPE_AV_SEND_RECV;
	//用户ID
	UINT unSec = 0;
	UINT unUsec = 0;
	SD_GetCurrentTime(unSec, unUsec);
	UINT unSeed = unSec^unUsec;
	SD_srandom(unSeed);
	UINT unUserId = SD_random32();

	//服务器域ID
	UINT unDomainId = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_DOMAIN_ID, 1);

	//房间ID
	UINT unRoomId = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_ROOM_ID, 1000);

	//发送位置
	BYTE bySendPosition = (BYTE)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_UP_POSITION, 0);
	//接收位置
	BYTE byRecvPosition = (BYTE)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_DOWN_POSITION, 255);


	//充当测试数据的H264文件路径
	memset(g_acH264FileUrl, 0x0, sizeof(g_acH264FileUrl));
	pIniFile->SDGetProfileString(SECTION_CONFIG, KEY_NAME_H264_FILE, g_acH264FileUrl, sizeof(g_acH264FileUrl));
	g_nH264Fps = pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_H264_FPS, H264_FILE_FPS_DEF);
	if ((g_nH264Fps <= 0) || (g_nH264Fps > 60))
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Please setup right h264 fps(%d), will use default:%d!", g_nH264Fps, H264_FILE_FPS_DEF);
		g_nH264Fps = H264_FILE_FPS_DEF;
	}

	//传输FEC参数读取
	//FEC方法
	UINT unFecRedunMethod = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_REDUN_METHOD, FEC_AUTO_REDUN);
	g_eRedunMethod = unFecRedunMethod != 0 ? FEC_FIX_REDUN : FEC_AUTO_REDUN;

	//FEC冗余比率
	UINT unFecRedundancy = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_REDUN_RATIO, 30);
	if (unFecRedundancy > 100)
	{
		unFecRedundancy = 100;
	}
	g_unRedunRatio = unFecRedundancy;

	//FEC Min Group大小 
	UINT unFecMinGroupSize = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_MIN_GROUP_SIZE, 16);
	if (unFecMinGroupSize > 64)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group min size:%d invalid [8, 64]!", unFecMinGroupSize);
		unFecMinGroupSize = 64;
	}

	if (unFecMinGroupSize < 8)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group min size:%d invalid [8, 64]!", unFecMinGroupSize);
		unFecMinGroupSize = 8;
	}
	g_unFecMinGroupSize = unFecMinGroupSize;


	//FEC Max Group大小 
	UINT unFecMaxGroupSize = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_MAX_GROUP_SIZE, 64);
	if (unFecMaxGroupSize > 64)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group max size:%d invalid [8, 64]!", unFecMaxGroupSize);
		unFecMaxGroupSize = 64;
	}

	if (unFecMaxGroupSize < 8)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_WARNING, "Fec Group max size:%d invalid [8, 64]!", unFecMaxGroupSize);
		unFecMaxGroupSize = 8;
	}
	g_unFecMaxGroupSize = unFecMaxGroupSize;

	if (g_unFecMaxGroupSize < g_unFecMinGroupSize)
	{
		delete pIniFile;
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Fec Group min size:%d  max size:%d  invalid!", g_unFecMinGroupSize, g_unFecMaxGroupSize);
		SDLOG_CLOSE();
		return FALSE;
	}

	//NACK设置
	UINT unEnableNack = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_FEC_ENABLE_NACK, TRUE);
	if (unEnableNack != 0)
	{
		g_bEnableNack = TRUE;
	}
	else
	{
		g_bEnableNack = FALSE;
	}

	//是否保存收到的文件
	UINT unSaveRecvData = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_RECV_FILE_SAVE, FALSE);
	if (unSaveRecvData != 0)
	{
		g_bSaveRecvData = TRUE;
	}
	else
	{
		g_bSaveRecvData = FALSE;
	}

	//是否发送虚拟数据
	UINT unSendNullData = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_SEND_NULL_DATA, FALSE);
	if (unSendNullData != 0)
	{
		g_bSendNullData = TRUE;
		//虚拟数据码率
		g_unNullDataBitrateKbps = (UINT)pIniFile->SDGetProfileInt(SECTION_CONFIG, KEY_NAME_NULL_DATA_BITRATE, 8000);

		SDLOG_PRINTF("CAVProcess", SD_LOG_LEVEL_INFO, "Will Send NULL data instead! bitrate:%d kbps", g_unNullDataBitrateKbps);
	}
	else
	{
		g_bSendNullData = FALSE;
	}


	SDLOG_PRINTF("CAVProcess", SD_LOG_LEVEL_INFO, "FecMethod:%s  RedunRatio:%d  MinGroup:%d  MaxGroup:%d  NackEnable:%s!", g_eRedunMethod == FEC_AUTO_REDUN ? "Auto":"Fix", 
				g_unRedunRatio, g_unFecMinGroupSize, g_unFecMaxGroupSize, g_bEnableNack == TRUE ? "Y":"N");

	delete pIniFile;
	
	//配置有效性校验
	if (strServerIp[0] == 0)
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Please setup right server ip!");
		SDLOG_CLOSE();
		return FALSE;
	}

	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "ServerIp:%s Room:%u Domain:%d UserId:%u  UpPosition:%d  DownPosition:%d!", 
				strServerIp, unRoomId, unDomainId, unUserId, bySendPosition, byRecvPosition);
	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "H264 File: %s  Fps:%d!", g_acH264FileUrl, g_nH264Fps);


	//启动测试服务
	if (!g_Client.Start(strServerIp, unDomainId, unRoomId, unUserId, eUserType, bySendPosition, byRecvPosition))
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_ERROR, "Test start fail...");
		SDLOG_CLOSE();
		return FALSE;
	}
	else
	{
		SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "sdk test start success...");
	}

	//循环
	CSDConsleIFace::RunCommandLine("sdk_test");

	g_Client.Close();
	

	SDLOG_PRINTF("Test", SD_LOG_LEVEL_INFO, "sdk test over success...");
	SDLOG_CLOSE();

	return TRUE;
}

