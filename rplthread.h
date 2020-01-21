/*
 * Copyright (c) 2014-2015, Claudio Lapilli and the newRPL Team
 * All rights reserved.
 * This file is released under the 3-clause BSD license.
 * See the file LICENSE.txt that shipped with this distribution.
 */

#ifndef RPLTHREAD_H
#define RPLTHREAD_H

#include <QThread>

class RPLThread:public QThread
{
  public:
    void run();

        RPLThread(QObject * parent);
       ~RPLThread();
};

#endif // RPLTHREAD_H
