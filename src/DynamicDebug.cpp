#include "DynamicDebug.h"
#include <cstring>
#include <memory> // for std::auto_ptr

//==============================================================================
DynamicDebug::DynamicDebug() {
  mpObserver = NULL;
  mInitialOK = false;

  // 不能直接赋值NULL给auto_ptr
  mpCurrentIndex = NULL;

  mpFile = NULL;
  mpStreamBuffer = std::cin.rdbuf(); // 保存std::cin的流，后面要重定向

  mRunning = false;
  mShowMessage = false;
  mRuntoCycle = Time(-3, 0);
}

//==============================================================================
DynamicDebug::~DynamicDebug() {
  Flush();

  mpIndex.reset();
  mpParserTime.reset();
  mpDecisionTime.reset();
  mpCommandSendTime.reset();
}

//==============================================================================
DynamicDebug &DynamicDebug::instance() {
  static DynamicDebug dynamic_debug;
  return dynamic_debug;
}

//==============================================================================
void DynamicDebug::Initial(Observer *pObserver) {
  if (mInitialOK == true) {
    return;
  }

  if (pObserver == NULL) {
    PRINT_ERROR("Observer Null Pointer");
    return;
  }

  mpObserver = pObserver;

  if (PlayerParam::instance().DynamicDebugMode() == true) // 动态调试
  {
    mpFileStream = std::auto_ptr<std::ifstream>(new std::ifstream("dynamicdebug.txt"));

    if (mpFileStream->is_open()) {
      std::cin.rdbuf(mpFileStream->rdbuf());
    }
  } else // 正常比赛时
  {
    if (PlayerParam::instance().SaveServerMessage() ==
        true) // 需要保存server信息
    {
      char file_name[64];
      sprintf(file_name, "%s/%s-%d-msg.log",
              PlayerParam::instance().logDir().c_str(),
              PlayerParam::instance().teamName().c_str(),
              mpObserver->SelfUnum());
      mpFile = fopen(file_name, "wb");
      if (mpFile) {
        if (setvbuf(mpFile, 0, _IOFBF, 1024 * 8192) != 0) {
          PRINT_ERROR("set buffer for file \"" << file_name << "\" error");
        }
      } else {
        PRINT_ERROR("open file \"" << file_name << "\" error");
      }
    }

    if (mpFile != NULL) {
      fseek(mpFile, sizeof(mFileHead) + 2 * sizeof(char),
            SEEK_SET); // 留出mFileHead的地方，最后再填
      mFileHead.mIndexTableSize = 0;
      mIndexTable.reserve(8192);
      mMessageTable.reserve(8192);
    }
  }
  mInitialOK = true;
}

//==============================================================================
void DynamicDebug::AddMessage(const char *msg, MessageType msg_type) {
  if (PlayerParam::instance().SaveServerMessage() &&
      !PlayerParam::instance().DynamicDebugMode()) {
    if (!mInitialOK) {
      return; // 没有初始化或者不用记录server信息都返回
    }

    mFileMutex.Lock(); // 有可能多个线程同时在写文件，所以要保证互斥

    MessageIndexTableUnit index_table_unit;
    index_table_unit.mServerTime = mpObserver->CurrentTime();
    index_table_unit.mDataSize = strlen(msg);

    switch (msg_type) {
    case MT_Parse:
      index_table_unit.mTimeOffset = mParserTimeTable.size();
      break;
    case MT_Run:
      index_table_unit.mTimeOffset = mDecisionTimeTable.size();
      break;
    case MT_Send:
      index_table_unit.mTimeOffset = mCommandSendTimeTable.size();
      break;
    default:
      break;
    }

    mMessageTable.push_back(Message(msg_type, msg));
    mIndexTable.push_back(index_table_unit);
    mFileHead.mMaxCycle =
        Max(mFileHead.mMaxCycle, index_table_unit.mServerTime);

    mFileMutex.UnLock();
  }
}

//==============================================================================
void DynamicDebug::AddTimeParser(timeval &time) {
  if (PlayerParam::instance().SaveServerMessage() &&
      !PlayerParam::instance().DynamicDebugMode()) {
    mParserTimeTable.push_back(time);
  }
}

