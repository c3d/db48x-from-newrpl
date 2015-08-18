#include "rplthread.h"

RPLThread::RPLThread(QObject *parent)
    : QThread(parent)
{
}

RPLThread::~RPLThread()
{
}

extern "C" void startup();

void RPLThread::run()
{
    startup();
}


extern "C" void thread_yield()
{
    QThread::msleep(1);
}
