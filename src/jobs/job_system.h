#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace raytracer {

    using JobFunction = void (*)(void* data);

    struct alignas(64) Job {
        JobFunction function = nullptr;
        void* data = nullptr;
        std::atomic<int> unfinished_jobs{ 1 };
        Job* parent = nullptr;

        Job() {}
    };

    class JobSystem {
    public:
        static constexpr int kMaxJobs = 4096;

        explicit JobSystem(int num_threads = 0);
        ~JobSystem();

        JobSystem(const JobSystem&) = delete;
        JobSystem& operator=(const JobSystem&) = delete;

        Job* CreateJob(JobFunction function, void* data = nullptr, Job* parent = nullptr);
        void Run(Job* job);
        void Wait(Job* job);
        Job* Dispatch(JobFunction function, void* data = nullptr, Job* parent = nullptr);
        void WaitAll();

        int GetWorkerCount() const { return static_cast<int>(workers_.size()); }

    private:
        struct Worker {
            std::thread thread;
            Job* queue[kMaxJobs]{};
            std::atomic<int> top{ 0 };
            std::atomic<int> bottom{ 0 };

            Worker() {}
            Worker(const Worker&) = delete;
            Worker& operator=(const Worker&) = delete;
        };

        void WorkerLoop(int worker_id);
        Job* Steal(int thief_id);
        void Finish(Job* job);
        void Push(Worker& worker, Job* job);
        Job* Pop(Worker& worker);

        std::vector<std::unique_ptr<Worker>> workers_;
        std::atomic<bool> running_{ true };

        std::unique_ptr<Job[]> job_pool_;
        std::atomic<int> job_pool_index_{ 0 };

        std::mutex external_mutex_;
        std::queue<Job*> external_queue_;

        std::mutex wait_mutex_;
        std::condition_variable wait_cv_;
        std::atomic<int> active_jobs_{ 0 };
    };

}  // namespace raytracer