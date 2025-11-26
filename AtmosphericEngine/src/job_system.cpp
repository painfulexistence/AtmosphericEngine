#include "job_system.hpp"

// Tries to pop a job from the local queue or steal from another
bool JobSystem::GetJob(Job& job, uint32_t thread_index) {
    // First, try to pop from the front of the local queue
    {
        std::lock_guard<std::mutex> lock(*_threadMutexes[thread_index]);
        if (!_threadQueues[thread_index].empty()) {
            job = std::move(_threadQueues[thread_index].front());
            _threadQueues[thread_index].pop_front();
            return true;
        }
    }

    // Local queue is empty, try to steal from other threads
    uint32_t other_thread = (thread_index + 1) % _numThreads;
    while(other_thread != thread_index) {
        std::lock_guard<std::mutex> lock(*_threadMutexes[other_thread]);
        if (!_threadQueues[other_thread].empty()) {
            // Steal from the back of the other queue
            job = std::move(_threadQueues[other_thread].back());
            _threadQueues[other_thread].pop_back();
            return true;
        }
        other_thread = (other_thread + 1) % _numThreads;
    }

    return false; // No job found
}


JobSystem::JobSystem() {
#ifdef __EMSCRIPTEN__
    _numThreads = 1;
#else
	auto numCores = std::thread::hardware_concurrency();
    _numThreads = std::max(1u, numCores);

    _threadQueues.resize(_numThreads);
    _threadMutexes.resize(_numThreads);
    for (uint32_t i = 0; i < _numThreads; ++i) {
        _threadMutexes[i] = std::make_unique<std::mutex>();
    }

    for (uint32_t threadID = 0; threadID < _numThreads; ++threadID) {
        _threads.emplace_back([this, threadID]() {
            while (!_stopped) {
                Job job;
                if (GetJob(job, threadID)) {
                    job(threadID);
                    finishedLabel.fetch_add(1);

                    // If we just finished the last job, notify the waiting thread
                    if (!IsBusy()) {
                         std::unique_lock<std::mutex> lock(_waitMutex);
                        _waitCondition.notify_all();
                    }
                } else {
                    // No job found, yield to prevent busy-spinning
                    std::this_thread::yield();
                }
            }
        });
    }
#endif
}