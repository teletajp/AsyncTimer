#include "Runnable.h"
#ifndef _WIN32
#include <unistd.h>
#endif
#include <thread>

namespace running
{
    class AutoThread::Impl
    {
        RunnablePtr runnable_object_;

    public:
        Impl(RunnablePtr &&runnable_object)
            : runnable_object_(std::move(runnable_object)),
              raw_object_pointer_(runnable_object_.get()),
              terminated_(false),
              core_id_(-1)
        {
            thread_ =
                std::thread(&AutoThread::Impl::run, this, raw_object_pointer_, std::ref(core_id_));
        }
        Impl(RunnablePtr &&runnable_object, int core_id)
            : runnable_object_(std::move(runnable_object)),
              raw_object_pointer_(runnable_object_.get()),
              terminated_(false),
              core_id_(core_id)
        {
            thread_ =
                std::thread(&AutoThread::Impl::run, this, raw_object_pointer_, std::ref(core_id_));
        }
        Impl(IRunnable *runnable_object)
            : runnable_object_(),
              raw_object_pointer_(runnable_object),
              terminated_(false),
              core_id_(-1)
        {
            thread_ =
                std::thread(&AutoThread::Impl::run, this, raw_object_pointer_, std::ref(core_id_));
        }
        Impl(IRunnable *runnable_object, int core_id)
            : runnable_object_(),
              raw_object_pointer_(runnable_object),
              terminated_(false),
              core_id_(core_id)
        {
            thread_ =
                std::thread(&AutoThread::Impl::run, this, raw_object_pointer_, std::ref(core_id_));
        }
        ~Impl()
        {
            terminated_.store(true);
            if (thread_.joinable())
            {
                if (thread_.get_id() == std::this_thread::get_id())
                    thread_.detach();
                else
                    thread_.join();
            }
        }

        static void run(Impl *thread, IRunnable *runnable_object, int &core_id)
        {
            try
            {
                Impl::runner(thread, runnable_object, core_id);
            }
            catch (...)
            {
            }
            thread->terminated_.store(true);
        }

        static void runner(Impl *thread, IRunnable *runnable_object, int &core_id)
        {
            if (core_id != -1 && !StickThreadToCore(core_id))
                core_id = -1;

            while (!thread->terminated_.load())
            {
                runnable_object->run(thread->terminated_);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        static bool StickThreadToCore(int core_id)
        {
            int num_cores = std::thread::hardware_concurrency();
            if (num_cores <= 0 || core_id >= num_cores)
                return false;
#ifdef __linux__
            // int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
            // if (num_cores == -1)
            //     return false;
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(core_id, &cpuset);
            auto current_thread = pthread_self();
            auto error = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
            if (error)
                return false;
            return true;
#endif
            return false;
        }

        IRunnable *raw_object_pointer_;
        std::atomic_bool terminated_;
        std::thread thread_;
        int core_id_;
    };

    AutoThread::AutoThread(RunnablePtr &&runnable_object)
        : pimpl_(std::make_unique<Impl>(std::move(runnable_object))) {}
    AutoThread::AutoThread(RunnablePtr &&runnable_object, int core_id)
        : pimpl_(std::make_unique<Impl>(std::move(runnable_object), core_id)) {}
    AutoThread::AutoThread(IRunnable *runnable_object)
        : pimpl_(std::make_unique<Impl>(runnable_object)) {}
    AutoThread::AutoThread(IRunnable *runnable_object, int core_id)
        : pimpl_(std::make_unique<Impl>(runnable_object, core_id)) {}
    AutoThread::~AutoThread() { terminate(); }
    bool AutoThread::terminated() const { return pimpl_->terminated_.load(); }
    void AutoThread::terminate() { pimpl_->terminated_.store(true); }
    int AutoThread::getCoreId() const { return pimpl_->core_id_; };
} // namespace running