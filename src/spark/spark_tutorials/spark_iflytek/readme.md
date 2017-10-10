使用迅飞离线语音引擎演示与机器进行语音交互的补充说明：
请严格按照iflytek/readme文档中内容进行操作。
编译过程问题：
使用catkin_make出错，错误包含：
/usr/bin/ld:cannot find -lmsc
Failed
请确认系统版本，如果为32位使用32的libmsc.so并复制到/usr/lib，64位使用64的libmsc.so并复制到/usr/lib。
并重新编译。

使用过程问题：
Vad值设置和退出码为11210。
1.离线模式
请务必保持网络断开，并查看iflytek文件下，只有res文件夹和msc.cfg，
如果试过过程中联网，请删除其他所有无关文件并断网重试。
出现“对不起，我好像不明白”，解决：
1.1 参考readme，调整indlude/wb_vad_c.h中#define VAD_POW_LOW 6000000000.0f 的数值，具体数值需要使用
~$ rosrun iflytek vad 
power sum=48952312.000000
power sum=98880912.000000
power sum=97929630.000000
power sum=90929106.000000
power sum=81574084.000000
power sum=72921546.000000
power sum=75584684.000000
power sum=81104758.000000

依据，没有语音环境和说话环境的sum值确定。
1.2 如果上述过程依旧不行，请再次确认是否联网，并查看iflytek文件下，只有res文件夹和msc.cfg，如果试过过程中联网，请删除其他所有无关文件并断网重试。这时，如果启动xf_asr或xf_tts，出现11210错误代码，基本就是这个原因。

2.在线模式
退出码为11210。
如果没有迅飞appid，请不要使用此步骤。
使用在线模式需要更换自己的appid，官网库libmsc.so和对应资源文件msc/res。具体如下：

2.1 分别修改xf_asr.cpp和xf_tts.cpp
将其中(login_config, "appid =xxxxxxxx, work_dir =%s",buf_path);
Xxxxxxxx替换为自己的appid。
2.2 使用迅飞官网对于的Linux_*_1135_xxxxxxxx中的对应的libmsc.so，并注意32位或64位系统。
2.3 替换msc/res中的asr和tts为官网对应id的文件。

上面为具体步骤，如果配置不正确，出现错误如下：
~$ rosrun iflytek xf_asr 
构建离线识别语法网络...
构建语法失败！11210
~$ rosrun iflytek xf_tts 
开始合成 ...
正在合成 ...

QTTSAudioGet failed, error code: 11210.
[ERROR] [1480652116.361784659]: text_to_speech failed, error code: 11210.

合成完毕


==========================================================================================================
0x00 概述
==========================================================================================================
本package使用迅飞离线语音引擎演示与机器进行语音交互的大概工作方式。
只有5个node，分别为vad、xf_asr、xf_tts、parse、voice_turtle。

其中代码参考了ROS官网、迅飞语音库、及其它网上博文，不一一列出。

==========================================================================================================
0x01 功能包结构
==========================================================================================================
功能包目录结构如下：
.
├── call2.bnf
├── call.bnf
├── CMakeLists.txt
├── doc
├── include
│   ├── iflytek
│   ├── msp_cmn.h
│   ├── msp_errors.h
│   ├── msp_types.h
│   ├── qisr.h
│   ├── qtts.h
│   ├── typedef.h
│   ├── wb_vad_c.h
│   ├── wb_vad.h
│   ├── xf_asr.h
│   └── xf_tts.h
├── launch
│   ├── voice_turtle.launch
│   └── xf.launch
├── lib
│   └── libmsc.so
├── msc
│   ├── 183464a7ced28d444cad1489c1d778fe
│   │   ├── cfg.ldata
│   │   ├── kaisound.dat
│   │   ├── kaitalk.dat
│   │   ├── u.data
│   │   ├── urec.data
│   │   ├── usr.cdata
│   │   └── usr.ldata
│   ├── msc.cfg
│   └── res
│       ├── asr
│       │   ├── common.jet
│       │   └── GrmBuilld
│       │       └── temp
│       └── tts
│           ├── common.jet
│           ├── xiaofeng.jet
│           └── xiaoyan.jet
├── package.xml
├── readme
└── src
    ├── parse.py
    ├── vad.cpp
    ├── voice_turtle.py
    ├── wb_vad.cpp
    ├── xf_asr.cpp
    └── xf_tts.cpp
include目录存放.h的头文件;src文件夹存放.cpp及.py源文件;launch文件夹存放.launch文件;doc文件夹存放迅飞的说明文档，供参考;lib下的.so为迅飞离线语音库;msc为存放迅飞语音引擎资源文件，也是其工作目录;call.bnf为语法定义文件。

==========================================================================================================
0x02 使用说明
==========================================================================================================
后面的一切操作均默认你的环境是：ubuntu14.04+indigo，且ROS环境正常。

安装 libasound2-dev及mplayer
$sudo apt-get install libasound2-dev mplayer

安装 turtlesim功能包(catkin_ws为工作空间，请根据实际自行替换)
$cd ~/catkin_ws/src
$git clone https://github.com/ros/ros_tutorials.git
$cd ros_tutorials
$git checkout indigo-devel

迅飞离线库处理
sudo cp `rospack find iflytek`/lib/libmsc.so /usr/lib
迅飞离线库与代码中的appid一一对应。本功能包提供的离线库只能保证暂时可用。过期后将不可用。请自行申请，并修改代码（xf_asr.cpp，xf_tts.cpp）中的appid。

