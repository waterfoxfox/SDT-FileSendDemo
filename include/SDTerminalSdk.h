//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：客户端对外SDK DLL接口
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-4-26
//**************************************************************************//

#ifndef _SD_TERMINAL_SDK_H_
#define _SD_TERMINAL_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DLL_EXPORTS
    #ifdef __GNUC__
      #define DLLIMPORT_SDT_SDK __attribute__ ((dllexport))
    #else
      #define DLLIMPORT_SDT_SDK __declspec(dllexport) 
    #endif
  #else
    #ifdef __GNUC__
      #define DLLIMPORT_SDT_SDK 
    #else
      #define DLLIMPORT_SDT_SDK
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define DLLIMPORT_SDT_SDK __attribute__ ((visibility ("default")))
  #else
    #define DLLIMPORT_SDT_SDK
  #endif
#endif

#ifdef __APPLE__
#ifndef OBJC_BOOL_DEFINED
typedef int BOOL;
#endif 
#else
#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//一个房间中支持最大位置数
#define     MAX_SUPPORT_POSITION_NUM	    12

//日志输出的级别
typedef enum LOG_OUTPUT_LEVEL
{
	LOG_OUTPUT_LEVEL_DEBUG = 1,
	LOG_OUTPUT_LEVEL_INFO,
	LOG_OUTPUT_LEVEL_WARNING,
	LOG_OUTPUT_LEVEL_ERROR,
	LOG_OUTPUT_LEVEL_ALARM,
	LOG_OUTPUT_LEVEL_FATAL,
	LOG_OUTPUT_LEVEL_NONE
} LOG_OUTPUT_LEVEL;

//用户类型
typedef enum USER_ONLINE_TYPE
{
    //其他类型，保留
    USER_ONLINE_TYPE_OTHER = 0,     
    //音视频收发类型
    USER_ONLINE_TYPE_AV_SEND_RECV,  
    //仅接收音视频类型
    USER_ONLINE_TYPE_AV_RECV_ONLY, 
    //仅发送音视频类型    
    USER_ONLINE_TYPE_AV_SEND_ONLY   
} USER_ONLINE_TYPE;


//客户端音频编解码类型
typedef enum
{
	CLIENT_AUDIO_TYPE_AAC = 0,
	CLIENT_AUDIO_TYPE_G711,
	CLIENT_AUDIO_TYPE_OPUS
} CLIENT_AUDIO_CODEC_TYPE;


//客户端视频编解码类型
typedef enum
{
	CLIENT_VIDEO_TYPE_H264 = 0,
	CLIENT_VIDEO_TYPE_HEVC,
} CLIENT_VIDEO_CODEC_TYPE;


//FEC冗余方法
typedef enum FEC_REDUN_TYPE
{
	//自动冗余度
	FEC_AUTO_REDUN = 0,
	//固定冗余度
	FEC_FIX_REDUN
} FEC_REDUN_TYPE;


//客户端通知外层状态变更
typedef enum STATUS_CHANGE_NOTIFY
{
	// [保留未使用]
	STATUS_NOTIFY_EXIT_NORMAL			= 0,
	// 异常退出 未知原因[保留未使用]
	STATUS_NOTIFY_EXIT_ABNORMAL			= 1,
    // 底层网络原因与服务器断开[保留未使用]
    STATUS_NOTIFY_EXIT_LOSTCONNECT		= 2,
    // 用户账号在其他位置登录，被KICKED
    STATUS_NOTIFY_EXIT_KICKED			= 3,
    // 登录成功[异步登录结果反馈]
    STATUS_NOTIFY_ONLINE_SUCCESS		= 4,
    // 登录失败[异步登录结果反馈]
    STATUS_NOTIFY_ONLINE_FAILED			= 5,
    // 客户端掉线，内部开始自动重连
    STATUS_NOTIFY_RECON_START			= 6,
    // 内部自动重连成功
    STATUS_NOTIFY_RECON_SUCCESS			= 7,
	// 某客户端开始发布
	STATUS_NOTIFY_ONPOSITION			= 8,
	// 某客户端停止发布
	STATUS_NOTIFY_OFFPOSITION			= 9,
	// 因其他客户端抢占位置，本客户端的上发流将被丢弃
	STATUS_NOTIFY_MYPOSITION_PAUSE		= 10,
	// 因其他客户端离开位置，本客户端的上发流将被恢复
	STATUS_NOTIFY_MYPOSITION_RESUME		= 11,
	// 用户账号鉴权失败
	STATUS_NOTIFY_EXIT_AUTH_FAILED		= 12

} STATUS_CHANGE_NOTIFY;



