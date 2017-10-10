#ifndef XF_ASR_H
#define XF_ASR_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "pthread.h"
#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#define SAMPLE_RATE 16000
#define CHANNLE 1
#define FRAMES_SIZE 1024
#define FORMAT SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE 2

#define SAMPLE_RATE_16K     (16000)
#define SAMPLE_RATE_8K      (8000)
#define MAX_GRAMMARID_LEN   (32)
#define MAX_PARAMS_LEN      (1024)

#define DEVICE  "default"


char data[10000];
int flag_begin=0;
int flag_understand=0;
int flag_unknow=0;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
   char            riff[4];                // = "RIFF"
   int             size_8;                 // = FileSize - 8
   char            wave[4];                // = "WAVE"
   char            fmt[4];                 // = "fmt "
   int             fmt_size;               // = 下一个结构体的大小 : 16

   short int       format_tag;             // = PCM : 1
   short int       channels;               // = 通道数 : 1
   int             samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
   int             avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
   short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
   short int       bits_per_sample;        // = 量化比特数: 8 | 16

   char            data[4];                // = "data";
   int             data_size;              // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;
/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr =
{
   { 'R', 'I', 'F', 'F' },
   0,
   {'W', 'A', 'V', 'E'},
   {'f', 'm', 't', ' '},
   16,
   1,
   1,
   16000,
   32000,
   2,
   16,
   {'d', 'a', 't', 'a'},
   0
};


typedef struct _UserData {
    int     build_fini; //标识语法构建是否完成
    int     update_fini; //标识更新词典是否完成
    int     errcode; //记录语法构建或更新词典回调错误码
    char    grammar_id[MAX_GRAMMARID_LEN]; //保存语法构建返回的语法ID
}UserData;
UserData asr_data;

int build_grammar(UserData *udata); //构建离线识别语法网络
int run_asr(void *udata); //进行离线语法识别
void wakeupcallback(const std_msgs::String::ConstPtr& msg);

#endif // XF_ASR_H
