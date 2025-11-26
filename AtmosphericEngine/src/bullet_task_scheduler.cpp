#include "bullet_task_scheduler.hpp"
#include "job_system.hpp"
#include "pch.hpp"

void BulletTaskScheduler::parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body)
{
    const int numTasks = (iEnd - iBegin) / grainSize;
    if (numTasks == 0)
    {
        body.forLoop(iBegin, iEnd);
        return;
    }

    for (int i = 0; i < numTasks; ++i)
    {
        const int start = iBegin + i * grainSize;
        const int end = (i == numTasks - 1) ? iEnd : start + grainSize;

        m_jobSystem.Execute([&body, start, end](int) {
            body.forLoop(start, end);
        });
    }

    m_jobSystem.Wait();
}
