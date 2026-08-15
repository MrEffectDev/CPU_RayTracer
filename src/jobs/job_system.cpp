#include "jobs/job_system.h"
#include <chrono>

namespace raytracer {

    JobSystem::JobSystem(int num_threads) {
        if (num_threads <= 0) {
            num_threads = static_cast<int>(std::thread::hardware_concurrency()) - 1;
            if (num_threads < 1) num_threads = 1;
        }

        job_pool_ = std::make_unique<Job[]>(kMaxJobs);

        workers_.reserve(num_threads);
        for (int i = 0; i < num_threads; ++i) {
            workers_.push_back(std::make_unique<Worker>());
            workers_[i]->thread = std::thread([this, i]() {
                WorkerLoop(i);
                });
        }
    }

    JobSystem::~JobSystem() {
        running_ = false;
        wait_cv_.notify_all();

        for (auto& worker : workers_) {
            if (worker && worker->thread.joinable()) {
                worker->thread.join();
            }
        }
    }

    Job* JobSystem::CreateJob(JobFunction function, void* data, Job* parent) {
        const int index = job_pool_index_.fetch_add(1, std::memory_order_relaxed) % kMaxJobs;
        Job* job = &job_pool_[index];

        job->function = function;
        job->data = data;
        job->parent = parent;
        job->unfinished_jobs.store(1, std::memory_order_relaxed);

        if (parent) {
            parent->unfinished_jobs.fetch_add(1, std::memory_order_relaxed);
        }

        return job;
    }

    void JobSystem::Run(Job* job) {
        active_jobs_.fetch_add(1, std::memory_order_relaxed);

        {
            std::lock_guard<std::mutex> lock(external_mutex_);
            external_queue_.push(job);
        }
        wait_cv_.notify_one();
    }

    Job* JobSystem::Dispatch(JobFunction function, void* data, Job* parent) {
        Job* job = CreateJob(function, data, parent);
        Run(job);
        return job;
    }

    void JobSystem::Wait(Job* job) {
        while (job->unfinished_jobs.load(std::memory_order_acquire) > 0) {
            Job* next_job = nullptr;
            {
                std::lock_guard<std::mutex> lock(external_mutex_);
                if (!external_queue_.empty()) {
                    next_job = external_queue_.front();
                    external_queue_.pop();
                }
            }

            if (!next_job) {
                next_job = Steal(-1);
            }

            if (next_job) {
                next_job->function(next_job->data);
                Finish(next_job);
            }
            else {
                std::this_thread::yield();
            }
        }
    }

    void JobSystem::WaitAll() {
        while (active_jobs_.load(std::memory_order_acquire) > 0) {
            Job* next_job = nullptr;
            {
                std::lock_guard<std::mutex> lock(external_mutex_);
                if (!external_queue_.empty()) {
                    next_job = external_queue_.front();
                    external_queue_.pop();
                }
            }

            if (!next_job) {
                next_job = Steal(-1);
            }

            if (next_job) {
                next_job->function(next_job->data);
                Finish(next_job);
            }
            else {
                std::this_thread::yield();
            }
        }
    }

    void JobSystem::WorkerLoop(int worker_id) {
        Worker& worker = *workers_[worker_id];

        while (running_) {
            Job* job = Pop(worker);

            if (!job) {
                std::lock_guard<std::mutex> lock(external_mutex_);
                if (!external_queue_.empty()) {
                    job = external_queue_.front();
                    external_queue_.pop();
                }
            }

            if (!job) {
                job = Steal(worker_id);
            }

            if (job) {
                job->function(job->data);
                Finish(job);
            }
            else {
                std::unique_lock<std::mutex> lock(wait_mutex_);
                wait_cv_.wait_for(lock, std::chrono::microseconds(50), [this]() {
                    return !running_ || active_jobs_.load(std::memory_order_acquire) > 0;
                    });
            }
        }
    }

    void JobSystem::Finish(Job* job) {
        const int unfinished = job->unfinished_jobs.fetch_sub(1, std::memory_order_acq_rel) - 1;

        if (unfinished == 0) {
            if (job->parent) {
                Finish(job->parent);
            }
            active_jobs_.fetch_sub(1, std::memory_order_relaxed);
            wait_cv_.notify_all();
        }
    }

    void JobSystem::Push(Worker& worker, Job* job) {
        const int b = worker.bottom.load(std::memory_order_relaxed);
        worker.queue[b % kMaxJobs] = job;
        worker.bottom.store(b + 1, std::memory_order_release);
    }

    Job* JobSystem::Pop(Worker& worker) {
        const int b = worker.bottom.load(std::memory_order_relaxed) - 1;
        worker.bottom.store(b, std::memory_order_relaxed);

        int t = worker.top.load(std::memory_order_acquire);

        if (t <= b) {
            Job* job = worker.queue[b % kMaxJobs];

            if (t != b) {
                return job;
            }

            if (!worker.top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst)) {
                job = nullptr;
            }
            worker.bottom.store(t + 1, std::memory_order_relaxed);
            return job;
        }

        worker.bottom.store(t, std::memory_order_relaxed);
        return nullptr;
    }

    Job* JobSystem::Steal(int thief_id) {
        const int num_workers = static_cast<int>(workers_.size());

        for (int i = 0; i < num_workers; ++i) {
            const int victim_id = (thief_id + 1 + i) % num_workers;
            if (victim_id == thief_id) continue;

            Worker& victim = *workers_[victim_id];

            int t = victim.top.load(std::memory_order_acquire);
            const int b = victim.bottom.load(std::memory_order_acquire);

            if (t < b) {
                Job* job = victim.queue[t % kMaxJobs];

                if (victim.top.compare_exchange_strong(t, t + 1, std::memory_order_seq_cst)) {
                    return job;
                }
            }
        }
        return nullptr;
    }

}  // namespace raytracer