/************************************************************************************
 * WrightEagle (Soccer Simulation League 2D) * BASE SOURCE CODE RELEASE 2016 *
 * Copyright (c) 1998-2016 WrightEagle 2D Soccer Simulation Team, * Multi-Agent
 * Systems Lab.,                                * School of Computer Science and
 * Technology,               * University of Science and Technology of China *
 * All rights reserved. *
 ************************************************************************************/

#include "core/Coach.h"
#include "./DynamicDebug.h"
#include "utils/Logger.h"
#include "core/Player.h"
#include "params/PlayerParam.h"
#include "params/ServerParam.h"
#include "core/Trainer.h"

 #include <cstdlib>
 #include <cstring>
 #include <iostream>
 #include <sstream>
 #include <string>

 #ifndef WIN32
 #include <signal.h>
 #include <unistd.h>
 #endif

 namespace {
 #ifndef WIN32
 /**
  * 创建 core dump 文件
  */
 void create_dump() {
     if (!fork()) {
         abort(); // 触发 SIGABRT 信号以生成 core dump 文件
     }
 }

 /**
  * 信号捕获器，用于处理系统信号
  */
 void signal_catcher(int sig) {
     signal(sig, signal_catcher);

     std::cerr << "Caught signal: " << sig << std::endl;

     create_dump(); // 生成 core dump 文件

     std::cerr << "Exiting safely..." << std::endl;

     Logger::instance().LogSight();
     Logger::instance().SetFlushCond(); // 设置日志刷新条件，确保日志写入文件
     DynamicDebug::instance().AddMessage("---Aborted", MT_Send);

     WaitFor(100); // 等待日志线程完成刷新

     exit(0); // 正常退出，触发静态对象析构
 }
 #endif
 } // namespace

 /**
  * 注册信号处理器
  */
 void RegisterSignalHandler() {
 #ifndef WIN32
     const int signals[] = {SIGINT, SIGQUIT, SIGSEGV, SIGILL, SIGFPE};
     for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
         if (signal(signals[i], signal_catcher) == SIG_ERR) {
             perror(("Failed to register signal: " + std::to_string(signals[i])).c_str());
             exit(EXIT_FAILURE);
         }
     }
 #endif
 }

 //==============================================================================
 int main(int argc, char *argv[]) {
     try {
         // 注册信号处理器
         RegisterSignalHandler();

         // 初始化参数
         ServerParam::instance().init(argc, argv);
         PlayerParam::instance().init(argc, argv);

         // 判断客户端类型
         const bool isCoach = PlayerParam::instance().isCoach();
         const bool isTrainer = PlayerParam::instance().isTrainer();

         // 创建客户端实例
         Client *client = 0;
         if (isCoach) {
             client = new Coach();
         } else if (isTrainer) {
             client = new Trainer();
         } else {
             client = new Player();
         }

         if (!client) {
             std::cerr << "Failed to create client instance" << std::endl;
             return EXIT_FAILURE;
         }

         // 根据动态调试模式选择运行方式
         if (PlayerParam::instance().DynamicDebugMode()) {
             client->RunDynamicDebug(); // 动态调试模式
         } else {
             client->RunNormal(); // 正常比赛模式
         }

         // 释放资源
         delete client;

     } catch (const std::exception &e) {
         // 捕获并处理已知异常
         std::cerr << "Exception caught: " << e.what() << std::endl;
     } catch (...) {
         // 捕获未知异常
         std::cerr << "Unknown exception caught" << std::endl;
     }

     return EXIT_SUCCESS; // 返回成功状态码
 }