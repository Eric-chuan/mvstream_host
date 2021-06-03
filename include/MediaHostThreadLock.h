#ifndef __MEDIAHOSTTHREADLOCK_H__
#define __MEDIAHOSTTHREADLOCK_H__

#include "common.h"

#define MAX_LOCK_NUM 5

class MediaThreadLockClient {
  private:
    std::atomic<int> * m_preTriggers;
    std::atomic<int> * m_postTriggers;
    std::atomic<bool> *m_flags;

  public:
    void init(std::atomic<int> *preTriggers, std::atomic<int> *postTriggers,
              std::atomic<bool> *flags)
    {
        this->m_preTriggers  = preTriggers;
        this->m_postTriggers = postTriggers;
        this->m_flags        = flags;
    }

    void syncThread(int index)
    {
        this->m_preTriggers[index]++;
       //printf("trigger++ at %d\n", index);
        while (!this->m_flags[index]) {
          //printf("lock here %d\n", index);
          usleep(500);
        }
        this->m_postTriggers[index]++;
    }
};

class MediaThreadLock {
  private:
    std::atomic<int>  m_preTriggers[MAX_LOCK_NUM];
    std::atomic<int>  m_postTriggers[MAX_LOCK_NUM];
    std::atomic<bool> m_flags[MAX_LOCK_NUM];
    std::atomic<int>  m_clientNum;
    std::atomic<bool> m_active;

  private:
    void m_loop()
    {
        while (this->m_active) {
            for (int i = 0; i < MAX_LOCK_NUM; i++) {
                if (this->m_preTriggers[i] == m_clientNum) {
                    //printf("preTrigger success in %d\n", i);
                    this->m_preTriggers[i] = 0;
                    this->m_flags[i]       = true;
                }
                if (this->m_postTriggers[i] == m_clientNum) {
                    this->m_postTriggers[i] = 0;
                    this->m_flags[i]        = false;
                }
            }
            usleep(500);
        }
    }

  public:
    MediaThreadLock()
    {
        memset(this->m_preTriggers, 0, sizeof(std::atomic<int>) * MAX_LOCK_NUM);
        memset(this->m_postTriggers, 0, sizeof(std::atomic<int>) * MAX_LOCK_NUM);
        memset(this->m_flags, 0, sizeof(std::atomic<bool>) * MAX_LOCK_NUM);
        this->m_clientNum = 0;
        this->m_active    = true;
    }

    void clientReg(MediaThreadLockClient &client)
    {
        client.init(&this->m_preTriggers[0], &this->m_postTriggers[0], &this->m_flags[0]);
        this->m_clientNum++;
    }

    void startWatcher() { std::thread(&MediaThreadLock::m_loop, this).detach(); }

    void stopWatcher() { this->m_active = false; }
};

#endif