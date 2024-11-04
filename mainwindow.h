#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGlobal>
#include <QtGui>
#include <qcustomplot.h>
#include <QVector>
#include <QMovie>
#include "worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QVector<QVector<QVector<int>>> daysShort;
    QVector<QVector<double>> days;
    QVector<double> clockVector;
    QVector<QString> Colors;
    QVector<QCPGraph*> graphs;
    QList<QCheckBox *> l_checkboxes;
    QCheckBox *holdTemp;



    QMovie *movieAcuPump = new QMovie(":/pump_rotated.gif");
    QMovie *movieGasPump = new QMovie(":/pump.gif");
    QMovie *movieStovePump = new QMovie(":/pump_rotated.gif");

    QPixmap *pixmapStove = new QPixmap(":/stove.png");
    QPixmap *pixmapStoveBW = new QPixmap(":/stoveBW.png");
    QPixmap pixmapStoveSc;
    QPixmap pixmapStoveBWSc;
    QPixmap *pixmapGas = new QPixmap(":/gas.png");
    QPixmap *pixmapGasBW = new QPixmap(":/gasBW.png");
    QPixmap pixmapGasSc;
    QPixmap pixmapGasBWSc;
    QPixmap *pixmapValve = new QPixmap(":/three-way_valve_classic.png");
    QPixmap pixmapValveSc;
    QPixmap *pixmapValveCharge = new QPixmap(":/three-way_valve_charge.png");
    QPixmap pixmapValveChargeSc;
    QPixmap *pixmapValveDischarge = new QPixmap(":/three-way_valve_discharge.png");
    QPixmap pixmapValveDischargeSc;
    QPixmap *pixmapConv = new QPixmap(":/convection.png");
    QPixmap pixmapConvSc;
    QPixmap *pixmapHoldTemp = new QPixmap(":/hold_icon.png");
    QPixmap pixmapHoldTempSc;
    QPixmap *pixmapFan = new QPixmap(":/fanV3.png");
    QPixmap pixmapFanSc;

    QLabel *targetRoomTemp;
    QLabel *roomTemp;
    QLabel *stoveTemp;
    QLabel *acBTemp;
    QLabel *acSTemp;
    QLabel *ClockDate;
    QLabel *alarmLabel;
    QLabel *gasLabel, *stoveLabel, *convLabel, *valveLabel;
    QLabel *holdTempLabel, *hotWaterLabel;
    QLabel *fanLabel;
    QCheckBox *blockGas, *stovePriority, *hotWater, *fan;

    bool stoveON, gasON, gasONcharge, charge, discharge;
    float rT = 0, aST = 0, aBT = 0, sT = 0, alarmTemp;
    QRadioButton *R1, *R2, *R3, *R4;
    QLineEdit *dT1, *dT2, *minTAcu, *maxTAcu, *alarmT, *fanOnT;


    int counter = 0;


private slots:
    void on_actionHeating_triggered();

    void on_actionThermostat_triggered();

    void on_actionTest_triggered();

    void tempSpin_valueChanged(int value);
    void clockSpin_valueChanged(int value);

    void set_clicked();

    void readData(QVector<QVector<int> > &day);

    static bool forSort(QVector<int> &a, QVector<int> &b);

    void updateData(QVector<QVector<int> > &day, QVector<double> &dayFull);

    QVector<int> updateColorVector();

    void plotCurves(const QVector<int> &idColor);

    void reset_clicked();

    void showTime();

    void setTargetRoomTemp(QString hour, const QString minute, const QVector<double> &dayFull);

    void setLcdBrightness(QDateTime date);

    void programLoop();

    void writeThermostat();
    void readThermostat();

    void updateTemperature();

    void checkAlarm();
    void alarmSound();

    void relay1();
    void relay2();
    void relay3();
    void relay4();

    void closeEvent(QCloseEvent *event);

    void relaysOff();

    void receiveTemp1(float newTemp);
    void receiveTemp2(float newTemp);
    void receiveTemp3(float newTemp);
    void receiveTemp4(float newTemp);


private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    QLabel *clockNumberLabel;
    QLabel *tempNumberLabel;
    QDial *clockSpin;
    QDial *tempSpin;
    QCustomPlot *customPlot;
    QTimer *timer2;
};

#endif // MAINWINDOW_H
