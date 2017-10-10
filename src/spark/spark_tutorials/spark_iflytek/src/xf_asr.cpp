#include "xf_asr.h"


const char * ASR_RES_PATH        = "/msc/res/asr/common.jet"; //离线语法识别资源路径
const char * GRM_BUILD_PATH      = "/msc/res/asr/GrmBuilld"; //构建离线语法识别网络生成数据保存路径
const char * GRM_FILE            = "/call.bnf"; //构建离线识别语法网络所用的语法文件
const char * WORK_PATH = "";


/***********************************************
 * 
 ***********************************************
 */
int build_grm_cb(int ecode, const char *info, void *udata)
{
      UserData *grm_data = (UserData *)udata;
      if (NULL != grm_data) {
          grm_data->build_fini = 1;
          grm_data->errcode = ecode;
      }

      if (MSP_SUCCESS == ecode && NULL != info) {
          printf("构建语法成功！ 语法ID:%s\n", info);
          if (NULL != grm_data)
              snprintf(grm_data->grammar_id, MAX_GRAMMARID_LEN - 1,"%s", info);
      }
      else
         printf("构建语法失败！%d\n", ecode);

      return 0;
}

/***********************************************
 * 
 ***********************************************
 */
int build_grammar(UserData *udata)
{
      FILE *grm_file                           = NULL;
      char *grm_content                        = NULL;
      unsigned int grm_cnt_len                 = 0;
      char grm_build_params[MAX_PARAMS_LEN]    = {};
      int ret                                  = 0;
      char file[255];
      sprintf(file,"%s%s",WORK_PATH,GRM_FILE);      
      grm_file = fopen(file, "rb");
      if(NULL == grm_file) {
          printf("打开\"%s\"文件失败！[%s]\n", file, strerror(errno));
          return -1;
      }
      fseek(grm_file, 0, SEEK_END);
      grm_cnt_len = ftell(grm_file);
      fseek(grm_file, 0, SEEK_SET);

      grm_content = (char *)malloc(grm_cnt_len + 1);
      if (NULL == grm_content)
      {
          printf("内存分配失败!\n");
          fclose(grm_file);
          grm_file = NULL;
          return -1;
      }
      fread((void*)grm_content, 1, grm_cnt_len, grm_file);
      grm_content[grm_cnt_len] = '\0';
      fclose(grm_file);
      grm_file = NULL;

      snprintf(grm_build_params, MAX_PARAMS_LEN - 1,
          "engine_type = local, \
          asr_res_path = fo|%s%s, sample_rate = %d, \
          grm_build_path = %s%s, ",
          WORK_PATH,ASR_RES_PATH,
          SAMPLE_RATE_16K,
          WORK_PATH,GRM_BUILD_PATH
          );
      
      ret = QISRBuildGrammar("bnf", grm_content, grm_cnt_len, grm_build_params, build_grm_cb, udata);

      free(grm_content);
      grm_content = NULL;

      return ret;
}
/***********************************************
 * 
 ***********************************************
 */