检测音频输入输出设备
由于功能包中多次调用音频输入，为了减少硬件调用错误，代码中固化音频输入输出设备均为"default"，因为在alsa下，默认设备可以复用。
使用alsa工具进行按下列顺序进行检测或设置
$ aplay -l
**** PLAYBACK 硬體裝置清單 ****
card 0: Intel [HDA Intel], device 0: AD1981 Analog [AD1981 Analog]
  子设备: 1/1
  子设备 #0: subdevice #0
card 0: Intel [HDA Intel], device 1: AD1981 Digital [AD1981 Digital]
  子设备: 1/1
  子设备 #0: subdevice #0
card 1: G4ME1 [Sennheiser 3D G4ME1], device 0: USB Audio [USB Audio]
  子设备: 1/1
  子设备 #0: subdevice #0

一般hw 0:0会被alsa选用默认设备。如果card 0坏了，想要选用card 1--[Sennheiser 3D G4ME1]，则需要修改alsa的配置文件。其中一种方法为：修改/etc/asound.conf文件，如果没有，则新建。
$sudo vi /etc/asound.conf

添加如下内容
defaults.ctl.card 1
defaults.pcm.card 1
defaults.timer.card 1
defaults.pcm.device 0
:wq保存退出
以上内容的值根据aplay -l的结果设置。

编译
$cd ~/catkin_ws
$catkin_make


使用turtlesim测试
$roslaunch iflytek xf.launch
ctrl+shift+t 再另开一个终端
$roslaunch iflytek voice_command.launch

语音指令：
commands = {'stop':[u'停止','stop'],
			'goal':[u'去端茶','get me the tea'],
			'home':[u'回家','home'],
			'left':[u'左',u'向左','left'],
			'right':[u'右',u'向右','right'],
			'forward':[u'前进','forward','ahead'],
			'bakward':[u'后退','back','backward'],			
			'fast':[u'快',u'快点','speed up', 'faster'],
			'slow':[u'慢',u'慢点','slow down', 'slower'],
			'blank':[],
			'round':[u'转圈',u'向左转圈'],
			'rround':[u'向右转圈'],
			'begin':[u'开始语音控制'],
			'end':[u'结束语音控制'],}

commands定义了命令控制指令，如果识别到每行[]内的内容，会向topic /command发送类型为std_msgs/String类型的message,内容为每行开头字串
chats =    {'请叫我spark':[u'你叫什么',u'你叫什么名字',"what's your name"],
			'我不告诉你':[u'你几岁了',u'你多大了'],
			'这个世界太不安静了':[u'陪我聊聊天',u'聊聊天',u'聊天'],
			'你也很好':[u'你好']}

chats定义了其它识别结果，如果识别如果识别到每行[]内的内容，会向topic ／xfsayword发送类型为std_msgs/String类型的message，内容为每行开头字串。xf_tts节点会接收字串并进行语音合成输出。如果其它节点需语音输出，只需向/xfsayword发送message即可。
以上commands和chats解析在parse.py中完成。
具体语音指令参看代码，也可自行调整。

如果需要增加识别词并进行解析，要先在call.bnf添加词汇，然后在parse.py中再作相应更改。

如果与其它移动平台语音交互，只需xf.launch。此launch启动了var、xf_asr、xf_tts和parse四个节点。

voice_command.launch启动了turtlesim中的turtle_node、和voice_command节点。voice_command.py也可用于其它移动平台，由于文件内定义的输出topic 为/cmd_vel，需要根据实际情况进行重定向，具体实例看voice_command.launch。

==========================================================================================================
0x03 输入输出
==========================================================================================================
vad
pub:
/xfwakeup       std_msgs::String       检测到语音输入触发

sub:
/vad            std_msgs::String       接收到message再次进入语音检测
-----------------------------------------------------------------

xf_asr
pub:
/xfsaywords      std_msgs::String      语音识别失败发送默认失败文本进行语音合成输出
/vad             std_msgs::String      请求进入语音检测
/xfunderstand    std_msgs::String      识别结果

sub:
/xfwakeup        std_msgs::String      接收到message进入语音识别
-----------------------------------------------------------------

xf_tts
sub:
/xfsaywords      std_msgs::String      接收语音合成文本
----------------------------------------------------------------

parse
pub:
/xfsaywords      std_msgs.msg.String   发送应答文本进行语音合成输出
/command         std_msgs.msg.String    动作指令序号，用作动作控制

sub:
/xfunderstand    std_msgs::String      识别结果
----------------------------------------------------------------
voice_turtle
pub:
/cmd_vel         std_msgs.msg.Twist    控制指令

sub:
/command         std_msgs.msg.String    动作指令序号

==========================================================================================================
0x04 其它说明
==========================================================================================================
vad节点语音端点是失败的。所以只有修改阀值在固定环境中使用。修改方法为：单运行vad节点，查看power值的输出，再修改wb_vad_c.h中的VAD_POW_LOW。
为此又准备了一个语音端点检测的代码(在功能包外的vad文件夹中)，但是只检测语音后端点，所以思路要改下，换成课件中的第一种方式。一直循环，固定间隔内如果检测到后端点，则认为语音有效，送交API进行识别，否则丢弃。这样一来，第一个节点就没用了，减少一个工作节点，代码需要微调下。代码使用的是PulseAudio接口。

==========================================================================================================
0x05 联系我
==========================================================================================================
QQ:OTkwMDM0Ng==


总之，一切看代码。








