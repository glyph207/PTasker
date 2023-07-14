#ifndef PTASKER_H
#define PTASKER_H

#include <queue>
#include <variant>
#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>

#include <QStringList>

#include "ptask.h"

namespace PT
{

template<typename TaskPtrVar>
class Tasker // Primitive Tasker
{
public:
    ~Tasker();

    void addTask(TaskPtrVar task, const Priority priority = Priority::Normal);
    void start(const int timeOut);
    void stop();
    QStringList info() const;

private:
    std::deque<TaskPtrVar> tasks;
    void processTasks();
    void wait();
    std::chrono::milliseconds timeOut;
    bool thatsEnough = false;
};

template<typename TaskPtrVar>
void Tasker<TaskPtrVar>::addTask(TaskPtrVar task, const Priority priority)
{
    switch (priority) {
    case Priority::Normal:
        tasks.push_back(std::move(task));
        break;
    case Priority::High:
        tasks.insert(std::find_if_not(std::begin(tasks), std::end(tasks),[](const TaskPtrVar &tpv)
                                      {
                                          bool isBusy = false;
                                          std::visit([&isBusy](auto &&arg)
                                                     {
                                                         isBusy = arg->isBusy();
                                                     }, tpv);
                                          return isBusy;
                                      }), std::move(task));
        break;
    }
}

template<typename TaskPtrVar>
void Tasker<TaskPtrVar>::start(const int timeOut)
{
    this->timeOut = std::chrono::milliseconds(timeOut);
    thatsEnough = false;
    processTasks();
}

template<typename TaskPtrVar>
void Tasker<TaskPtrVar>::stop()
{
    thatsEnough = true;
    tasks.clear();
}

template<typename TaskPtrVar>
QStringList Tasker<TaskPtrVar>::info() const
{
    QStringList report;
    for (const auto &t: tasks) {
        std::visit([&report](auto &&arg)
                   {
                       report += arg->info();
                   }, t);
    }
    return report;
}

template<typename TaskPtrVar>
Tasker<TaskPtrVar>::~Tasker() { thatsEnough = true; }

template<typename TaskPtrVar>
void Tasker<TaskPtrVar>::processTasks()
{
    if (!tasks.empty()) {
        bool del = false;
        std::visit([&del](auto &&arg)
                   {
                       arg->exec();
                       del = !arg->isBusy();
                   }, tasks.front());
        if (del) {
            tasks.pop_front();
        }
    }
    if (!thatsEnough) {
        std::thread t(&::PT::Tasker<TaskPtrVar>::wait, this);
        t.detach();
    }
}

template<typename TaskPtrVar>
void Tasker<TaskPtrVar>::wait()
{
    std::this_thread::sleep_for(timeOut);
    processTasks();
}

} // PT

#endif // PTASKER_H