//==============================================================================
void DynamicDebug::AddTimeDecision(timeval &time) {
  if (PlayerParam::instance().SaveServerMessage() &&
      !PlayerParam::instance().DynamicDebugMode()) {
    mDecisionTimeTable.push_back(time);
  }
}

//==============================================================================
void DynamicDebug::AddTimeCommandSend(timeval &time) {
  if (PlayerParam::instance().SaveServerMessage() &&
      !PlayerParam::instance().DynamicDebugMode()) {
    mCommandSendTimeTable.push_back(time);
  }
}

//==============================================================================
MessageType DynamicDebug::Run(char *msg) {
  std::cerr << std::endl << mpObserver->CurrentTime(); // 输出当前周期

  if (mRunning == true) {
    if (mRuntoCycle >= Time(0, 0)) {
      if (mRuntoCycle <= mpObserver->CurrentTime()) {
        mRunning = false;
        mRuntoCycle = 0;
      }
    }

    if (mRunning) {
      return GetMessage(msg);
    }
  }

  std::string read_msg;
  while (std::cin) {
    if (std::cin.eof()) {
      std::cin.rdbuf(mpStreamBuffer);
    }

    std::cerr << std::endl << ">>> ";
    std::cin >> read_msg;

    if (*read_msg.c_str() == '\0') {
      std::cin.rdbuf(mpStreamBuffer); // 到达文件末尾会读入'\0'，这里重定向
      continue;
    }

    if (read_msg == "load") {
      std::string file_name;
      std::cin >> file_name;
      mpFile = fopen(file_name.c_str(), "rb");
      if (mpFile == 0) {
        std::cerr << "Can't open dynamicdebug file, exit..." << std::endl;
        continue;
      }

      char ch1(0), ch2(0);

      if (fread(&ch1, sizeof(char), 1, mpFile) < 1) {
        Assert(0);
      }

      if (fread(&ch2, sizeof(char), 1, mpFile) < 1) {
        Assert(0);
      }

      if (ch1 != 'D' || ch2 != 'D') {
        std::cerr << "Not a dynamicdebug logfile!" << std::endl;
        return MT_Null;
      }

      if (fread(&mFileHead, sizeof(mFileHead), 1, mpFile) < 1) {
        Assert(0);
      }

      long long size;

      size = mFileHead.mIndexTableSize;
      mpIndex = std::auto_ptr<MessageIndexTableUnit>(new MessageIndexTableUnit[size]);

      fseek(mpFile, mFileHead.mIndexTableOffset, SEEK_SET);
      if (size > 0 &&
          fread(mpIndex.get(), size * sizeof(MessageIndexTableUnit), 1, mpFile) < 1) {
        Assert(0);
      }

      size = mFileHead.mParserTableSize;
      mpParserTime = std::auto_ptr<timeval>(new timeval[size]);

      fseek(mpFile, mFileHead.mParserTableOffset, SEEK_SET);
      if (size > 0 &&
          fread(mpParserTime.get(), size * sizeof(timeval), 1, mpFile) < 1) {
        Assert(0);
      }

      size = mFileHead.mDecisionTableSize;
      mpDecisionTime = std::auto_ptr<timeval>(new timeval[size]);

      fseek(mpFile, mFileHead.mDecisionTableOffset, SEEK_SET);
      if (size > 0 &&
          fread(mpDecisionTime.get(), size * sizeof(timeval), 1, mpFile) < 1) {
        Assert(0);
      }

      size = mFileHead.mCommandSendTableSize;
      mpCommandSendTime = std::auto_ptr<timeval>(new timeval[size]);

      fseek(mpFile, mFileHead.mCommandSendTableOffset, SEEK_SET);
      if (size > 0 &&
          fread(mpCommandSendTime.get(), size * sizeof(timeval), 1, mpFile) < 1) {
        Assert(0);
      }

      fseek(mpFile, sizeof(mFileHead) + 2 * sizeof(char), SEEK_SET);
      mpCurrentIndex = mpIndex.get(); // load后，第一个为初始化信息，先进行初始化

      std::cerr << "Load finished." << std::endl;
      fseek(mpFile, mpCurrentIndex->mDataOffset, SEEK_SET);
      if (fread(msg, 1, 1, mpFile) < 1) {
        Assert(0);
      }
      if (mpCurrentIndex->mDataSize > 0 &&
          fread(msg, mpCurrentIndex->mDataSize, 1, mpFile) < 1) {
        Assert(0);
      }
      msg[mpCurrentIndex->mDataSize] = '\0';
      return MT_Parse;
    } else if (read_msg == "step" || read_msg == "s") {
      if (mpCurrentIndex == 0) {
        std::cerr << "no file loaded!";
        continue;
      }
      return GetMessage(msg);
    } else if (read_msg == "goto" || read_msg == "g") {
      if (mpCurrentIndex == 0) {
        std::cerr << "no file loaded!";
        continue;
      }

      int cycle;
      std::cin >> cycle;
      if (FindCycle(cycle) == true) {
        std::cerr << "goto finished ..." << std::endl;
      } else {
        std::cerr << "no such cycle ..." << std::endl;
      }
      continue;
    } else if (read_msg == "runto" || read_msg == "rt") {
      if (mpCurrentIndex == 0) {
        std::cerr << "no file loaded!";
        continue;
      }

      std::string str;
      std::cin >> str;

      char a;
      int t; // time
      int s; // stop time
      if (sscanf(str.c_str(), "%d%c%d", &t, &a, &s) != 3) {
        s = 0;
      }

      mRuntoCycle = Time(t, s);

      if (mRuntoCycle == mpObserver->CurrentTime()) {
        std::cerr << "already here ...";
        continue;
      } else if (mRuntoCycle < mpObserver->CurrentTime()) {
        std::cerr << "can not run to previous cycle";
        continue;
      } else {
        mRunning = true;
        return GetMessage(msg);
      }
    } else if (read_msg == "run" || read_msg == "r") {
      mRuntoCycle = -1;
      mRunning = true;
      return GetMessage(msg);
    } else if (read_msg == "msg" || read_msg == "m") {
      mShowMessage = !mShowMessage;
      std::cerr << "Set ShowMessage: " << mShowMessage << std::endl;
      continue;
    } else if (read_msg == "quit" || read_msg == "q") {
      std::cerr << "Bye ..." << std::endl;
      return MT_Null;
    } else {
      std::cerr << "Error command, only the following commands are aviliable: "
                << std::endl;
      std::cerr << "\tload" << std::endl;
      std::cerr << "\tstep(s)" << std::endl;
      std::cerr << "\trunto(rt)" << std::endl;
      std::cerr << "\tgoto(g)" << std::endl;
      std::cerr << "\trun(r)" << std::endl;
      std::cerr << "\tmsg(m)" << std::endl;
      std::cerr << "\ttype(t)" << std::endl;
      std::cerr << "\tquit(q)" << std::endl;
      continue;
    }
  }

  return MT_Null;
}