int recode_asr()
{
      char asr_params[MAX_PARAMS_LEN]    = {};
      const char *rec_rslt               = NULL;
      const char *session_id             = NULL;

      FILE *f_pcm                        = NULL;
      char *pcm_data                     = NULL;
      long pcm_count                     = 0;

      int aud_stat                       = MSP_AUDIO_SAMPLE_CONTINUE;
      int ep_status                      = MSP_EP_LOOKING_FOR_SPEECH;
      int rec_status                     = MSP_REC_STATUS_INCOMPLETE;
      int rss_status                     = MSP_REC_STATUS_INCOMPLETE;
      int errcode                        = -1;

      long loops;
      int rc, size;
      float time = 5;
      snd_pcm_t *handle;
      snd_pcm_hw_params_t *params;
      snd_pcm_uframes_t frames, ret;
      char *buffer;
      char *ptr_buffer;
      FILE *fp = fopen("/tmp/voice/listened.wav", "wb");

      //离线语法识别参数设置
      snprintf(asr_params, MAX_PARAMS_LEN - 1,
          "engine_type = local,\
          vad_eos=1000,\
          nlp_version=2.0,vad_eos=1,\
          asr_res_path = fo|%s%s, sample_rate = %d, \
          grm_build_path = %s%s, local_grammar = %s, \
          result_type = json, result_encoding = UTF-8 ",
          WORK_PATH,ASR_RES_PATH,
          SAMPLE_RATE_16K,
         WORK_PATH, GRM_BUILD_PATH,
          asr_data.grammar_id
          );
      session_id = QISRSessionBegin(NULL, asr_params, &errcode);
      printf("开始识别...");

      rc = snd_pcm_open(&handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0);
      /*if (rc < 0) {
        
      } else{        
        ROS_DEBUG("OK:snd_pcm_open()\n");
      }*/
      snd_pcm_hw_params_alloca(&params);
      //ROS_DEBUG("OK:after alloca\n");
      
      rc = snd_pcm_hw_params_any(handle, params);      
      /*if (rc < 0) {
        
      } else{ 
        ROS_DEBUG("OK:snd_pcm_hw_params_any()\n");  
      }*/
      
      rc = snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
      /*if (rc < 0) {
        
      }  else{
        ROS_DEBUG("OK:snd_pcm_hw_params_set_access()\n"); 
      }*/
      
      rc = snd_pcm_hw_params_set_format(handle, params, FORMAT);
      /*if (rc < 0) {
        
      }  else{ 
        ROS_DEBUG("OK:snd_pcm_hw_params_set_format()\n");  
      }*/
      
      rc = snd_pcm_hw_params_set_channels(handle, params, CHANNLE);
      /*if (rc < 0) {
        
      }  else{ 
        ROS_DEBUG("OK:snd_pcm_hw_params_set_channels()\n"); 
      }*/
      
      rc = snd_pcm_hw_params_set_rate(handle, params,SAMPLE_RATE, 0);
      /*if (rc < 0) {
        
      }  else{ 
        ROS_DEBUG("OK:snd_pcm_hw_params_set_rate()\n");  
      }*/
      

      rc = snd_pcm_hw_params(handle, params);      
      /*if (rc < 0) {
        
      }  else{
        ROS_DEBUG("OK:snd_pcm_hw_params()\n"); 
      }*/
	  //frames = FRAMES_SIZE;
	  snd_pcm_hw_params_get_period_size(params,&frames, 0);

	  unsigned int val;
      snd_pcm_hw_params_get_period_time(params,&val, 0);

      size = frames * PER_SAMPLE *CHANNLE; /* 2 bytes/sample, 1 channels */
      ptr_buffer = buffer = (char *) malloc(size);
      /*if(buffer == NULL){
        
      }else{
        ROS_DEBUG("OK:malloc()\n");  
      }*/
      loops = time*1000000/val;

      while (loops > 0)
      {
            loops--;
            ret = snd_pcm_readi(handle, ptr_buffer, frames);
            if (ret == -EPIPE) {
              ROS_ERROR( "overrun occurred\n");
              snd_pcm_prepare(handle);
            } else if (ret < 0)
              ROS_ERROR("error from read: %s\n",snd_strerror(ret));
            else if (ret != frames)
              ROS_ERROR( "short read, read %d frames\n", (int)ret);            

          printf(">");
          fflush(stdout);
          if (0 == pcm_count) 
                  aud_stat = MSP_AUDIO_SAMPLE_FIRST;
          else   
                  aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
          errcode = QISRAudioWrite(session_id, (const void *)ptr_buffer, size, aud_stat, &ep_status, &rec_status);
          if (MSP_EP_AFTER_SPEECH == ep_status)
                  break;     //检测到音频结束
      }

      QISRAudioWrite(session_id, (const void *)NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_status, &rec_status);

      if(buffer != NULL)  free(buffer);
      if(fp != NULL)  fclose(fp);
      snd_pcm_drain(handle);
      snd_pcm_close(handle);

      free(pcm_data);
      pcm_data = NULL;

      //获取识别结果
      while (MSP_REC_STATUS_COMPLETE != rss_status && MSP_SUCCESS == errcode) {
          rec_rslt = QISRGetResult(session_id, &rss_status, 0, &errcode);
          printf(".");
          usleep(150 * 1000);
      }
      printf("\n识别结束%d：\n",(int)loops);
      printf("=============================================================\n");
      if (NULL != rec_rslt)
      {
          printf("%s\n", rec_rslt);
          sprintf(data,"%s\n", rec_rslt);
          flag_understand=1;
      }
      else
      {
          printf("没有识别结果！\n");
          flag_unknow=1;
      }
      printf("=============================================================\n");

      goto run_exit;

      if (NULL != pcm_data) {
          free(pcm_data);
          pcm_data = NULL;
      }
      if (NULL != f_pcm) {
          fclose(f_pcm);
          f_pcm = NULL;
      }
  run_exit:
      ROS_INFO("exit with code :%d..\n",errcode);
      QISRSessionEnd(session_id, NULL);
      return errcode;
}
/***********************************************
 * 
 ***********************************************
 */
