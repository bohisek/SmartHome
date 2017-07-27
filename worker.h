#ifndef WORKER_H
#define WORKER_H

#include "mainwindow.h"
#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker();

signals:
    void finished1();
    void finished2();
    void finished3();
    void finished4();
    void finished5();

public slots:
    void process1();
    void process2();
    void process3();
    void process4();
    void process5();

};

#endif // WORKER_H
