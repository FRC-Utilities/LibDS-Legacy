/*
 * Copyright (C) 2015-2016 Alex Spataru <alex_spataru@outlook>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "Window.h"
#include "DriverStation.h"

#include <math.h>
#include <QMessageBox>

/**
 * Initializes the window and its widgets.
 * Finally, this function also configures the respective signal/slot connections
 * between the window's widgets and the driver station class.
 */
Window::Window (QWidget* parent) : QMainWindow (parent)
{
    ui = new Ui::Window;
    ui->setupUi (this);

    /* Set window geometry */
    move (100, 100);
    setFixedSize (minimumSizeHint());

    /* Set window title to {OS} Driver Station */
#if defined (Q_OS_LINUX)
    setWindowTitle (tr ("Linux Driver Station"));
#elif defined (Q_OS_MAC)
    setWindowTitle (tr ("Mac OSX Driver Station"));
#elif defined (Q_OS_WIN)
    setWindowTitle (tr ("Windows Driver Station"));
#endif

    /* Get DriverStation instance */
    ds = DriverStation::getInstance();

    /* Fill UI options */
    ui->Protocols->addItems (ds->protocols());
    ui->TeamStation->addItems (ds->stations());

    /* React to enable/disable buttons */
    connect (ui->StatusGroup, SIGNAL (buttonClicked (int)),
             this,             SLOT  (updateEnabled (int)));

    /* React to user changing control modes */
    connect (ui->ModesGroup, SIGNAL (buttonClicked (int)),
             this,             SLOT (updateControlMode (int)));

    /* Update team number automatically */
    connect (ui->TeamNumber, SIGNAL (valueChanged (int)),
             ds,             SLOT   (setTeamNumber (int)));

    /* Update protocol automatically */
    connect (ui->Protocols, SIGNAL (currentIndexChanged (int)),
             ds,              SLOT (setProtocol (int)));

    /* Make sure that enabled buttons are in sync with DS */
    connect (ds,       SIGNAL (enabledChanged (bool)),
             ui->Enable, SLOT (setChecked (bool)));

    /* Update team station automatically */
    connect (ui->TeamStation, SIGNAL (currentIndexChanged (int)),
             ds,                SLOT (setTeamStation (int)));

    /* Update netconsole text automatically */
    connect (ds,        SIGNAL (newMessage (QString)),
             ui->Console, SLOT (append (QString)));

    /* Update voltage automatically */
    connect (ds, SIGNAL (voltageChanged (double)),
             this, SLOT (setVoltage (double)));

    /* Update status text automatically */
    connect (ds,       SIGNAL (statusChanged (QString)),
             ui->Status, SLOT (setText (QString)));

    /* Initialize the DS with the 2016 protocol */
    ds->setProtocol (DriverStation::kProtocol2016);
}

/**
 * Destroys the window's widgets
 */
Window::~Window()
{
    delete ui;
}

/**
 * Called when the user clicks any of the enable/disable buttons.
 * Checks if the user wants to enable the robot and enables it only if it is
 * safe to do so, otherwise, the function shall warn the user and disable the
 * robot.
 *
 * \param unused unused value (which is needed for the singal/slot connection)
 */
void Window::updateEnabled (int unused)
{
    Q_UNUSED (unused);

    /* User wants to enable the robot under safe conditions */
    if (ui->Enable->isChecked() && ds->canBeEnabled()) {
        ds->setEnabled (true);
        return;
    }

    /* It is not safe to enable the robot */
    else if (ui->Enable->isChecked()) {
        QMessageBox::warning (this,
                              tr ("Error"),
                              tr ("You cannot enable the robot with "
                                  "the current conditions!"));
    }

    /* Disable the robot and update UI values */
    ds->setEnabled (false);
    ui->Enable->setChecked (false);
    ui->Disable->setChecked (true);
}

/**
 * Called when the user changes the desired operation mode of the robot
 *
 * \param unused unused value (which is needed for the singal/slot connection)
 */
void Window::updateControlMode (int unused)
{
    Q_UNUSED (unused);

    /* Switch to test mode */
    if (ui->Test->isChecked())
        ds->switchToTestMode();

    /* Switch to autonomous */
    else if (ui->Autonomous->isChecked())
        ds->switchToAutonomous();

    /* Switch to teleoperated */
    else if (ui->Teleoperated->isChecked())
        ds->switchToTeleoperated();
}

/**
 * Called when the Driver Station detects a different robot voltage, this
 * function reduces the voltage to the nearest two decimal places and updates
 * the user interface.
 *
 * \param voltage the voltage detected by the Driver Station
 */
void Window::setVoltage (double voltage)
{
    voltage = roundf (voltage * 100) / 100;
    ui->Voltage->setText (QString ("%1 V").arg (voltage));
}