#ifndef PS_POTCREATOR_THREADPOOL_H
#define PS_POTCREATOR_THREADPOOL_H

#include <functional>
#include <vector>
#include <array>
#include <deque>
#include <thread>
#include <mutex>
#include <chrono>
#include <iostream>
#include <atomic>

namespace ps {
namespace potcreator {

/**
 * ThreadPool
 *
 * Example Usage:
 * ```cpp
 * ThreadPool<std::vector<Output>, 3> pool;
 * // ...
 * pool.spawn(someLambda);
 * pool.spawn(someLambda);
 * pool.spawn(someLambda);
 * // ...
 * pool.sync();
 * ```
 *
 * @tparam ResponseType
 * @tparam ThreadCount
 */
template<typename ResponseType, uint32_t ThreadCount = 3>
class ThreadPool
{
public:
  ThreadPool()
  {

  }

  /**
   * dtor
   *
   * makes sure, that threads are finished
   */
  ~ThreadPool()
  {
    waitForThreadsFinished();
  }

  /**
   * add a task to the queue
   */
  void spawn(const std::function<ResponseType()>& task)
  {
    {
      std::lock_guard<std::mutex> lock(taskLock);
      tasks.push_back(task);
    }
  }

  /**
   * Sync
   *
   * Run all tasks that are in queue and wait until all threads are finished.
   *
   * We expect wanted tasks to be already spawn. This way we don't block ourselves with the lock all the time when
   * checking for new tasks.
   */
  void sync()
  {
    start();
    waitForThreadsFinished();
  }

  /**
   * get task responses
   *
   * should only be called after a call to sync
   */
  std::vector<ResponseType> getResponses()
  {
    {
      std::lock_guard<std::mutex> lock(responseLock);
      return responses;
    }
  }

  /**
   * clear Responses
   *
   * clear the responses, so that the ThreadPool is reusable
   */
  void clearResponses()
  {
    {
      std::lock_guard<std::mutex> lock(responseLock);
      responses.clear();
    }
  }

private:
  std::mutex taskLock;
  std::deque<std::function<ResponseType()>> tasks;
  std::mutex responseLock;
  std::vector<ResponseType> responses;

  std::array<std::atomic_bool, ThreadCount> isWorkerWorking;
  std::array<std::thread, ThreadCount> worker;

  void start()
  {
    for (uint32_t i = 0; i < ThreadCount; ++i)
    {
      isWorkerWorking[i].store(false, std::memory_order_relaxed);

      this->worker[i] = std::thread([this, i]()
      {
        while (true)
        {
          std::function<ResponseType()> task;

          {
            std::lock_guard<std::mutex> lock(taskLock);

            if (tasks.empty())
            {
              return;
            }

            isWorkerWorking[i].store(true, std::memory_order_relaxed);

            task = tasks.front();
            tasks.pop_front();
          }

          ResponseType res = task();

          {
            std::lock_guard<std::mutex> lock(responseLock);
            responses.push_back(res);
          }

          isWorkerWorking[i].store(false, std::memory_order_relaxed);
        }
      });

      if (this->worker[i].joinable())
      {
        this->worker[i].detach();
      }
    }
  }

  bool areWorkersDone()
  {
    bool areTasksEmpty = false;

    {
      std::lock_guard <std::mutex> lock(taskLock);
      areTasksEmpty = tasks.empty();
    }

    if (areTasksEmpty)
    {
      for (const auto& flag : isWorkerWorking)
      {
        if (flag.load(std::memory_order_relaxed))
        {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  void waitForThreadsFinished()
  {
    if (areWorkersDone())
    {
      return;
    }

    while (true)
    {
      std::this_thread::sleep_for(std::chrono::seconds(3));

      if (areWorkersDone())
      {
        return;
      }
    }
  }
};

} // namespace potcreator
} // namespace ps

#endif // PS_POTCREATOR_THREADPOOL_H
