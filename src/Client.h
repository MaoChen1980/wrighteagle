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

#ifndef CLIENT_H_
#define CLIENT_H_

// 定义Client类，提供足球模拟比赛中的客户端功能接口
class Observer;
class WorldModel;
class Agent;
class Parser;
class CommandSender;

// Client类，负责与服务器的通信、解析数据、决策等
class Client {
  friend class Player;     // 允许Player类访问Client的私有和保护成员
  friend class Coach;      // 允许Coach类访问Client的私有和保护成员
  friend class Trainer;    // 允许Trainer类访问Client的私有和保护成员

  Observer *mpObserver;   // 观察者对象指针
  WorldModel *mpWorldModel; // 世界模型对象指针
  Agent *mpAgent;         // 球员代理对象指针

  Parser *mpParser;       // 解析器对象指针
  CommandSender *mpCommandSender; // 命令发送器对象指针

public:
  Client();               // Client类的构造函数
  virtual ~Client();      // Client类的虚析构函数

  /**
   * 动态调试时球员入口函数
   * 无参数
   * 无返回值
   */
  void RunDynamicDebug();

  /**
   * 正常比赛时的球员入口函数
   * 无参数
   * 无返回值
   */
  void RunNormal();

  /**
   * 正常比赛时的球员主循环函数
   * 无参数
   * 无返回值
   */
  void MainLoop();

  /**
   * 创建Agent，并完成相关调用
   * 无参数
   * 无返回值
   */
  void ConstructAgent();

  /**
   * 球员决策函数，每周期执行1次。
   * 该函数是纯虚函数，需要在派生类中实现具体的决策逻辑。
   * 无参数
   * 无返回值
   */
  virtual void Run() = 0;

  /**
   * 给server发送一些选项，如synch_see,eye_on等。
   * 该函数是纯虚函数，需要在派生类中实现具体发送选项的逻辑。
   * 无参数
   * 无返回值
   */
  virtual void SendOptionToServer() = 0;
};

#endif /* CLIENT_H_ */