//码率自适应模式
typedef enum AUTO_BITRATE_TYPE
{
	//优先降低帧率，其次降低码率
	AUTO_BITRATE_TYPE_ADJUST_DISABLE = 0,
	//优先降低帧率，其次降低码率
	AUTO_BITRATE_TYPE_ADJUST_FRAME_FIRST = 1,
	//优先降低码率，其次降低帧率
	AUTO_BITRATE_TYPE_ADJUST_BITRATE_FIRST = 2
} AUTO_BITRATE_TYPE;



//送外层视频码流时附带的信息
typedef struct VideoFrameInfor
{
	unsigned int unWidth;
	unsigned int unHeight;
	unsigned int unFps;
	BOOL bPacketLost;
	BOOL bKeyFrame;
	BOOL bInfoUpdated;
	//注意SPS\PPS中不含起始码
	unsigned char bySps[512];
	unsigned int unSpsSize;
	unsigned char byPps[512];
	unsigned int unPpsSize;
}VideoFrameInfor;


//送外层音频码流时附带的信息
typedef struct AudioFrameInfor
{
	unsigned int unCodecType;
	unsigned int unSampleRate;
	unsigned int unChannelNum;
	unsigned int unFrameNo;
	BOOL bInfoUpdated;
}AudioFrameInfor;





//回调函数
// 【注意事项】
//	1、通知型回调函数中应尽可能快的退出，不进行耗时操作，不调用SDTerminal系列API接口。
//  2、数据型回调函数中允许进行解码处理

//来自底层的状态变更反馈（比如异步登录成功、异步登录失败、启动重连、重连成功、账号被顶下去等）
typedef void (*SystemStatusNotifyFunc)(void* pObject, STATUS_CHANGE_NOTIFY unStatus);

// 收到服务器发来的视频
typedef  void (*RecvRemoteVideoFunc)(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, VideoFrameInfor* pFrameInfo);

// 收到服务器发来的音频
typedef  void (*RecvRemoteAudioFunc)(void* pObject, unsigned char ucPosition, unsigned char* data, unsigned int unLen, unsigned int unPTS, AudioFrameInfor* pFrameInfo);


//当使用模块的码率自适应评估时，评估结果由本接口送出，外层负责具体的实施
//比如unFrameDropInterval=2表示每2帧丢1帧。
//比如fBitrateRatio=0.8表示需要将码率降低为原始码率的0.8倍
//外层需同时响应帧率调整和码率调整
//当外层执行了码率自适应动作时，返回TRUE
typedef BOOL (*AutoBitrateNotifyFunc)(void* pObject, unsigned int unFrameDropInterval, float fBitrateRatio);

//响应远端的IDR请求，注意本接口为同步调用方式，因此外层不应在其中执行耗时操作，应尽快返回
typedef BOOL (*RemoteIdrRequestNotifyFunc)(void* pObject);

//房间内当前时刻位置和音视频发布状态信息周期性通知上层，外层不应在其中执行耗时操作，应尽快返回
typedef  void (*RoomInfoNotifyFunc)(void* pObject, unsigned int *punOnposUserId, unsigned int *punUserPos, unsigned int *punAudioOn, unsigned int *punVideoOn, unsigned int unOnposUserNum);






