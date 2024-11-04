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
    void emitTemp(float newTemp);
    void finished();

public slots:
    void process1();
    void process2();
    void process3();
    void process4();
    void process5();
    float readTemperature(QString fileName);
};

#endif // WORKER_H
