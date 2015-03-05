#ifndef RPLTHREAD_H
#define RPLTHREAD_H

#include <QThread>

class RPLThread : public QThread
{
public:
    void run();

    RPLThread(QObject *parent);
    ~RPLThread();
};

#endif // RPLTHREAD_H
