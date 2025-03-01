/************************************************************************************
 * WrightEagle (Soccer Simulation League 2D) * BASE SOURCE CODE RELEASE 2016 *
 * Copyright (c) 1998-2016 WrightEagle 2D Soccer Simulation Team, * Multi-Agent
 *Systems Lab.,                                * School of Computer Science and
 *Technology,               * University of Science and Technology of China *
 * All rights reserved. *
 *                                                                                  *
 * Redistribution and use in source and binary forms, with or without *
 * modification, are permitted provided that the following conditions are met: *
 *     * Redistributions of source code must retain the above copyright *
 *       notice, this list of conditions and the following disclaimer. *
 *     * Redistributions in binary form must reproduce the above copyright *
 *       notice, this list of conditions and the following disclaimer in the *
 *       documentation and/or other materials provided with the distribution. *
 *     * Neither the name of the WrightEagle 2D Soccer Simulation Team nor the *
 *       names of its contributors may be used to endorse or promote products *
 *       derived from this software without specific prior written permission. *
 *                                                                                  *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND  * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED    * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *PURPOSE ARE           * DISCLAIMED. IN NO EVENT SHALL WrightEagle 2D Soccer
 *Simulation Team BE LIABLE    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *EXEMPLARY, OR CONSEQUENTIAL       * DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *PROCUREMENT OF SUBSTITUTE GOODS OR       * SERVICES; LOSS OF USE, DATA, OR
 *PROFITS; OR BUSINESS INTERRUPTION) HOWEVER       * CAUSED AND ON ANY THEORY OF
 *LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,    * OR TORT (INCLUDING
 *NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF * THIS SOFTWARE,
 *EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
 ************************************************************************************/

#include "core/Coach.h"
#include "DynamicDebug.h"
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
#include <stdlib.h>
#include <unistd.h>
#endif

namespace {
#ifndef WIN32
void create_dump(void) {
  if (!fork()) {
    abort(); // cause to send SIGABRT to itself, a core dump is generated by
             // default
  }
}
#endif

void signal_catcher(int sig) {
#ifndef WIN32
  signal(sig, signal_catcher);

  std::cerr << "catch signal " << sig << std::endl;

  create_dump(); //先产生coredump

  std::cerr << "exiting safely now" << std::endl;

  Logger::instance().LogSight();
  Logger::instance().SetFlushCond(); // set flush cond and let the logger thread
                                     // flush the logs to file.
  DynamicDebug::instance().AddMessage("---Aborted", MT_Send);

  WaitFor(100); // wait logger thread to finish flushing

  exit(0); //这里会调用静态对象的析构函数
#endif
}
} // namespace

void RegisterSignalHandler();

//==============================================================================
int main(int argc, char *argv[]) {
  RegisterSignalHandler();

  ServerParam::instance().init(argc, argv);
  PlayerParam::instance().init(argc, argv);

  Client *client = 0;

  if (PlayerParam::instance().isCoach()) {
    client = new Coach;
  } else if (PlayerParam::instance().isTrainer()) {
    client = new Trainer;
  } else {
    client = new Player;
  }

  if (PlayerParam::instance().DynamicDebugMode()) {
    client->RunDynamicDebug(); // 进入动态调试模式
  } else {
    client->RunNormal(); // 进入正常比赛模式
  }

  delete client;

  return 0;
}

void RegisterSignalHandler() {
#ifndef WIN32
  if (signal(SIGINT, signal_catcher) == SIG_ERR) //中断
    perror("SIGINT"), exit(1);
  if (signal(SIGQUIT, signal_catcher) == SIG_ERR) //退出
    perror("SIGQUIT"), exit(1);
  //	if(signal(SIGABRT, signal_catcher) == SIG_ERR)
  ////不能注册SIGABRT，否则调用 abort 时，会产生死循环 		perror("SIGABRT"),
  //exit(1);
  if (signal(SIGSEGV, signal_catcher) == SIG_ERR) //段错误
    perror("SIGSEGV"), exit(1);
  if (signal(SIGILL, signal_catcher) == SIG_ERR) //非法指令
    perror("SIGILL"), exit(1);
  if (signal(SIGFPE, signal_catcher) == SIG_ERR) //浮点异常
    perror("SIGFPE"), exit(1);
#endif
}
