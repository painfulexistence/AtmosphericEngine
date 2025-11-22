#include "job_system.hpp"

JobSystem::JobSystem() {
#ifdef __EMSCRIPTEN__
    _numThreads = 1;
#else
	auto numCores = std::thread::hardware_concurrency();

    _numThreads = std::max(1u, numCores);
    for (uint32_t threadID = 0; threadID < _numThreads; ++threadID) {
        _threads.emplace_back([this, threadID]() {
            while (!_stopped) {
                // std::unique_ptr<IJob> job;
                // {
                //     std::unique_lock<std::mutex> lock(_queueMutex);
                //     _queueCondition.wait(lock, [this]() {
                //         return !_jobQueue.empty() || _stopped;
                //     });

                //     if (_stopped && _jobQueue.empty()) {
                //         return;
                //     }
                //     job = std::move(_jobQueue.front());
                //     _jobQueue.pop();
                // }
                // job->Execute(threadID);
                Job job;
                {
                    std::unique_lock<std::mutex> lock(_queueMutex);
                    _queueCondition.wait(lock, [this]() {
                        return !_jobQueue.empty() || _stopped;
                    });
                    if (_stopped && _jobQueue.empty()) {
                        return;
                    }
                    job = _jobQueue.front();
                    _jobQueue.pop();
                }
                job(threadID);

                finishedLabel.fetch_add(1);
                _waitCondition.notify_all();
            }
        });
    }
#endif

    _groups[0] = std::make_shared<JobGroup>();
}

JobSystem::~JobSystem() {
#ifndef __EMSCRIPTEN__
    _stopped = true;
    _queueCondition.notify_all();

    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
#endif
}

void JobSystem::Init() {
}

// void JobSystem::Submit(std::unique_ptr<IJob> job) {
//     currentLabel += 1;
//     {
//         std::lock_guard<std::mutex> lock(_queueMutex);
//         _jobQueue.push(std::move(job));
//     }
//     _queueCondition.notify_one();
// }

void JobSystem::Execute(const Job& job, JobGroupID groupID) {
#ifdef __EMSCRIPTEN__
    job(0);
#else
    currentLabel += 1;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _jobQueue.push(job);
    }
    _queueCondition.notify_one();
#endif
}

bool JobSystem::IsBusy() {
#ifdef __EMSCRIPTEN__
    return false;
#else
    return finishedLabel.load() < currentLabel;
#endif
}

void JobSystem::Wait() {
#ifdef __EMSCRIPTEN__
    return;
#else
    std::unique_lock<std::mutex> lock(_waitMutex);
    _waitCondition.wait(lock, [this]() {
        return !IsBusy();
    });
    // Or:
    // while (IsBusy()) {
    //     std::this_thread::yield();
    // }
#endif
}

JobGroupID JobSystem::CreateGroup() {
#ifdef __EMSCRIPTEN__
    return 0;
#else
    _groups[_nextGroupID] = std::make_shared<JobGroup>();
    return _nextGroupID++;
#endif
}

void JobSystem::WaitGroup(JobGroupID id) {
#ifdef __EMSCRIPTEN__
    return;
#else
    std::shared_ptr<JobGroup> group;
    {
        std::lock_guard<std::mutex> lock(_groupsMutex);
        group = _groups.at(id);
    }

    std::unique_lock<std::mutex> lock(_waitMutex);
    _waitCondition.wait(lock, [group]() {
        return group->finishedLabel.load() >= group->currentLabel;
    });
#endif
}