void wakeupcallback(const std_msgs::String::ConstPtr& msg)
{
    std::cout<<"waking up"<<std::endl;    
    flag_begin=1;
}
void getworkspace(char * buf,int size)
{
  FILE   *stream;   
  memset( buf, '\0', size );//初始化buf,以免后面写如乱码到文件中
  stream = popen( "rospack find spark_iflytek", "r" ); //将命令的输出 通过管道读取（“r”参数）到FILE* stream  
  fread( buf, sizeof(char), size, stream); //将刚刚FILE* stream的数据流读取到buf中  
  pclose( stream );    
}

int main(int argc,char **argv)
{     
      int ret                                   = 0 ;
      char  buf_path[255] ;
      getworkspace(buf_path,sizeof(buf_path));
      int l = strlen(buf_path);
      buf_path[l-1] = 0x0;     
      WORK_PATH  = buf_path;      
      
//      const char *login_config    = "appid =58218918, work_dir ="; //登录参数
      char  login_config[128] = {};
      sprintf(login_config, "appid =58218918, work_dir =%s",buf_path);
      ret = MSPLogin(NULL, NULL, login_config); //第一个参数为用户名，第二个参数为密码，传NULL即可，第三个参数是登录参数
      if (MSP_SUCCESS != ret) {
            printf("登录失败：%d\n", ret);
            return  -1;
      }
      printf("构建离线识别语法网络...\n");
      ret = build_grammar(&asr_data);  //第一次使用某语法进行识别，需要先构建语法网络，获取语法ID，之后使用此语法进行识别，无需再次构建
      if (MSP_SUCCESS != ret) {
            printf("构建语法调用失败！\n");
            return -1;
      }
      while (1 != asr_data.build_fini)    usleep(300 * 1000);
      if (MSP_SUCCESS != asr_data.errcode)
            return -1;

      if (MSP_SUCCESS != ret) {
            printf("离线语法识别出错: %d \n", ret);
           return -1;
      }
      ros::init(argc, argv, "xf_asr");
      ros::NodeHandle n;
      ros::Rate loop_rate(100);
      ros::Subscriber sub = n.subscribe("xfwakeup", 1000, wakeupcallback);
      ros::Publisher pub = n.advertise<std_msgs::String>("xfsaywords", 1000);
      ros::Publisher pub1 = n.advertise<std_msgs::String>("vad", 1000);
      ros::Publisher pub2= n.advertise<std_msgs::String>("xfunderstand",10000);
      //ros::param::param<std::string>("default_param", asr_res, "default_value");
      while (ros::ok())
      {
          if(flag_begin)
          {
				flag_begin=0;                
				ret = recode_asr();
				usleep(500*1000);                
                std_msgs::String msg;
                std::stringstream ss;                
                ss << "s ";
                msg.data = ss.str();
                pub1.publish(msg);
          }
          if(flag_unknow)
          {
                std_msgs::String msg;
                std::stringstream ss;
                flag_unknow=0;
                ss << "对不起，我好像不明白！ ";
                msg.data = ss.str();
                pub.publish(msg);

          }
          if (flag_understand)
          {
                flag_understand=0;
                std_msgs::String msg;
                std::stringstream ss2;
                ss2 << data;
                msg.data = ss2.str();
                pub2.publish(msg);

          }
       ros::spinOnce();
       loop_rate.sleep();
      }

      MSPLogout();
}