//==============================================================================
MessageType DynamicDebug::GetMessage(char *msg) {
  if (mpCurrentIndex == 0) {
    return MT_Null;
  }

  if (mpCurrentIndex->mServerTime >= mFileHead.mMaxCycle) {
    std::cerr << "End ..." << std::endl;
    return MT_Null;
  } else {
    ++mpCurrentIndex;
  }

  fseek(mpFile, mpCurrentIndex->mDataOffset, SEEK_SET);
  if (fread(msg, 1, 1, mpFile) < 1) // 读取信息类型
  {
    Assert(0);
  }
  MessageType msg_type = (MessageType)msg[0];

  if (mpCurrentIndex->mDataSize > 0 &&
      fread(msg, mpCurrentIndex->mDataSize, 1, mpFile) < 1) // 读取信息内容
  {
    Assert(0);
  }
  msg[mpCurrentIndex->mDataSize] = '\0';

  if (mShowMessage == true) {
    std::cerr << msg << std::endl;
  }

  return msg_type;
}

//==============================================================================
bool DynamicDebug::FindCycle(int cycle) {
  Time cycle_time(cycle, 0);
  if (cycle_time == mpObserver->CurrentTime()) {
    return true;
  }

  int begin = 0;
  int end = mFileHead.mIndexTableSize - 1;
  int mid;
  while (begin < end) {
    mid = (begin + end) / 2;
    MessageIndexTableUnit* pIndex = mpIndex.get();
    if (pIndex[mid].mServerTime == cycle_time) {
      mpCurrentIndex = &pIndex[mid];
      return true;
    } else if (pIndex[mid].mServerTime < cycle_time) {
      begin = mid;
    } else {
      end = mid;
    }
  }
  return false;
}

