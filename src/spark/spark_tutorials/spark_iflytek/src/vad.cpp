#include "stdio.h"   
#include "wb_vad.h"
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#include <stdlib.h>
#include "ros/ros.h"
#include "std_msgs/String.h"
bool flag_vad = true;


void vadcallback(const std_msgs::String::ConstPtr& msg)
{
    std::cout<<"vad checking"<<std::endl;
   // printf("%s", *msg->data.c_str());    
     flag_vad = true;
}


int main(int argc,char* argv[])   
{      
    const char* pcm_device_name = "default";
//    char* output_file_name = ;
    int   record_seconds = 2;
    // for alsa
    long loops;
    int rc;
    int size;
//    int size_one_channel;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int dir;
    char *buffer;
//    char *buffer_one_channel;
    int i;

    snd_pcm_stream_t  stream = SND_PCM_STREAM_CAPTURE;
    snd_pcm_access_t  mode = SND_PCM_ACCESS_RW_INTERLEAVED;//SND_PCM_ACCESS_RW_NONINTERLEAVED;
    snd_pcm_format_t  format = SND_PCM_FORMAT_S16_LE;
    unsigned int      channels = 1;
    unsigned int      rate     = 16000;
//    snd_pcm_uframes_t frames   = FRAME_LEN;
    snd_pcm_uframes_t frames   = FRAME_LEN;
    
    //for vda
    float indata[FRAME_LEN];
    VadVars *vadstate; 
    int temp,vad;
    bool flag_close = true;
    ros::init(argc, argv, "vad");
    ros::NodeHandle n;
    ros::Rate loop_rate(10);
    ros::Subscriber sub = n.subscribe("vad", 1000, vadcallback);
    ros::Publisher pub = n.advertise<std_msgs::String>("xfwakeup", 1000);
    sleep(1);
    while(ros::ok())
    {
    // alsa init
      if (flag_vad)
      {
          rc = snd_pcm_open(&handle, pcm_device_name,stream, 0);
          flag_close = true;
          if (rc < 0) {
              fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
              exit(1);
          }
          snd_pcm_hw_params_alloca(&params);
          snd_pcm_hw_params_any(handle, params);
          snd_pcm_hw_params_set_access(handle, params,mode);
          snd_pcm_hw_params_set_format(handle, params,format);
          snd_pcm_hw_params_set_channels(handle, params, channels);
          snd_pcm_hw_params_set_rate_near(handle, params,&rate, &dir);
          snd_pcm_hw_params_set_period_size_near(handle,params, &frames, &dir);
          rc = snd_pcm_hw_params(handle, params);
          if (rc < 0) {
              fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
              exit(1);
          }
          snd_pcm_hw_params_get_period_size(params,&frames, &dir);
          size = frames * 2*channels; /* 2 bytes/sample, 1 channels */          
          buffer = (char*) malloc(size);
          
          
          //vad init
          wb_vad_init(&(vadstate));
//          sprintf(name,"%s.pcm",output_file_name);
//          fp_all = fopen(name,"wb");
          
          /* We want to loop for 2 seconds */
          snd_pcm_hw_params_get_period_time(params,&rate, &dir);
          loops = record_seconds *2*1000000 / rate;
          
          while ( loops) {
                  loops--;
                  rc = snd_pcm_readi(handle, buffer, frames);
                  if (rc == -EPIPE) {
                    /* EPIPE means overrun */
                    fprintf(stderr, "overrun occurred\n");
                    snd_pcm_prepare(handle);
                  } else if (rc < 0) {
                    fprintf(stderr, "error from read: %s\n",snd_strerror(rc));
                  } else if (rc != (int)frames) {
                    fprintf(stderr, "short read, read %d frames\n", rc);
                  }else if (rc == -EBADFD){
                    fprintf(stderr, "PCM is not in the right state\n");
                  }else if (rc == -ESTRPIPE){
                    fprintf(stderr, "a suspend event occurred\n");
                  }
          
                  for(i = 0; i< frames; i++)
                  {
                      indata[i]=0;
                      temp = 0;
                      memcpy(&temp,buffer+2*i,2);
                      
                      indata[i]=temp;                     
//                      outdata[i]=temp;
                      if(indata[i]>65535/2)
                          indata[i]=indata[i]-65536;
                  }
                  vad=wb_vad(vadstate,indata);
                  
                  if(vad == 1)
                  {
                      snd_pcm_close(handle);
					  printf("VAD checked!!!");
                      std_msgs::String msg;
                      std::stringstream ss;                      
                      ss << "s ";
                      msg.data = ss.str();
                      pub.publish(msg);
                      flag_vad = false;
                      flag_close = false;
                      vad = 0;                      
                      sleep(2);
                      break;
                  }
           
          }
          if (flag_close )
              snd_pcm_close(handle);
           if (buffer != NULL) free(buffer); 
           wb_vad_exit(&vadstate);
        }
      ros::spinOnce();
      loop_rate.sleep();
     }
}
