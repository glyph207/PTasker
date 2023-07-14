#ifndef PTASK_H
#define PTASK_H

#include <QString>

namespace PT
{

enum class Priority
{
    Normal, High
};

class Task
{
public:
    virtual void exec() = 0;
    virtual QString info() const  = 0;
    virtual bool isBusy() const = 0;
};

}

#endif // PTASK_H