//==============================================================================
timeval DynamicDebug::GetTimeParser() {
  timeval* pParserTime = mpParserTime.get();
  timeval time_val = pParserTime[mpCurrentIndex->mTimeOffset++];
  return time_val;
}

//==============================================================================
timeval DynamicDebug::GetTimeDecision() {
  timeval* pDecisionTime = mpDecisionTime.get();
  timeval time_val = pDecisionTime[mpCurrentIndex->mTimeOffset++];
  return time_val;
}

//==============================================================================
timeval DynamicDebug::GetTimeCommandSend() {
  timeval* pCommandSendTime = mpCommandSendTime.get();
  timeval time_val = pCommandSendTime[mpCurrentIndex->mTimeOffset++];
  return time_val;
}

void DynamicDebug::Flush() {
  if (PlayerParam::instance().SaveServerMessage() &&
      !PlayerParam::instance().DynamicDebugMode()) {
    if (mpFile != NULL) {
      long long i = 0; // 循环变量
      // mFileHead.mHeadFlag[0] = 'D';
      // mFileHead.mHeadFlag[1] = 'D';

      // 赋值4个表的大小，后面将要写到文件中
      mFileHead.mIndexTableSize = mIndexTable.size();
      mFileHead.mParserTableSize = mParserTimeTable.size();
      mFileHead.mDecisionTableSize = mDecisionTimeTable.size();
      mFileHead.mCommandSendTableSize = mCommandSendTimeTable.size();

      // message
      long long size = mFileHead.mIndexTableSize;
      for (i = 0; i < size; ++i) {
        mIndexTable[i].mDataOffset = ftell(mpFile);
        fwrite(&mMessageTable[i].mType, 1, 1, mpFile); // 先写入消息类型
        fwrite(mMessageTable[i].mString.c_str(), mIndexTable[i].mDataSize, 1,
               mpFile);
      }

      // index table
      mFileHead.mIndexTableOffset = ftell(mpFile);
      mpIndex = std::auto_ptr<MessageIndexTableUnit>(new MessageIndexTableUnit[size]);
      MessageIndexTableUnit* pIndex = mpIndex.get();

      for (i = 0; i < size; ++i) {
        memcpy(&pIndex[i], &mIndexTable[i], sizeof(MessageIndexTableUnit));
      }
      fwrite(mpIndex.get(), size * sizeof(MessageIndexTableUnit), 1, mpFile);

      // observer time
      mFileHead.mParserTableOffset = ftell(mpFile);
      size = mFileHead.mParserTableSize;
      mpParserTime = std::auto_ptr<timeval>(new timeval[size]);
      timeval* pParserTime = mpParserTime.get();

      for (i = 0; i < size; ++i) {
        pParserTime[i] = mParserTimeTable[i];
      }
      fwrite(mpParserTime.get(), size * sizeof(timeval), 1, mpFile);

      // decision time
      mFileHead.mDecisionTableOffset = ftell(mpFile);
      size = mFileHead.mDecisionTableSize;
      mpDecisionTime = std::auto_ptr<timeval>(new timeval[size]);
      timeval* pDecisionTime = mpDecisionTime.get();

      for (i = 0; i < size; ++i) {
        pDecisionTime[i] = mDecisionTimeTable[i];
      }
      fwrite(mpDecisionTime.get(), size * sizeof(timeval), 1, mpFile);

      // commandsend time
      mFileHead.mCommandSendTableOffset = ftell(mpFile);
      size = mFileHead.mCommandSendTableSize;
      mpCommandSendTime = std::auto_ptr<timeval>(new timeval[size]);
      timeval* pCommandSendTime = mpCommandSendTime.get();

      for (i = 0; i < size; ++i) {
        pCommandSendTime[i] = mCommandSendTimeTable[i];
      }
      fwrite(mpCommandSendTime.get(), size * sizeof(timeval), 1, mpFile);

      fseek(mpFile, 0, SEEK_SET);
      fprintf(mpFile, "DD");
      fwrite(&mFileHead, sizeof(mFileHead), 1, mpFile);
    }

    if (mpFile != NULL) {
      fclose(mpFile);
    }
  }
}

// end of file DynamicDebug.cpp