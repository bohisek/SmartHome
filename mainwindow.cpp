#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGlobal>
#include <QtGui>
#include <qcustomplot.h>
#include <QMovie>
#include <wiringPi.h>
#include <QCloseEvent>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <QFile>
#include <QString>
#include <QIODevice>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QFont basicFont;
    basicFont.setPointSize(16);
    basicFont.setBold(true);

    QFont Font;
    Font.setPointSize(16);

    ui->setupUi(this);

    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(showTime()));
    timer->start(1000);

    timer2 = new QTimer(this);
    connect(timer2,SIGNAL(timeout()),this,SLOT(programLoop()));
    connect(timer2,SIGNAL(timeout()),this,SLOT(updateTemperature()));
    timer2->start(5000);

    QTimer *timer3 = new QTimer(this);   // for alarm (sT >= alarmT)
    connect(timer3,SIGNAL(timeout()),this,SLOT(alarmSound()));
    timer3->start(20000);


    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    QWidget *heating = new QWidget;
    QWidget *thermostat = new QWidget;
    QWidget *test = new QWidget;
    stackedWidget->addWidget(heating);
    stackedWidget->addWidget(thermostat);
    stackedWidget->addWidget(test);


   // HEATING page
    QGridLayout *gridLayout = new QGridLayout;
    QLabel *backgroundLabel = new QLabel(heating);
    backgroundLabel->setStyleSheet("border-image: url(:/fire_gas.png) 0 0 0 0 stretch stretch");
    gridLayout->addWidget(backgroundLabel,0,0,7,7);

    alarmLabel = new QLabel(heating);
    alarmLabel->setStyleSheet("background-color: rgba(255,0,0,0);");
    gridLayout->addWidget(alarmLabel,0,0,7,7);

    ClockDate = new QLabel(heating); ClockDate->setFont(basicFont);
    gridLayout->addWidget(ClockDate,0,2,1,3,Qt::AlignCenter);

    acBTemp = new QLabel("50.5",heating);  acBTemp->setFont(basicFont);
    gridLayout->addWidget(acBTemp,0,0,Qt::AlignBottom|Qt::AlignRight);
    acSTemp = new QLabel("49.3",heating);  acSTemp->setFont(basicFont);
    gridLayout->addWidget(acSTemp,1,2,Qt::AlignBottom|Qt::AlignRight);
    targetRoomTemp = new QLabel("20.0",heating);  targetRoomTemp->setFont(Font);  targetRoomTemp->setToolTip("target room temperature");
    gridLayout->addWidget(targetRoomTemp,5,0,Qt::AlignHCenter|Qt::AlignBottom);
    roomTemp = new QLabel("19.8",heating);  roomTemp->setFont(basicFont);         roomTemp->setToolTip("actual room temperature");
    gridLayout->addWidget(roomTemp,6,0,Qt::AlignHCenter|Qt::AlignTop);
    stoveTemp = new QLabel("65.8",heating);  stoveTemp->setFont(basicFont);       stoveTemp->setToolTip("wood stove temperature (outlet water)");
    gridLayout->addWidget(stoveTemp,4,2,Qt::AlignCenter);

    pixmapStoveSc = pixmapStove->scaled(50,85);  pixmapStoveBWSc = pixmapStoveBW->scaled(50,85);
    stoveLabel = new QLabel(heating);
    stoveLabel->setPixmap(pixmapStoveSc);
    gridLayout->addWidget(stoveLabel,5,2,2,1,Qt::AlignCenter);

    pixmapGasSc = pixmapGas->scaled(50,85);   pixmapGasBWSc = pixmapGasBW->scaled(50,85);
    gasLabel = new QLabel(heating);
    gasLabel->setPixmap(pixmapGasSc);
    gridLayout->addWidget(gasLabel,5,6,2,1,Qt::AlignCenter);

    pixmapConvSc = pixmapConv->scaled(80,40);
    convLabel = new QLabel(heating);
    convLabel->setPixmap(pixmapConvSc);
    gridLayout->addWidget(convLabel,0,5,2,2,Qt::AlignTop|Qt::AlignHCenter);

    pixmapValveSc = pixmapValve->scaled(43,38);  pixmapValveChargeSc = pixmapValveCharge->scaled(43,38);   pixmapValveDischargeSc = pixmapValveDischarge->scaled(43,38);
    valveLabel = new QLabel(heating);
    valveLabel->setPixmap(pixmapValveSc);
    gridLayout->addWidget(valveLabel,1,4,Qt::AlignBottom);

    pixmapHoldTempSc = pixmapHoldTemp->scaled(40,40);
    holdTempLabel = new QLabel(heating);
    holdTempLabel->setPixmap(pixmapHoldTempSc);
    holdTempLabel->hide();
    gridLayout->addWidget(holdTempLabel,5,1,Qt::AlignCenter);

    movieAcuPump->setScaledSize(QSize(55,55));
    QLabel *pumpAcu = new QLabel(heating);
    pumpAcu->setMovie(movieAcuPump);
    gridLayout->addWidget(pumpAcu,2,3,Qt::AlignBottom|Qt::AlignRight);
    movieAcuPump->start();
    movieAcuPump->setPaused(true);

    movieStovePump->setScaledSize(QSize(55,55));
    QLabel *pumpStove = new QLabel(heating);
    pumpStove->setMovie(movieStovePump);
    gridLayout->addWidget(pumpStove,6,3,Qt::AlignBottom);
    movieStovePump->start();
    movieStovePump->setPaused(true);

    movieGasPump->setScaledSize(QSize(55,55));
    QLabel *pumpGas = new QLabel(heating);
    pumpGas->setMovie(movieGasPump);
    gridLayout->addWidget(pumpGas,6,5,Qt::AlignBottom);
    movieGasPump->start();
    movieGasPump->setPaused(true);

    blockGas = new QCheckBox("block gas", heating);   blockGas->setToolTip("gas heating will never be used");
    gridLayout->addWidget(blockGas,3,5,1,2,Qt::AlignBottom);

    stovePriority = new QCheckBox("stove priority", heating);   stovePriority->setToolTip("gas OFF when stove ON");  stovePriority->setChecked(true);
    gridLayout->addWidget(stovePriority,3,2,1,2,Qt::AlignBottom);

    heating->setLayout(gridLayout);

    gridLayout->setMargin(0);
    gridLayout->setColumnMinimumWidth(0,85); gridLayout->setColumnStretch(0,1);
    gridLayout->setColumnMinimumWidth(1,35); gridLayout->setColumnStretch(1,1);
    gridLayout->setColumnMinimumWidth(2,45); gridLayout->setColumnStretch(2,1);
    gridLayout->setColumnMinimumWidth(3,60); gridLayout->setColumnStretch(3,1);
    gridLayout->setColumnMinimumWidth(4,20); gridLayout->setColumnStretch(4,1);
    gridLayout->setColumnMinimumWidth(5,20); gridLayout->setColumnStretch(5,1);
    gridLayout->setColumnMinimumWidth(6,20); gridLayout->setColumnStretch(6,1);


    gridLayout->setRowMinimumHeight(0,40);  gridLayout->setRowStretch(0,1);
    gridLayout->setRowMinimumHeight(1,45);  gridLayout->setRowStretch(1,1);
    gridLayout->setRowMinimumHeight(2,75);  gridLayout->setRowStretch(2,1);
    gridLayout->setRowMinimumHeight(3,10);  gridLayout->setRowStretch(3,1);
    gridLayout->setRowMinimumHeight(4,10);  gridLayout->setRowStretch(4,1);
    gridLayout->setRowMinimumHeight(5,20);  gridLayout->setRowStretch(5,1);
    gridLayout->setRowMinimumHeight(6,50);  gridLayout->setRowStretch(6,1);




    // THERMOSTAT page
    tempSpin  = new QDial; tempSpin->setMinimumSize(100,100);
    tempSpin->setMinimum(10);
    tempSpin->setMaximum(30);
    tempSpin->setValue(18);
    QLabel *tempLabel  = new QLabel("temperature"); tempNumberLabel = new QLabel();
    tempNumberLabel->setText(QString::number(tempSpin->value()));
    connect(tempSpin,SIGNAL(valueChanged(int)),this,SLOT(tempSpin_valueChanged(int)));

    clockSpin = new QDial; clockSpin->setMinimumSize(100,100);
    clockSpin->setMinimum(1);
    clockSpin->setMaximum(24);
    clockSpin->setValue(8);
    QLabel *clockLabel = new QLabel("clock"); clockNumberLabel = new QLabel;
    clockNumberLabel->setText(QString::number(clockSpin->value()) + ":00");
    connect(clockSpin,SIGNAL(valueChanged(int)),this,SLOT(clockSpin_valueChanged(int)));

    QPushButton *resetButton = new QPushButton("reset");  resetButton->setMaximumWidth(80);
    connect(resetButton,SIGNAL(clicked()),this,SLOT(reset_clicked()));
    QPushButton *setButton   = new QPushButton("set");  setButton->setMaximumWidth(80);
    connect(setButton,SIGNAL(clicked()),this,SLOT(set_clicked()));
    holdTemp = new QCheckBox("hold");
    customPlot = new QCustomPlot;
    QVBoxLayout *clockLabels = new QVBoxLayout;
    clockLabels->addWidget(clockLabel);
    clockLabels->addWidget(clockNumberLabel); clockLabels->addStretch(0);
    QVBoxLayout *tempLabels = new QVBoxLayout;
    tempLabels->addWidget(tempLabel);
    tempLabels->addWidget(tempNumberLabel);
    tempLabels->addWidget(holdTemp); tempLabels->addStretch(0);void on_actionTest_triggered();
    QGroupBox *groupBox = new QGroupBox;         groupBox->setMaximumWidth(80);
    QCheckBox *Mo = new QCheckBox("Mo",groupBox);
    QCheckBox *Tu = new QCheckBox("Tu",groupBox);
    QCheckBox *We = new QCheckBox("We",groupBox);
    QCheckBox *Th = new QCheckBox("Th",groupBox);
    QCheckBox *Fr = new QCheckBox("Fr",groupBox);
    QCheckBox *Sa = new QCheckBox("Sa",groupBox);
    QCheckBox *So = new QCheckBox("Su",groupBox);
    l_checkboxes = groupBox->findChildren<QCheckBox *>();
    QVBoxLayout *daysBox = new QVBoxLayout;
    daysBox->addWidget(Mo);
    daysBox->addWidget(Tu);
    daysBox->addWidget(We);
    daysBox->addWidget(Th);
    daysBox->addWidget(Fr);
    daysBox->addWidget(Sa);
    daysBox->addWidget(So); daysBox->setSpacing(0); daysBox->setMargin(2);
    groupBox->setLayout(daysBox);
    QHBoxLayout *customPlotBox = new QHBoxLayout;
    customPlotBox->addWidget(customPlot);
    //customPlotBox->addStretch(0);
    QVBoxLayout *buttons = new QVBoxLayout;
    buttons->addWidget(resetButton);
    buttons->addWidget(setButton); buttons->setSpacing(0); buttons->setMargin(2);
    QHBoxLayout *spinBoxes = new QHBoxLayout;
    spinBoxes->addLayout(clockLabels);
    spinBoxes->addWidget(clockSpin);
    spinBoxes->addLayout(tempLabels);
    spinBoxes->addWidget(tempSpin);
    //spinBoxes->addStretch(0);
    QHBoxLayout *spinsButtons = new QHBoxLayout;
    spinsButtons->addLayout(spinBoxes);
    spinsButtons->addLayout(buttons);void on_actionTest_triggered();
    QHBoxLayout *plotDays = new QHBoxLayout;
    plotDays->addLayout(customPlotBox);
    plotDays->addWidget(groupBox);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(plotDays);
    mainLayout->addLayout(spinsButtons);

    thermostat->setLayout(mainLayout);

    Colors = {"red","magenta","cyan","green","yellow","blue","black"};
    int T = 20;
    int n = 24;
    clockVector.fill(T,n);
    std::iota(clockVector.begin(),clockVector.end(),1);  // fill vector with sequentially increasing values starting from 1

    for (int i=0;i<7;i++)
    {
        QVector<double> data;
        data.fill(T,n);
        days.push_back(data);
        graphs.push_back(customPlot->addGraph());
        QVector<QVector<int>> day;
        daysShort.push_back(day);
    }

    // TEST RELAYS page
    R1 = new QRadioButton("first (from gas heating)" ,test); R1->setToolTip("activate RELAY 1"); connect(R1,SIGNAL(toggled(bool)),this,SLOT(relay1()));
    R2 = new QRadioButton("second (acu charging)",test); R2->setToolTip("activate RELAY 2"); connect(R2,SIGNAL(toggled(bool)),this,SLOT(relay2()));
    R3 = new QRadioButton("third (from acu heating)" ,test); R3->setToolTip("activate RELAY 3"); connect(R3,SIGNAL(toggled(bool)),this,SLOT(relay3()));
    R4 = new QRadioButton("fourth (not in use)",test); R4->setToolTip("activate RELAY 4"); connect(R4,SIGNAL(toggled(bool)),this,SLOT(relay4()));

    dT1 = new QLineEdit("1",test);              dT1->setMaximumSize(50,30);
    QLabel *dT1name = new QLabel("dT<sub>1</sub>", test);  dT1name->setMinimumWidth(60);
    QLabel *dT1unit = new QLabel(QString::fromUtf8("°C"), test);  dT1name->setToolTip("thermostat temperature difference for ON/OFF the heating.");
    QHBoxLayout *dT1Layout = new QHBoxLayout;
    dT1Layout->addWidget(dT1name);
    dT1Layout->addWidget(dT1);
    dT1Layout->addWidget(dT1unit);  dT1Layout->addStretch(1);

    dT2 = new QLineEdit("5",test);              dT2->setMaximumSize(50,30);
    QLabel *dT2name = new QLabel("dT<sub>2</sub>", test);  dT2name->setMinimumWidth(60);
    QLabel *dT2unit = new QLabel(QString::fromUtf8("°C"), test);  dT2name->setToolTip("temperature difference for charging/discharging the acus.");
    QHBoxLayout *dT2Layout = new QHBoxLayout;
    dT2Layout->addWidget(dT2name);
    dT2Layout->addWidget(dT2);
    dT2Layout->addWidget(dT2unit);  dT2Layout->addStretch(1);

    minTAcu = new QLineEdit("55",test);                  minTAcu->setMaximumSize(50,30);
    QLabel *minTAcuname = new QLabel("minT<sub>acu</sub>", test);   minTAcuname->setMinimumWidth(60);
    QLabel *minTAcuunit = new QLabel(QString::fromUtf8("°C"), test);  minTAcuname->setToolTip("min temperature to allow discharging the acus.");
    QHBoxLayout *minTAcuLayout = new QHBoxLayout;
    minTAcuLayout->addWidget(minTAcuname);
    minTAcuLayout->addWidget(minTAcu);
    minTAcuLayout->addWidget(minTAcuunit);   minTAcuLayout->addStretch(1);

    maxTAcu = new QLineEdit("80",test);                  maxTAcu->setMaximumSize(50,30);
    QLabel *maxTAcuname = new QLabel("maxT<sub>acu</sub>", test);    maxTAcuname->setMinimumWidth(60);
    QLabel *maxTAcuunit = new QLabel(QString::fromUtf8("°C"), test);  maxTAcuname->setToolTip("max temperature after charging.");
    QHBoxLayout *maxTAcuLayout = new QHBoxLayout;
    maxTAcuLayout->addWidget(maxTAcuname);
    maxTAcuLayout->addWidget(maxTAcu);
    maxTAcuLayout->addWidget(maxTAcuunit);    maxTAcuLayout->addStretch(1);

    alarmT = new QLineEdit("80",test);                  alarmT->setMaximumSize(50,30);
    QLabel *alarmTname = new QLabel("T<sub>alarm</sub>", test);    alarmTname->setMinimumWidth(60);
    QLabel *alarmTunit = new QLabel(QString::fromUtf8("°C"), test);  alarmTname->setToolTip("alarm triggered at this temperature");
    QHBoxLayout *alarmTLayout = new QHBoxLayout;
    alarmTLayout->addWidget(alarmTname);
    alarmTLayout->addWidget(alarmT);
    alarmTLayout->addWidget(alarmTunit);    alarmTLayout->addStretch(1);

    QVBoxLayout *settings = new QVBoxLayout;
    settings->addLayout(dT1Layout);
    settings->addLayout(dT2Layout);
    settings->addLayout(minTAcuLayout);
    settings->addLayout(maxTAcuLayout);
    settings->addLayout(alarmTLayout); settings->addSpacing(20);
    settings->addWidget(R1);
    settings->addWidget(R2);
    settings->addWidget(R3);
    settings->addWidget(R4); settings->addStretch(1);

    test->setLayout(settings);

    readThermostat();
    set_clicked();   // comprises writeThermostat();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionHeating_triggered()
{
    if (stackedWidget->currentIndex()==2)
    {
        timer2->start(1500);
        relaysOff();
    }
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_actionThermostat_triggered()
{
    if (stackedWidget->currentIndex()==2)
    {
        timer2->start(1500);
        relaysOff();
    }
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_actionTest_triggered()
{
    QMessageBox::StandardButton resBtn = QMessageBox::warning(this,"Testing relays","Make sure the wood stove NOT in use!",QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (resBtn == QMessageBox::Ok)
    {
        timer2->stop();
        relaysOff();
        stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::tempSpin_valueChanged(int value)
{
    tempNumberLabel->setText(QString::number(value));
}

void MainWindow::clockSpin_valueChanged(int value)
{
    clockNumberLabel->setText(QString::number(value) + ":00");
}

void MainWindow::set_clicked()
{
    int i=0;
    foreach(QCheckBox *checkBox, l_checkboxes)
    {
        if (checkBox->isChecked())
        {
            readData(daysShort[i]);
        }
        updateData(daysShort[i],days[i]);
        i++;
    }
    QVector<int> idColor = updateColorVector();
    plotCurves(idColor);

    // save thermostat/parameters into file
    writeThermostat();

}

void MainWindow::readData(QVector<QVector<int> > &day)
{
    QVector<int> data;
    data << clockSpin->value()-1 << tempSpin->value();
    day.push_back(data);
    qSort(day.begin(),day.end(),forSort);
}

bool MainWindow::forSort(QVector<int> &a, QVector<int> &b)
{
    return a[0] < b[0];
}




void MainWindow::updateData(QVector<QVector<int> > &day, QVector<double> &dayFull)
{
    for (int i=0; i<day.size(); i++)
    {
        if (i<day.size()-1)
        {
            for (int j=day[i][0]; j<day[i+1][0]; j++)
            {
                dayFull[j] = day[i][1];
            }
        }
        else
        {
            for (int j=0; j<day[0][0]; j++)
            {
                dayFull[j] = day[i][1];
            }
            for (int j=day[i][0]; j<dayFull.size(); j++)
            {
                dayFull[j] = day[i][1];
            }
        }
    }
}

QVector<int> MainWindow::updateColorVector()
{
    QVector<int> idColor(7); idColor.fill(0); int idColorMax = 0;
    for (int i=1;i<7;i++)
    {
        bool p = false;
        for(int j=0;j<i;j++)
        {
            if (days[i]==days[j])
            {
                idColor[i] = idColor[j];
                p = true;
            }
        }
        if (!p)
        {
            idColorMax += 1;
            idColor[i] = idColorMax;
        }
    }
    return idColor;
}

void MainWindow::plotCurves(const QVector<int> &idColor)
{
    for (int i=0;i<7;i++)
    {
        graphs[i]->setData(clockVector, days[i]);
        QPen pen;
        pen.setWidth(3);
        pen.setColor(QColor(Colors[idColor[i]]));
        graphs[i]->setPen(pen);
    }
    customPlot->rescaleAxes();
    customPlot->replot();

    int i=0;
    foreach(QCheckBox *checkBox, l_checkboxes)
    {
        checkBox->setStyleSheet("QCheckBox{background-color :"+Colors[idColor[i]]+"}");
        i++;
    }
}

void MainWindow::reset_clicked()
{
    int i=0;
    foreach(QCheckBox *checkBox, l_checkboxes)
    {
        if (checkBox->isChecked())
        {
            daysShort[i].clear();
            days[i].fill(20);
            updateData(daysShort[i],days[i]);
        }
        i++;
    }
    QVector<int> idColor = updateColorVector();
    plotCurves(idColor);
}


void MainWindow::showTime()
{
    QDateTime date = QDateTime::currentDateTime();
    QString dateText = date.toString("dd MMM   ");

    QTime time = QTime::currentTime();
    QString timeText = time.toString("hh:mm");

    checkAlarm();

    if (counter%2)
    {
        timeText = time.toString("hh mm");
    }
    counter++;

    ClockDate->setText(dateText + timeText);

    // read and set targetRoomTemp from thermostat
    QStringList day;
    day << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";  // list of strings to get day indices 0-6 ENGLISH
    setTargetRoomTemp(time.toString("hh"),time.toString("mm"),days[day.indexOf(date.toString("ddd"))]);
    setLcdBrightness(date);

}


void MainWindow::setLcdBrightness(QDateTime date)
{
    boost::gregorian::date d(date.toString("yyyy").toInt(),date.toString("M").toInt(),date.toString("d").toInt());

    float minLcdBright = 15;
    float maxLcdBright = 255;
    const float PI = 3.14159265;
    float A =  1.974*cos(2*PI/365*(d.day_of_year()+3.677)) +  5.897;   // hour of sunrise approximated
    float B = -1.998*cos(2*PI/365*(d.day_of_year()+12.72)) + 18.14;    // hour of sunset approximated
    A += ((date.toString("M").toInt()>3) & (date.toString("M").toInt()<11));  // summer time (starts on last Sunday in March, ends on last Sundayin October)
    B += ((date.toString("M").toInt()>3) & (date.toString("M").toInt()<11));
    float period = 2*PI/(B-A);
    float shiftX = 0.5*(A+B);
    float shiftY = 0.5*(minLcdBright+maxLcdBright);
    float ampltd = 0.5*(maxLcdBright-minLcdBright);
    float h = date.toString("hh").toFloat() + date.toString("mm").toFloat()/60;

    int bright = ampltd*cos(period*(h-shiftX)) + shiftY;
    bright = ((h>A)&(h<B)) * bright + ((h<=A)|(h>=B)) * minLcdBright;

    QString brightStr = "echo " + QString::number(bright) + " > /sys/class/backlight/rpi_backlight/brightness";
    system(brightStr.toLatin1().data());   // necessary to convert QString to *char
}

void MainWindow::setTargetRoomTemp(QString hour, const QString minute, const QVector<double> &dayFull)
{
    if (holdTemp->isChecked())  // targetRoomTemp read directly from TempSpin
    {
        targetRoomTemp->setText(QString::number(tempSpin->value(),'f',1));
        holdTempLabel->show();
    }
    else  // else targetRoomTemp read every hour from thermostat
    {
        holdTempLabel->hide();
        //if (minute == "00")
        //{
            if (hour == "00"){hour="24";}
            double hourLeft  = dayFull[hour.toInt()-1];
            if (hour == "24"){hour="00";}
            double hourRight = dayFull[hour.toInt()];
            double temp = hourLeft * (1-minute.toInt()/60.) + hourRight * minute.toInt()/60.;
            targetRoomTemp->setText(QString::number(temp,'f',1));
            //targetRoomTemp->setText(QString::number(dayFull[hour.toInt()-1]));
        //}
    }
}

void MainWindow::programLoop()
{
        float delta       = dT1->text().toFloat();          // temperature difference
        float deltaAcu    = dT2->text().toFloat();
        float minAcSTemp  = minTAcu->text().toFloat();
        float maxAcSTemp  = maxTAcu->text().toFloat();
        float tRT         = targetRoomTemp->text().toFloat();
        alarmTemp         = alarmT->text().toFloat();

        stoveON = digitalRead(16);           // from gas pump relay

        readTemperature("/home/pi/testLayouts/w1_slave_1", &sT);
        readTemperature("/home/pi/testLayouts/w1_slave_2", &aST);
        readTemperature("/home/pi/testLayouts/w1_slave_3", &aBT);
        readTemperature("/home/pi/testLayouts/w1_slave_4", &rT);

        stoveTemp->setText(QString::number(sT,'f',1));
        acSTemp->setText(QString::number(aST,'f',1));
        acBTemp->setText(QString::number(aBT,'f',1));
        roomTemp->setText(QString::number(rT,'f',1));

        if (rT>tRT)         // heating OFF
        {
            movieAcuPump->setPaused(true);
            movieGasPump->setPaused(true);
            gasLabel->setPixmap(pixmapGasBWSc);
            digitalWrite(6,0); // gas heating OFF

            if (stoveON && (sT>aST+deltaAcu) && (aST<maxAcSTemp))
            {
                digitalWrite(13,1);  // acu charging ON
            }
            else if (!stoveON || (sT<aST+deltaAcu-delta) || (aST>maxAcSTemp+delta))
            {
                digitalWrite(13,0);  // acu charging OFF (to radiators)
            }

            if (digitalRead(13))     // acu charging ON
            {
                convLabel->hide();
                valveLabel->setPixmap(pixmapValveChargeSc);
                movieStovePump->setPaused(false);
                stoveLabel->setPixmap(pixmapStoveSc);
            }
            else    // acu charging OFF
            {
                valveLabel->setPixmap(pixmapValveSc);
                if (stoveON)
                {
                    convLabel->show();
                    movieStovePump->setPaused(false);
                    stoveLabel->setPixmap(pixmapStoveSc);
                }
                else
                {
                    convLabel->hide();
                    movieStovePump->setPaused(true);
                    stoveLabel->setPixmap(pixmapStoveBWSc);
              }
            }
        }
        else if (rT<tRT-delta)   // heating ON
        {

            if ((aST>minAcSTemp) && !stoveON)
            {
                digitalWrite(19,1);   // from acu heating ON
            }
            else if ((aST<minAcSTemp-delta) || stoveON)
            {
                digitalWrite(19,0);   // from acu heating OFF
            }

            if (digitalRead(19))  // from acu heating ON
            {
                    convLabel->show();

                    movieAcuPump->setPaused(false);
                    valveLabel->setPixmap(pixmapValveDischargeSc);

                    digitalWrite(6,0); // from gas heating OFF
                    movieGasPump->setPaused(true);
                    gasLabel->setPixmap(pixmapGasBWSc);

                    movieStovePump->setPaused(true);
                    stoveLabel->setPixmap(pixmapStoveBWSc);
            }
            else
            {
                    convLabel->hide();

                    movieAcuPump->setPaused(true);
                    valveLabel->setPixmap(pixmapValveSc);

                    if ( !blockGas->isChecked() && (!stoveON || !stovePriority->isChecked()) )
                    {
                        convLabel->show();

                        digitalWrite(6,1); // from gas heating ON
                        movieGasPump->setPaused(false);
                        gasLabel->setPixmap(pixmapGasSc);
                    }
                    else
                    {
                        digitalWrite(6,0);  // from gas heating OFF
                        movieGasPump->setPaused(true);
                        gasLabel->setPixmap(pixmapGasBWSc);
                    }

                    if (stoveON)
                    {
                        convLabel->show();

                        movieStovePump->setPaused(false);
                        stoveLabel->setPixmap(pixmapStoveSc);
                    }
                    else
                    {
                        movieStovePump->setPaused(true);
                        stoveLabel->setPixmap(pixmapStoveBWSc);
                    }
            }
        }

}

void MainWindow::writeThermostat()
{
    qDebug() << "writing thermostat...";
    QString filename = "thermParams";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream out(&file);

        out << dT1->text() << endl
            << dT2->text() << endl
            << minTAcu->text() << endl
            << maxTAcu->text() << endl
            << alarmT->text() << endl
            << holdTemp->isChecked() << endl
            << stovePriority->isChecked() << endl
            << blockGas->isChecked() << endl
            << tempSpin->value() << endl;

        for (int j=0; j<7; j++)
        {
            out << daysShort[j].size() << endl;
        }

        for (int j=0; j<7; j++)
        {
            if (daysShort[j].size()>0)
            {
                for (int i=0; i<daysShort[j].size(); i++)
                {
                    out << daysShort[j][i][0] << " " << daysShort[j][i][1] << endl;
                }
            }
        }
        file.close();
    }
}

void MainWindow::readThermostat()
{
    qDebug() << "reading thermostat...";
    QString filename = "thermParams";
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        dT1->setText(in.readLine());
        dT2->setText(in.readLine());
        minTAcu->setText(in.readLine());
        maxTAcu->setText(in.readLine());
        alarmT->setText(in.readLine());
        holdTemp->setChecked(in.readLine().toInt());
        stovePriority->setChecked(in.readLine().toInt());
        blockGas->setChecked(in.readLine().toInt());
        tempSpin->setValue(in.readLine().toInt());

        QVector<int> dayLength;

        for (int i=0; i<7; i++)
        {
            dayLength.push_back(in.readLine().toInt());
        }

        for (int i=0; i<7; i++)
        {
            for (int j=0; j<dayLength[i]; j++)
            {
                QStringList textList = in.readLine().split(" ");
                QVector<int> data;
                data << textList.at(0).toInt() << textList.at(1).toInt();
                daysShort[i].push_back(data);
            }
        }
    }
    file.close();
}

void MainWindow::readTemperature(QString fileName, float *temperature)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);

        for (int i=0; i<2; ++i)
        {
            QString line = in.readLine();
            QString trackName("t=");
            int pos = line.indexOf(trackName);
            //qDebug() << "position is " << pos;
            if (pos>0)
            {
                QString theTrackName = line.mid(pos + trackName.length());
                //qDebug() << theTrackName;
                *temperature = theTrackName.toFloat() * 1e-3;
                //if ((theTrackName.toFloat() * 1e-3 > 30) || (theTrackName.toFloat() * 1e-3 < 20))
                if (theTrackName.toFloat() * 1e-3 < 18)
                {
                    qDebug() << theTrackName << " ,the number: " << theTrackName.toFloat() * 1e-3;
                }
            }
        }
        file.close();
    }
}

