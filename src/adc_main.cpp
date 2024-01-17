#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "caio.h"

namespace contec
{
class AnalogInput
{
public:
  AnalogInput(double rate) : nh_("~"), rate_(rate)
  {
    init();
  }

  void startPublish()
  {
    long Ret;
    float AiData[256];
    ros::Rate loop_rate(rate_);
    setContinuousRead();
    while (ros::ok())
    {
      long AiStatus;
      Ret = AioGetAiStatus(id_, &AiStatus);
      // ROS_INFO("%lx", AiStatus);
      bool is_busy = (AiStatus & AIS_BUSY) == AIS_BUSY;
      bool is_ok = (AiStatus & AIS_DATA_NUM) == AIS_DATA_NUM;

      if(!is_ok) continue;

      // ADコンバータからデータを読み取る
      long AiSamplingTimes = ai_channels;
      Ret = AioGetAiSamplingDataEx(id_, &AiSamplingTimes, AiData);
      if(!isValidStatus(Ret))
      {
        ROS_ERROR("AioGetAiSamplingDataEx Failed");
        break;
      }

      // データをROSトピックとしてパブリッシュ
      for (int i = 0; i < ai_channels; i++) {
        std_msgs::Float64 msg;
        msg.data = AiData[i];
        pub_[i].publish(msg);
      }

      ros::spinOnce();
      loop_rate.sleep();
    }

    stopContinuousRead();
    // デバイスの終了処理
    AioExit(id_);
  }

private:
  void init()
  {
    // Device Setup
    initializeDevice();

    // チャンネル数の設定
    nh_.param("ai_channels", ai_channels, 1); 
    bool valid = checkRangeOfMaxChannel(static_cast<short>(ai_channels));
    if(!valid) 
    {
      long Ret = AioExit(id_);
      return;
    }

    // Publisher Setup
    for (int i = 0; i < ai_channels; i++)
    {
      std::string topic_name = "channel_" + std::to_string(i);
      pub_[i] = nh_.advertise<std_msgs::Float64>(topic_name.c_str(), 20);
    }
  }

  void initializeDevice()
  {
    // デバイスの初期化
    long Ret;
    std::string device_name;
    nh_.param<std::string>("device_name", device_name, "AIO000");

    char* temp_device_name = new char[device_name.length() + 1];
    strcpy(temp_device_name, device_name.c_str());
    Ret = AioInit(temp_device_name, &id_);
    delete[] temp_device_name;
    if(!isValidStatus(Ret))
    {
      ROS_ERROR("AioInit Failed");
      return;
    }

    // 入力レンジの設定(全チャンネル)
    int ai_range;
    nh_.param("ai_range", ai_range, 0);
    Ret = AioSetAiRangeAll(id_, static_cast<short>(ai_range));
    if(!isValidStatus(Ret))
    {
      ROS_ERROR("AioSetAiRangeAll Failed");
    }
  }

  void setContinuousRead()
  {
    long Ret;

    // メモリ形式の設定：FIFO
	  Ret = AioSetAiMemoryType(id_, 0);
    if(!isValidStatus(Ret)) ROS_ERROR("AioSetAiMemoryType Failed");

    // クロック種類の設定：内部
	  Ret = AioSetAiClockType(id_, 0);
    if(!isValidStatus(Ret)) ROS_ERROR("AioSetAiClockType Failed");

    // 変換速度の設定 [usec]
    double sampling_period_usec = 1e6 / rate_ / ai_channels;
	  Ret = AioSetAiSamplingClock(id_, sampling_period_usec);
    if(!isValidStatus(Ret)) ROS_ERROR("AioSetAiSamplingClock Failed");

    // 変換開始条件の設定 : ソフトウェア
    Ret = AioSetAiStartTrigger(id_, 0);
    if(!isValidStatus(Ret)) ROS_ERROR("AioSetAiStartTrigger Failed");

    // 変換停止条件の設定 : コマンド（AioStopAi ）
    Ret = AioSetAiStopTrigger(id_, 4);
    if(!isValidStatus(Ret)) ROS_ERROR("AioSetAiStopTrigger Failed");

    // メモリのリセット
	  Ret = AioResetAiMemory(id_);
    if(!isValidStatus(Ret)) ROS_ERROR("AioResetAiMemory Failed");

    // 変換開始
    // long TimeOut_ms = 1000;
    // Ret = AioStartAiSync(id_, TimeOut_ms);
    Ret = AioStartAi(id_);
    if(!isValidStatus(Ret)) ROS_ERROR("AioStartAi Failed");
  }

  void stopContinuousRead()
  {
    long Ret = AioStopAi(id_);
  }

  bool isValidStatus(long Ret)
  {
    if(Ret == 0)
    {
      return true;
    }
    else
    {
      char ErrorString[256];
      AioGetErrorString(Ret, ErrorString);
      printf("Device Message: %s, %ld\n", ErrorString, Ret);
      Ret = AioExit(id_);
      return false;
    }
  }

  bool checkRangeOfMaxChannel(short AiChannels)
  {
    short	MaxAiChannels;
    long Ret = AioGetAiMaxChannels(id_, &MaxAiChannels);
    if(!isValidStatus(Ret))
    {
      ROS_ERROR("AioGetAiMaxChannels Failed");
      return false;
    }
    if((AiChannels < 1) || (AiChannels > MaxAiChannels))
    {
      ROS_ERROR("\nチャネル数が設定可能範囲外です\n\n");
      return false;
    }
    return true;
  }

  ros::NodeHandle nh_;
  ros::Publisher pub_[8];
  short id_;
  int ai_channels;
  double rate_;
};
}// namespace contec

int main(int argc, char **argv)
{
  ros::init(argc, argv, "contec_adc_driver");
  contec::AnalogInput contec_driver(10000);
  contec_driver.startPublish();
  ros::spin();
}