////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												SDTerminal SDK接口（基础API）
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/***
* 环境初始化，系统只需调用一次，主要用于SRT环境以及日志模块的初始化
* @param: outputPath表示日志存放路径，支持相对路径和绝对路径，若目录不存在将自动创建
* @param: outputLevel表示日志输出的级别，只有等于或者高于该级别的日志输出到文件，取值范围参考LOG_OUTPUT_LEVEL
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_Enviroment_Init(const char* outputPath, int outputLevel);

DLLIMPORT_SDT_SDK void  SDTerminal_Enviroment_Free();

/***
* 创建SDTerminal
* @return: 返回模块指针，为NULL则失败
*/
DLLIMPORT_SDT_SDK void*  SDTerminal_Create();

/***
* 销毁SDTerminal，并设置指针为NULL，使用者应该做好与其他API之间的互斥保护
* @param ppTerminal: 模块指针的指针
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_Delete(void** ppTerminal);


/***
* 登录(同步方式接口)
* @param pTerminal: 模块指针
* @param strMediaServerIp: 服务器IP地址
* @param unDomainId: 服务器域号，默认1
* @param unUid: 唯一的用户ID
* @param unRoomId: 房间号
* @param eUserType: 用户类别
* @return: <0为失败错误码，>=0为成功
*/
DLLIMPORT_SDT_SDK int  SDTerminal_Online(
	void* pTerminal,
    char* strMediaServerIp,
    unsigned int unDomainId,
    unsigned int unUid, 
    unsigned int unRoomId, 
    USER_ONLINE_TYPE eUserType
    );
    
    
/***
* 下线。若用户在位置上，将同时从位置上下来。
* @param pTerminal: 模块指针
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_Offline(void* pTerminal);


/***
* 上到指定的位置，准备发送音视频【必须Online成功后调用】
* @param pTerminal: 模块指针
* @param pucPosition: 期望上到的位置。
* 当指定位置[0，MAX_SUPPORT_POSITION_NUM）时，将抢占式占据指定位置。
* 当指定位置为255时，表示由系统分配可用的位置，同时分配结果将通过pucPosition返回
* @return: <0为失败错误码，>=0为成功
*/
DLLIMPORT_SDT_SDK int  SDTerminal_OnPosition(void* pTerminal, unsigned char *pucPosition);


/***
* 从位置上下来
* @param pTerminal: 模块指针
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_OffPosition(void* pTerminal);


/***
* 发送音频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧音频码流【必须输入ADTS流】
* @param nLen: 数据长度
* @param unDts: SDTerminalP2P_SetUseInternalTimeStamp指定为用户提供时间戳时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SendAudioData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts);

/***
* 发送视频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧视频码流，内部自带拆分功能【必须输入带起始码的码流】
* @param nLen: 数据长度
* @param unDts: SDTerminalP2P_SetUseInternalTimeStamp指定为用户提供时间戳时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SendVideoData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts);


/***
* 设置上行传输参数，若不调用本API将使用默认传输参数【必须Online之前调用】
* @param pTerminal: 模块指针
* @param unJitterBuffDelay: 接收缓存时间，单位ms
* @param eFecRedunMethod: FEC 冗余调整方法
* @param unFecRedunRatio: FEC 固定冗余度时对应的上行冗余比率，比如设置为30，则表示使用30%冗余。
* @param unFecMinGroupSize: FEC 分组的下限，512Kbps以下建议8，512Kbps~1Mbps建议设置为16，1Mbps~2Mbps建议设置24，2Mbps~4Mbp建议设置28，4Mbps以上建议36
* @param unFecMaxGroupSize: FEC 分组的上限，根据终端CPU能力而定，最大不超过72，越大FEC所消耗的CPU越高，抗丢包能力也越强
* @param bEnableNack：是否启用NACK功能，收发双方均开启时生效。
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetTransParams(void* pTerminal, unsigned int unJitterBuffDelay, FEC_REDUN_TYPE eFecRedunMethod, 
                                          unsigned int unFecRedunRatio, unsigned int unFecMinGroupSize, unsigned int unFecMaxGroupSize, BOOL bEnableNack);


/***
* 设置本客户端接收哪几路音视频流【Online之前或者之后均可调用】
* 通过设置音视频下行掩码，可以选择从服务器接收哪几个位置的音视频数据。每一个bit对应一个位置，最低位对应0号位置，最高位对应31号位置。
* 比如希望接收某个index位置的音视频时，可以设置其对应bit设置为1，否则设置为0。
* @param pTerminal: 模块指针
* @param unAudioMask: 控制音频接收的掩码
* @param unVideoMask: 控制视频接收的掩码
* @return: <0为失败错误码，>=0为成功
*/
DLLIMPORT_SDT_SDK int  SDTerminal_SetAvDownMasks(void* pTerminal, unsigned int unAudioMask, unsigned int unVideoMask);



