#pragma once

#include <mw/util/ThreadUtil.h>
#include <mw/common/Logger.h>
#include <tl/optional.hpp>

#include <map>
#include <mutex>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <cstdint>
#include <unordered_map>

class Task
{
public:
    Task(std::function<void()>&& task)
        : m_task(std::move(task)), m_intervalOpt(tl::nullopt) { }

    Task(std::function<void()>&& task, const std::chrono::milliseconds& interval)
        : m_task(std::move(task)), m_intervalOpt(tl::make_optional(interval)) { }

    void Execute() noexcept
    {
        try
        {
            // FUTURE: If named task, set thread name
            m_task();
        }
        catch (std::exception& e)
        {
            LOG_WARNING_F("Exception thrown: {}", e);
        }
    }

    tl::optional<std::chrono::milliseconds> GetInterval() const { return m_intervalOpt; }

private:
    std::function<void()> m_task;
    tl::optional<std::chrono::milliseconds> m_intervalOpt;
};

class Scheduler
{
public:
    using Ptr = std::shared_ptr<Scheduler>;

    static Scheduler::Ptr Create(const uint8_t numThreads)
    {
        auto pScheduler = std::shared_ptr<Scheduler>(new Scheduler());
        pScheduler->m_scheduler = std::thread(Scheduler::MainThread, pScheduler.get());

        for (uint8_t i = 0; i < numThreads; i++)
        {
            pScheduler->m_workers.push_back(std::thread(Scheduler::Worker, pScheduler.get()));
        }

        return pScheduler;
    }

    ~Scheduler()
    {
        m_stop = true;
        ThreadUtil::Join(m_scheduler);

        m_conditional.notify_all();
        ThreadUtil::Join(m_workers);
    }

    uint32_t RunEvery(std::function<void()>&& task, const std::chrono::milliseconds& interval)
    {
        const uint32_t taskId = m_nextId++;

        std::unique_lock<std::mutex> lock(m_taskMutex);
        m_tasksById.insert({ taskId, std::make_shared<Task>(std::move(task), interval) });
        m_tasks.insert({ std::chrono::system_clock::now() + interval, taskId });
        return taskId;
    }

    uint32_t RunOnce(std::function<void()>&& task)
    {
        const uint32_t taskId = m_nextId++;

        std::unique_lock<std::mutex> lock(m_taskMutex);
        m_tasksById.insert({ taskId, std::make_shared<Task>(std::move(task)) });
        m_tasks.insert({ std::chrono::system_clock::now(), taskId });
        m_conditional.notify_one();
        return taskId;
    }

    void RemoveTask(const uint32_t taskId)
    {
        std::unique_lock<std::mutex> lock(m_taskMutex);
        auto iter = m_tasksById.find(taskId);
        if (iter != m_tasksById.end())
        {
            m_tasksById.erase(iter);
        }
    }

private:
    Scheduler() : m_stop(false) {}

    static void MainThread(Scheduler* pScheduler)
    {
        while (!pScheduler->m_stop)
        {
            std::unique_lock<std::mutex> lock(pScheduler->m_taskMutex);
            if (!pScheduler->m_tasks.empty() && pScheduler->m_tasks.begin()->first <= std::chrono::system_clock::now())
            {
                pScheduler->m_conditional.notify_one();
                continue;
            }

            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    static void Worker(Scheduler* pScheduler)
    {
        while (!pScheduler->m_stop)
        {
            std::unique_lock<std::mutex> lock(pScheduler->m_taskMutex);
            pScheduler->m_conditional.wait(lock);
            if (pScheduler->m_stop)
            {
                break;
            }

            if (pScheduler->m_tasks.empty() || pScheduler->m_tasks.begin()->first > std::chrono::system_clock::now())
            {
                continue;
            }

            const uint32_t taskId = pScheduler->m_tasks.begin()->second;
            pScheduler->m_tasks.erase(pScheduler->m_tasks.begin());
            lock.unlock();

            auto taskIter = pScheduler->m_tasksById.find(taskId);
            if (taskIter != pScheduler->m_tasksById.end())
            {
                taskIter->second->Execute();
                auto intervalOpt = taskIter->second->GetInterval();

                std::unique_lock<std::mutex> taskLock(pScheduler->m_taskMutex);
                if (intervalOpt.has_value())
                {
                    pScheduler->m_tasks.insert({ std::chrono::system_clock::now() + intervalOpt.value(), taskId });
                }
                else
                {
                    pScheduler->m_tasksById.erase(pScheduler->m_tasksById.find(taskId));
                }
            }
        }
    }

    std::atomic_bool m_stop;
    std::atomic<uint32_t> m_nextId;
    std::thread m_scheduler;
    std::vector<std::thread> m_workers;
    std::condition_variable m_conditional;
    
    std::mutex m_taskMutex;
    std::unordered_map<uint32_t, std::shared_ptr<Task>> m_tasksById;
    std::multimap<std::chrono::time_point<std::chrono::system_clock>, uint32_t> m_tasks;
};