void MainWindow::updateTemperature()
{
    QThread* thread1 = new QThread;
    Worker* worker1 = new Worker();
    worker1->moveToThread(thread1);
    connect(thread1,SIGNAL(started()),worker1,SLOT(process1()));
    connect(worker1,SIGNAL(emitTemp(float)),this,SLOT(receiveTemp1(float)));
    connect(worker1,SIGNAL(finished()),thread1,SLOT(quit()));
    connect(worker1,SIGNAL(finished()),worker1,SLOT(deleteLater()));
    connect(thread1,SIGNAL(finished()),thread1,SLOT(deleteLater()));
    thread1->start();

    QThread* thread2 = new QThread;
    Worker* worker2 = new Worker();
    worker2->moveToThread(thread2);
    connect(thread2,SIGNAL(started()),worker2,SLOT(process2()));
    connect(worker2,SIGNAL(emitTemp(float)),this,SLOT(receiveTemp2(float)));
    connect(worker2,SIGNAL(finished()),thread2,SLOT(quit()));
    connect(worker2,SIGNAL(finished()),worker2,SLOT(deleteLater()));
    connect(thread2,SIGNAL(finished()),thread2,SLOT(deleteLater()));
    thread2->start();

    QThread* thread3 = new QThread;
    Worker* worker3 = new Worker();
    worker3->moveToThread(thread3);
    connect(thread3,SIGNAL(started()),worker3,SLOT(process3()));
    connect(worker3,SIGNAL(emitTemp(float)),this,SLOT(receiveTemp3(float)));
    connect(worker3,SIGNAL(finished()),thread3,SLOT(quit()));
    connect(worker3,SIGNAL(finished()),worker3,SLOT(deleteLater()));
    connect(thread3,SIGNAL(finished()),thread3,SLOT(deleteLater()));
    thread3->start();

    QThread* thread4 = new QThread;
    Worker* worker4 = new Worker();
    worker4->moveToThread(thread4);
    connect(thread4,SIGNAL(started()),worker4,SLOT(process4()));
    connect(worker4,SIGNAL(emitTemp(float)),this,SLOT(receiveTemp4(float)));
    connect(worker4,SIGNAL(finished()),thread4,SLOT(quit()));
    connect(worker4,SIGNAL(finished()),worker4,SLOT(deleteLater()));
    connect(thread4,SIGNAL(finished()),thread4,SLOT(deleteLater()));
    thread4->start();
}
void MainWindow::receiveTemp1(float newTemp)
{
    sT = newTemp;
}