/***
* 设置状态变更反馈回调函数（比如异步登录成功、异步登录失败、启动重连、重连成功、账号被顶下去等）【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param pfStatusNotifyCallback: 状态变更反馈回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetSystemStatusNotifyCallback(void* pTerminal, SystemStatusNotifyFunc pfStatusNotifyCallback, void* pObject);


/***
* 设置接收视频回调函数【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param pfRecvRemoteVideoCallback: 接收视频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetRecvRemoteVideoCallback(void* pTerminal, RecvRemoteVideoFunc pfRecvRemoteVideoCallback, void* pObject);


/***
* 设置接收音频回调函数【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param pfRecvRemoteAudioCallback: 接收音频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetRecvRemoteAudioCallback(void* pTerminal, RecvRemoteAudioFunc pfRecvRemoteAudioCallback, void* pObject);









////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												SDTerminal SDK接口（高级API）
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/***
* 登录(异步方式接口)
* @param pTerminal: 模块指针
* @param strMediaServerIp: 服务器IP地址
* @param unDomainId: 服务器域号，默认1
* @param unUid: 唯一的用户ID
* @param unRoomId: 房间号
* @param eUserType: 用户类别
* @return: <0为失败错误码，>=0为接口调用成功，真正的登录状态将通过回调函数SystemStatusNotifyFunc通知外层。
*/
DLLIMPORT_SDT_SDK int  SDTerminal_OnlineAsync(
	void* pTerminal,
	char* strMediaServerIp,
	unsigned int unDomainId,
	unsigned int unUid, 
	unsigned int unRoomId, 
	USER_ONLINE_TYPE eUserType
	);



/***
* 获取音视频丢包率统计信息（内部已经乘100得到百分比）
* @param pTerminal: 模块指针
* @param pfVideoUpLostRatio: 视频上行丢包率
* @param pfVideoDownLostRatio: 视频下行丢包率
* @param pfAudioUpLostRatio: 音频上行丢包率
* @param pfAudioDownLostRatio: 音频下行丢包率
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_GetVideoAudioUpDownLostRatio(void* pTerminal, float *pfVideoUpLostRatio, float *pfVideoDownLostRatio, 
	float *pfAudioUpLostRatio, float *pfAudioDownLostRatio);

/***
* 获取音视频码率统计信息，单位Kbps
* @param pTerminal: 模块指针
* @param pfVideoUpRate: 视频上行码率
* @param pfVideoDownRate: 视频下行码率
* @param pfAudioUpRate: 音频上行码率
* @param pfAudioDownRate: 音频下行码率
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_GetVideoAudioUpDownBitrate(void* pTerminal, float *pfVideoUpRate, float *pfVideoDownRate, 
	float *pfAudioUpRate, float *pfAudioDownRate);


/***
* 获取当前时刻RTT
* @param pTerminal: 模块指针
* @return: RTT值
*/
DLLIMPORT_SDT_SDK unsigned int  SDTerminal_GetCurrentRtt(void* pTerminal);


/***
* 获取SDK版本信息
* @param pTerminal: 模块指针
* @return: 版本号
*/
DLLIMPORT_SDT_SDK unsigned int  SDTerminal_GetVersion(void* pTerminal);


