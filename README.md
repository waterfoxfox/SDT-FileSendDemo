# DEMO说明
本DEMO用于演示纯传输层SDK的使用，DEMO读取指定的H264裸码流文件，解析成帧，调用API发送给对端。<br>

可通过DEMO配置文件设置服务器IP、房间号，FEC相关参数，H264裸码流文件路径等。


```js
[Config]
;服务器IP
ServerIp=47.106.195.225
;服务器域号
DomainId=3
;房间号
RoomId=888
;位置
UpPosition=1
;接收位置，设置为255，仅发送
DownPosition=255
;待发送h264文件
H264FileUrl=./output-360p.h264
;发送帧率
H264FileFps=25

;Fec method, 0-Auto   1-Fix
FecRedunMethod=0
FecRedunRatio=30
FecMinGroupSize=16
FecMaxGroupSize=32
FecEnableNack=1
```
<br>

SDK API的调用集中在SDClient.cpp中
<br>


ffmpeg制作任意分辨率H264测试码流命令：

```js
ffmpeg.exe -f lavfi -i testsrc=duration=100:size=1280x720:rate=25:decimals=2 -pix_fmt yuv420p -vcodec libx264  -profile:v high -x264opts force-cfr:fps=25:keyint=50:min-keyint=1:ref=1:bitrate=1600:bframes=0  -t 30  -y  output.h264
#本命令即生成1参考帧、无B帧、720P分辨率、1.6Mbps的H264测试流
```
<br>
测试工程使用VS2010或更高版本编译



---

# 相关资源
跟多文档、代码资源见：https://mediapro.apifox.cn

SDK 商用及定制化、技术支持服务可联系：[http://www.mediapro.cc/](http://www.mediapro.cc/)

