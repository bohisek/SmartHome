#include "worker.h"

Worker::Worker()
{
}

void Worker::process1()
{
    if (QFileInfo("/home/pi/testLayouts/w1_slave_1").exists())
    {
        QFile("/home/pi/testLayouts/w1_slave_1").remove();
    }
    system("cp /sys/bus/w1/devices/28-000006db7b69/w1_slave /home/pi/testLayouts/w1_slave_1");       // WITHOUT tape
    emit finished1(); // stove sensor
}

void Worker::process2()
{
    if (QFileInfo("/home/pi/testLayouts/w1_slave_2").exists())
    {
        QFile("/home/pi/testLayouts/w1_slave_2").remove();
    }
    system("cp /sys/bus/w1/devices/28-0315902022ff/w1_slave /home/pi/testLayouts/w1_slave_2");       // WITHOUT tape
    emit finished2();  // small accu
}

void Worker::process3()
{
    if (QFileInfo("/home/pi/testLayouts/w1_slave_3").exists())
    {
        QFile("/home/pi/testLayouts/w1_slave_3").remove();
    }
    system("cp /sys/bus/w1/devices/28-0315902484ff/w1_slave /home/pi/testLayouts/w1_slave_3");       // WITH tape
    emit finished3();   // big accu
}

void Worker::process4()
{
    if (QFileInfo("/home/pi/testLayouts/w1_slave_4").exists())
    {
        QFile("/home/pi/testLayouts/w1_slave_4").remove();
    }
    system("cp /sys/bus/w1/devices/28-00000883d788/w1_slave /home/pi/testLayouts/w1_slave_4");       // WITH tape
    emit finished4();   // room temperature
}

void Worker::process5()
{
    system("aplay /home/pi/testLayouts/FireAlarm.wav");    // alarm
    emit finished5();
}
