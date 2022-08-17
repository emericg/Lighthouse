/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 2022 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2020
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef DEVICE_BEACON_H
#define DEVICE_BEACON_H
/* ************************************************************************** */

#include "device.h"

#include <QObject>
#include <QString>
#include <QVariant>

#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

/* ************************************************************************** */

class ButtonParameters: public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ getId CONSTANT)

    Q_PROPERTY(int singleAction READ getSingleAction WRITE setSingleAction NOTIFY actionUpdated)
    Q_PROPERTY(QString singleParameters READ getSingleParameters WRITE setSingleParameters NOTIFY actionUpdated)

    Q_PROPERTY(int doubleAction READ getDoubleAction WRITE setDoubleAction NOTIFY actionUpdated)
    Q_PROPERTY(QString doubleParameters READ getDoubleParameters WRITE setDoubleParameters NOTIFY actionUpdated)

    Q_PROPERTY(int longAction READ getLongAction WRITE setLongAction NOTIFY actionUpdated)
    Q_PROPERTY(QString longParameters READ getLongParameters WRITE setLongParameters NOTIFY actionUpdated)

    int m_id = 0;

    int m_single_action = 0;
    QString m_single_parameters;

    int m_double_action = 0;
    QString m_double_parameters;

    int m_long_action = 0;
    QString m_long_parameters;

    void setSingleAction(int a) { m_single_action = a; setSqlButton(); Q_EMIT actionUpdated(); }
    void setSingleParameters(const QString &p) { m_single_parameters = p; setSqlButton(); Q_EMIT actionUpdated(); }

    void setDoubleAction(int a) { m_double_action = a; setSqlButton(); Q_EMIT actionUpdated(); }
    void setDoubleParameters(const QString &p) { m_double_parameters = p; setSqlButton(); Q_EMIT actionUpdated(); }

    void setLongAction(int a) { m_long_action = a; setSqlButton(); Q_EMIT actionUpdated(); }
    void setLongParameters(const QString &p) { m_long_parameters = p; setSqlButton(); Q_EMIT actionUpdated(); }

    bool setSqlButton();

Q_SIGNALS:
    void actionUpdated();

public:
    ButtonParameters(int id,
                     int single_action, const QString &single_param,
                     int double_action, const QString &double_param,
                     int long_action, const QString &long_param,
                     QObject *parent = nullptr) : QObject(parent)
    {
        m_id = id;
        m_single_action = single_action;
        m_single_parameters = single_param;
        m_double_action = double_action;
        m_double_parameters = double_param;
        m_long_action = long_action;
        m_long_parameters = long_param;
    }
    virtual ~ButtonParameters() {}

    int getId() const { return m_id; }

    int getSingleAction() const { return m_single_action; }
    QString getSingleParameters() const { return m_single_parameters; }

    int getDoubleAction() const { return m_double_action; }
    QString getDoubleParameters() const { return m_double_parameters; }

    int getLongAction() const { return m_long_action; }
    QString getLongParameters() const { return m_long_parameters; }
};

/* ************************************************************************** */

/*!
 * \brief The DeviceBeacon class
 */
class DeviceBeacon: public Device
{
    Q_OBJECT

    Q_PROPERTY(int buttonCount READ getButtonCount CONSTANT)

    Q_PROPERTY(bool hasPress READ hasPress CONSTANT)
    Q_PROPERTY(bool hasDoublePress READ hasDoublePress CONSTANT)
    Q_PROPERTY(bool hasLongPress READ hasLongPress CONSTANT)

    Q_PROPERTY(QVariant btn1 READ getBtn1 CONSTANT)
    Q_PROPERTY(QVariant btn2 READ getBtn2 CONSTANT)
    Q_PROPERTY(QVariant btn3 READ getBtn3 CONSTANT)
    Q_PROPERTY(QVariant btn4 READ getBtn4 CONSTANT)
    Q_PROPERTY(QVariant btn5 READ getBtn5 CONSTANT)
    Q_PROPERTY(QVariant btn6 READ getBtn6 CONSTANT)

    ButtonParameters *btn1 = nullptr;
    ButtonParameters *btn2 = nullptr;
    ButtonParameters *btn3 = nullptr;
    ButtonParameters *btn4 = nullptr;
    ButtonParameters *btn5 = nullptr;
    ButtonParameters *btn6 = nullptr;

    bool getSqlButtons();

    QVariant getBtn1() { return QVariant::fromValue(btn1); };
    QVariant getBtn2() { return QVariant::fromValue(btn2); };
    QVariant getBtn3() { return QVariant::fromValue(btn3); };
    QVariant getBtn4() { return QVariant::fromValue(btn4); };
    QVariant getBtn5() { return QVariant::fromValue(btn5); };
    QVariant getBtn6() { return QVariant::fromValue(btn6); };

Q_SIGNALS:
    void buttonPressed();

    void rotateLeft();
    void rotateRight();
    void rotateLeftPressed();
    void rotateRightPressed();

    void button1Pressed();
    void button2Pressed();
    void button3Pressed();
    void button4Pressed();
    void button5Pressed();
    void button6Pressed();

    void button1DoublePressed();
    void button2DoublePressed();
    void button3DoublePressed();
    void button4DoublePressed();
    void button5DoublePressed();
    void button6DoublePressed();

    void button1LongPressed();
    void button2LongPressed();
    void button3LongPressed();
    void button4LongPressed();
    void button5LongPressed();
    void button6LongPressed();

protected:
    virtual bool hasPress() { return false; }
    virtual bool hasDoublePress() { return false; }
    virtual bool hasLongPress() { return false; }
    virtual int getButtonCount() { return 0; }

    bool getAction(int button, int mode, int &action, QString &params);
    void triggerAction(int button, int mode);
    void triggerEvent(int button, int mode);

public:
    DeviceBeacon(QString &deviceAddr, QString &deviceName, QObject *parent = nullptr);
    DeviceBeacon(const QBluetoothDeviceInfo &d, QObject *parent = nullptr);
    virtual ~DeviceBeacon();

    Q_INVOKABLE bool setSqlButtons(int button);

public slots:
    //
};

/* ************************************************************************** */
#endif // DEVICE_BEACON_H
