#pragma once
#include "asset_manager.hpp"
#include "globals.hpp"

using JobGroupID = uint32_t;

struct JobGroup {
    std::mutex mutex;
    std::condition_variable condition;
    uint64_t currentLabel = 0;
    std::atomic<uint64_t> finishedLabel{ 0 };
};

using Job = std::function<void(int)>;

class IJob {
public:
    virtual void Execute(int threadIndex) = 0;
    virtual ~IJob() = default;
};

class SampleJob : public IJob {
private:
    int _jobId;

public:
    explicit SampleJob(int jobId) : _jobId(jobId) {
    }

    void Execute(int threadIndex) override {
        fmt::print("start Job {} on thread {}\n", _jobId, threadIndex);

        float fakeWork = 0.0f;
        for (int i = 0; i < 1000000; i++) {
            fakeWork = 1.21f * fakeWork;
        }

        fmt::print("finished Job {} on thread {}\n", _jobId, threadIndex);
    }
};

class TextureLoadJob : public IJob {
private:
    std::string _path;
    std::shared_ptr<Image>* _result;
    std::atomic<bool>* _completed;

public:
    TextureLoadJob(const std::string& path, std::shared_ptr<Image>* result, std::atomic<bool>* completed)
      : _path(path), _result(result), _completed(completed) {
    }

    void Execute(int threadIndex) override {
        *_result = AssetManager::Get().LoadImage(_path);
        *_completed = true;
    }
};

class JobSystem {
public:
    static JobSystem* Get() {
        static JobSystem instance;
        return &instance;
    }

    JobSystem();
    ~JobSystem();

    void Init();
    void Execute(const Job& job, JobGroupID groupID = 0);
    // void Submit(std::unique_ptr<IJob> job);
    bool IsBusy();
    void Wait();

    JobGroupID CreateGroup();
    void WaitGroup(JobGroupID groupID);

private:
    uint32_t _numThreads = 0;
    std::vector<std::thread> _threads;
    // std::queue<std::unique_ptr<IJob>> _jobQueue;
    std::queue<Job> _jobQueue;
    std::mutex _queueMutex;
    std::condition_variable _queueCondition;
    std::mutex _waitMutex;
    std::condition_variable _waitCondition;
    std::atomic<bool> _stopped{ false };
    uint64_t currentLabel = 0;
    std::atomic<uint64_t> finishedLabel{ 0 };

    std::mutex _groupsMutex;
    std::unordered_map<JobGroupID, std::shared_ptr<JobGroup>> _groups;
    std::atomic<JobGroupID> _nextGroupID = 1;
};