/***
* 设置自动冗余度模式下的冗余度上下限，未调用时默认为0~100【Online之前调用生效】
* @param pTerminal: 模块指针
* @param unAutoRedunRatioMin: 自动冗余度下限。
* @param unAutoRedunRatioMax: 自动冗余度上限。
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetAutoRedunMinMax(void* pTerminal, unsigned int unAutoRedunRatioMin, unsigned int unAutoRedunRatioMax);


/***
* 设置视频帧率信息，作为发送时内部的Smoother处理参考
* 注意该帧率要符合实际帧率,可以高于实际帧率，但不能低于实际帧率，否则将导致发送速度不足。不调用本函数时，默认关闭smooth处理【Online之前或者之后均可调用】
* @param pTerminal: 模块指针
* @param unFrameRate: 视频参考帧率
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetVideoFrameRateForSmoother(void* pTerminal, unsigned int unFrameRate);


/***
* 指定发送音视频时使用内部自动生成时间戳还是使用外部提供的时间戳(Send接口传入)，默认为内部时间戳【Online之前或者之后均可调用】
* @param pTerminal: 模块指针
* @param bUseInternalTimestamp: TRUE标识采用内部时间戳，FALSE标识用户提供时间戳
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetUseInternalTimeStamp(void* pTerminal, BOOL bUseInternalTimestamp);




/***
* 指定本客户端的音视频编码类型，未调用本API时默认视频编码类型为H264，音频编码类型为AAC。【必须Online之前调用才能生效】
* 类型需要与客户端码流的实际情况相符
* @param pTerminal: 模块指针
* @param eVideoCodecType: 本客户端使用的视频编解码类型
* @param eAudioCodecType: 本客户端使用的音频编解码类型
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetVideoAudioCodecType(void* pTerminal, CLIENT_VIDEO_CODEC_TYPE eVideoCodecType, CLIENT_AUDIO_CODEC_TYPE eAudioCodecType);


/***
* 是否使能丢包冻结机制，在出现丢包时停止对外输出视频流，等待下一个完整关键帧到来时继续对外输出。默认开启。【Online之前或者之后均可调用】
* @param pTerminal: 模块指针
* @param bEnable: TRUE开启，FALSE关闭
* @return:
*/
DLLIMPORT_SDT_SDK void  SDTerminal_EnableFreezeFrameWhenLost(void* pTerminal, BOOL bEnable);




/***
* 设置码率自适应开关、模式以及反馈回调函数【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param eAutoBitrateMethod: 码率自适应方法
* @param pfAutoBitrateNotifyCallback: 码率自适应反馈回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetAutoBitrateNotifyCallback(void* pTerminal, AUTO_BITRATE_TYPE eAutoBitrateMethod, 
														AutoBitrateNotifyFunc pfAutoBitrateNotifyCallback, void* pObject);


/***
* 设置远端IDR请求反馈回调函数【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param pfRemoteIdrRequestNotifyCallback: DR请求反馈回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetRemoteIdrRequestNotifyCallback(void* pTerminal, RemoteIdrRequestNotifyFunc pfRemoteIdrRequestNotifyCallback, void* pObject);


/***
* 设置定时获取当前所在房间内音视频发布者状态回调函数【回调接口建议统一在Online之前调用】
* @param pTerminal: 模块指针
* @param unQueryIntervalSec为获取周期（秒），为0或不调用本API时表示不获取，最小获取间隔2秒。
* @param pfRoomInfoNotifyCallback: 房间内音视频发布者状态回调函数
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_SDT_SDK void  SDTerminal_SetRoomInfoNotifyCallback(void* pTerminal, unsigned int unQueryIntervalSec, 
														RoomInfoNotifyFunc pfRoomInfoNotifyCallback, void* pObject);
														



#ifdef __cplusplus
}
#endif

#endif // _SD_TERMINAL_SDK_H_
