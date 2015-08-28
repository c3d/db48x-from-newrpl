/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

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
