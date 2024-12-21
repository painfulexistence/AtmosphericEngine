#include "job_system.hpp"

JobSystem::JobSystem() {
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

    _groups[0] = std::make_shared<JobGroup>();
}

JobSystem::~JobSystem() {
    _stopped = true;
    _queueCondition.notify_all();

    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
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
    currentLabel += 1;
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _jobQueue.push(job);
    }
    _queueCondition.notify_one();
}

bool JobSystem::IsBusy() {
    return finishedLabel.load() < currentLabel;
}

void JobSystem::Wait() {
    std::unique_lock<std::mutex> lock(_waitMutex);
    _waitCondition.wait(lock, [this]() {
        return !IsBusy();
    });
    // Or:
    // while (IsBusy()) {
    //     std::this_thread::yield();
    // }
}

JobGroupID JobSystem::CreateGroup() {
    _groups[_nextGroupID] = std::make_shared<JobGroup>();
    return _nextGroupID++;
}

void JobSystem::WaitGroup(JobGroupID id) {
    std::shared_ptr<JobGroup> group;
    {
        std::lock_guard<std::mutex> lock(_groupsMutex);
        group = _groups.at(id);
    }

    std::unique_lock<std::mutex> lock(_waitMutex);
    _waitCondition.wait(lock, [group]() {
        return group->finishedLabel.load() >= group->currentLabel;
    });
}