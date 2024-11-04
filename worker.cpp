#include "worker.h"

Worker::Worker()
{
}

void Worker::process1()   // stove sensor
{
    float temp = readTemperature("/sys/bus/w1/devices/28-0117b34e1eff/w1_slave");
    emit emitTemp(temp);
    emit finished();
}

void Worker::process2()   // small accu
{
    float temp = readTemperature("/sys/bus/w1/devices/28-0315902022ff/w1_slave");
    emit emitTemp(temp);
    emit finished();
}

void Worker::process3()   // big accu
{
    float temp = readTemperature("/sys/bus/w1/devices/28-0315902484ff/w1_slave");
    emit emitTemp(temp);
    emit finished();
}

void Worker::process4()   // room temperature
{
    float temp = readTemperature("/sys/bus/w1/devices/28-00000883d788/w1_slave");
    emit emitTemp(temp);
    emit finished();
}

void Worker::process5()
{
    system("aplay /home/pi/testLayouts/FireAlarm.wav");    // alarm
    emit finished();
}


float Worker::readTemperature(QString fileName)
{
    float temperature = 0.1;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);

        for (int i=0; i<2; ++i)
        {
            QString line = in.readLine();
            QString trackName("t=");
            int pos = line.indexOf(trackName);
            if (pos>0)
            {
                QString theTrackName = line.mid(pos + trackName.length());
                temperature = theTrackName.toFloat() * 1e-3;
            }
        }
        file.close();
    }
    return temperature;
}









