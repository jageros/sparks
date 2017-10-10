#include "xf_tts.h"
#define SAYIT  system("cp  /tmp/voice/say.wav /tmp/voice/temp.wav>/tmp/cmd/Mplayer_cmd");system("echo loadfile /tmp/voice/temp.wav>/tmp/cmd/Mplayer_cmd")


const char * XIAOYAN_PATH        = "/msc/res/tts/xiaoyan.jet"; 
const char * COMMON_PATH      = "/msc/res/tts/common.jet"; 

const char * WORK_PATH;


/* *******************************************************
 * 文本合成 
*********************************************************
*/
int text_to_speech(const char* src_text, const char* des_path, const char* params)
{
    int          ret          = -1;
    FILE*        fp           = NULL;
    const char*  sessionID    = NULL;
    unsigned int audio_len    = 0;
    wave_pcm_hdr wav_hdr      = default_wav_hdr;
    int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

    if (NULL == src_text || NULL == des_path)
    {
        printf("params is error!\n");
        return ret;
    }
    fp = fopen(des_path, "wb");
    if (NULL == fp)
    {
        printf("open %s error.\n", des_path);
        return ret;
    }
    /* 开始合成 */
    sessionID = QTTSSessionBegin(params, &ret);
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSSessionBegin failed, error code: %d.\n", ret);
        fclose(fp);
        return ret;
    }
    ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSTextPut failed, error code: %d.\n",ret);
        QTTSSessionEnd(sessionID, "TextPutError");
        fclose(fp);
        return ret;
    }
    printf("正在合成 ...\n");
    fwrite(&wav_hdr, sizeof(wav_hdr) ,1, fp); //添加wav音频头，使用采样率为16000
    while (1) 
    {
        /* 获取合成音频 */
        const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
        if (MSP_SUCCESS != ret)
            break;
        if (NULL != data)
        {
            fwrite(data, audio_len, 1, fp);
            wav_hdr.data_size += audio_len; //计算data_size大小
        }
        if (MSP_TTS_FLAG_DATA_END == synth_status)
            break;
    }//合成状态synth_status取值请参阅《讯飞语音云API文档》
    printf("\n");
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSAudioGet failed, error code: %d.\n",ret);
        QTTSSessionEnd(sessionID, "AudioGetError");
        fclose(fp);
        return ret;
    }
    /* 修正wav文件头数据的大小 */
    wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);
    /* 将修正过的数据写回文件头部,音频文件为wav格式 */
    fseek(fp, 4, 0);
    fwrite(&wav_hdr.size_8,sizeof(wav_hdr.size_8), 1, fp); //写入size_8的值
    fseek(fp, 40, 0); //将文件指针偏移到存储data_size值的位置
    fwrite(&wav_hdr.data_size,sizeof(wav_hdr.data_size), 1, fp); //写入data_size的值
    fclose(fp);
    fp = NULL;
    /* 合成完毕 */
    ret = QTTSSessionEnd(sessionID, "Normal");
    if (MSP_SUCCESS != ret)
    {
        printf("QTTSSessionEnd failed, error code: %d.\n",ret);
    }
    return ret;
}

/*****************************************************************************
 * 
 *****************************************************************************
 */
int xf_tts(const char* text,const char *filename)
{
    int  ret   = MSP_SUCCESS;
    char  login_params[64]         = {};//登录参数,appid与msc库绑定,请勿随意改动   
    sprintf(login_params, "appid = 58218918, work_dir =%s",WORK_PATH);
    char session_begin_params[1024] = {};
    snprintf(session_begin_params, 1023,
        "engine_type =local, \
        text_encoding = UTF8, \
        tts_res_path = fo|%s%s;fo|%s%s, \
        sample_rate = 16000, speed = 50, \
        volume = 50, pitch = 50, rdn = 2",
        WORK_PATH,XIAOYAN_PATH,       
        WORK_PATH, COMMON_PATH    
        );
    /* 用户登录 */
    ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://open.voicecloud.cn注册获取
    if (MSP_SUCCESS != ret)
    {
        printf("MSPLogin failed, error code: %d.\n", ret);
        goto exit ;//登录失败，退出登录
    }
    /* 文本合成 */
    printf("开始合成 ...\n");
    ret = text_to_speech(text, filename, session_begin_params);
    if (MSP_SUCCESS != ret)
    {
        ROS_ERROR("text_to_speech failed, error code: %d.\n", ret);
    }
    printf("合成完毕\n");

exit:
    MSPLogout(); //退出登录
    return 0;
}
void xfcallback(const std_msgs::String::ConstPtr& msg)
{
  char cmd[2000];
  //std::cout<<"I heard,I will say:"<<msg->data.c_str()<<std::endl;
  xf_tts(msg->data.c_str(),"/tmp/voice/say.wav");
  sprintf(cmd,"echo %s>/tmp/cmd/saywords",msg->data.c_str());
  //popen(cmd,"r");
  SAYIT;
}

/*************************************************************
 *
 *************************************************************
 */
void getworkspace(char * buf,int size)
{
  FILE   *stream;   
  memset( buf, '\0', size );//初始化buf,以免后面写如乱码到文件中
  stream = popen( "rospack find spark_iflytek", "r" ); //将命令的输出 通过管道读取（“r”参数）到FILE* stream  
  fread( buf, sizeof(char), size, stream); //将刚刚FILE* stream的数据流读取到buf中  
  pclose( stream );    
}

/**************************************************************
 * 
 **************************************************************
 */
int main(int argc,char **argv)
{
    if (NULL == opendir("/tmp/voice"))
        mkdir("/tmp/voice",0777);
    if (NULL == opendir("/tmp/cmd"))
     mkdir("/tmp/cmd",0777);
    char  buf_path[255] ;
    getworkspace(buf_path,sizeof(buf_path));
    int l = strlen(buf_path);
    buf_path[l-1] = 0x0;     
    WORK_PATH  = buf_path;   
    
    unlink("/tmp/cmd/Mplayer_cmd");
    mkfifo("/tmp/cmd/Mplayer_cmd", 0777);
    FILE * fp = popen("mplayer -quiet -slave -input file=/tmp/cmd/Mplayer_cmd -idle","r");    
    const char* filename        = "/tmp/voice/say.wav"; //合成的语音文件名称
    const char* text                 = "语音合成模块启动成功！"; //合成文本
    xf_tts(text,filename);
    SAYIT;
    ros::init(argc,argv,"xf_tts");
    ros::NodeHandle n;
    ros::Subscriber sub =n.subscribe("xfsaywords",1000,xfcallback);
    ros::spin();
    pclose(fp);    
    return 0;
}