void MainWindow::receiveTemp2(float newTemp)
{
    aST = newTemp;
}

void MainWindow::receiveTemp3(float newTemp)
{
    aBT = newTemp;
}

void MainWindow::receiveTemp4(float newTemp)
{
    rT = newTemp;
}

void MainWindow::checkAlarm()
{
    if ((counter%2) && (sT>alarmTemp))
    {
        alarmLabel->setStyleSheet("background-color: rgba(255,0,0,150);");
    }
    else
    {
        alarmLabel->setStyleSheet("background-color: rgba(255,0,0,0);");
    }
}

void MainWindow::alarmSound()
{
    if (sT>=alarmTemp)
    {
        QThread* thread1 = new QThread;
        Worker* worker1 = new Worker();
        worker1->moveToThread(thread1);
        connect(thread1,SIGNAL(started()),worker1,SLOT(process5()));
        connect(worker1,SIGNAL(finished()),thread1,SLOT(quit()));
        connect(worker1,SIGNAL(finished()),worker1,SLOT(deleteLater()));
        connect(thread1,SIGNAL(finished()),thread1,SLOT(deleteLater()));
        thread1->start();
    }
}

void MainWindow::relay1()
{
    if (R1->isChecked())   digitalWrite(6,1);   else digitalWrite(6,0);
}

void MainWindow::relay2()
{
    if (R2->isChecked())   digitalWrite(13,1);   else digitalWrite(13,0);
}

void MainWindow::relay3()
{
    if (R3->isChecked())   digitalWrite(19,1);   else digitalWrite(19,0);
}

void MainWindow::relay4()
{
    if (R4->isChecked())   digitalWrite(26,1);   else digitalWrite(26,0);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(this,"SMARTHOME", tr("Are you sure?\n"), QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

    if (resBtn != QMessageBox::Yes)
    {
        event->ignore();
    }
    else
    {
        relaysOff();
        system("gpio unexportall");
        event->accept();
    }
}

void MainWindow::relaysOff()
{
    digitalWrite(6,0);
    digitalWrite(13,0);
    digitalWrite(19,0);
    digitalWrite(26,0);
}













