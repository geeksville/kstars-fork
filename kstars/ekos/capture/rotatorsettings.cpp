/*
    SPDX-FileCopyrightText: 2017 Jasem Mutlaq <mutlaqja@ikarustech.com>
                            2022 Toni Schriber

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/******************************************************************************************************
* In 'rotatorGauge' and 'paGauge' all angles are displayed in viewing direction and positiv CCW.
*******************************************************************************************************/

#include "rotatorsettings.h"
#include "Options.h"
#include "fov.h"
#include "kstarsdata.h"
#include "ekos/manager.h"
#include "indi/indirotator.h"
#include <indicom.h>
#include <basedevice.h>
#include <cmath>
#include "capture.h"
#include "ekos/capture/capturedeviceadaptor.h"
#include "ekos/align/align.h"
#include "ekos/align/opsalign.h"
#include "ekos/auxiliary/rotatorutils.h"

#include "ekos_capture_debug.h"

RotatorSettings::RotatorSettings(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    connect(RotatorUtils::Instance(), &RotatorUtils::changedPierside, this, &RotatorSettings::updateZeroPos);
    connect(RotatorUtils::Instance(), &RotatorUtils::adjustParallacticAngle, this, &RotatorSettings::derotateCamera);
    // connect(RotatorUtils::Instance(), &RotatorUtils::adjustParallacticAngle, this, &RotatorSettings::updateParallacticAngle);
    connect(FlipPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RotatorSettings::setFlipPolicy);
    connect(AlignOptions,  &QPushButton::clicked, this, &RotatorSettings::showAlignOptions);

    // -- Parameter -> ui file

    // -- Camera position angle
    CameraPA->setKeyboardTracking(false);
    connect(CameraPA, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,  [ = ](double PAngle)
    {
        double RAngle = RotatorUtils::Instance()->calcRotatorAngle(PAngle);
        RotatorAngle->setValue(RAngle);
        syncFOV(PAngle);
        activateRotator(RAngle);
        CameraPASlider->setSliderPosition(PAngle * 100); // Prevent rounding to integer
    });
    connect(CameraPASlider, &QSlider::sliderReleased, this, [this]()
    {
        CameraPA->setValue(CameraPASlider->sliderPosition() / 100.0); // Set position angle
    });
    connect(CameraPASlider, &QSlider::valueChanged, this, [this](int PAngle100)
    {
        double PAngle = PAngle100 / 100.0;
        paGauge->setValue(-(PAngle)); // Preview cameraPA in gauge
        syncFOV(PAngle); // Preview FOV
    });

    // -- Options
    connect(reverseDirection,  &QCheckBox::toggled, this, [ = ](bool toggled)
    {
        commitRotatorDirection(toggled);
    });
    connect(derotateField, &QCheckBox::toggled, this, &RotatorSettings::setDerotation);

    // Rotator Gauge
    rotatorGauge->setFormat("R");   // dummy format
    rotatorGauge->setMinimum(-360); // display in viewing direction
    rotatorGauge->setMaximum(0);

    // Sky rotation center, default "N" for EQ_GEM
    // if (Options::useAltAz())
    //    SkyCenter->setText("Z"); // zenith for ALTAZ

    // Position Angle Gauge
    QColor TransparentGrey(90, 90, 90, 100);
    paGauge->setFormat("P");   // dummy format
    paGauge->setMinimum(-181); // display in viewing direction
    paGauge->setMaximum(181);
    QPalette p = paGauge->palette();
    p.setColor(QPalette::Shadow, TransparentGrey);
    paGauge->setPalette(p);
    paGauge->setOutlinePenWidth(3);

    // Angle Ruler
    paRuler->plotLayout()->clear();
    m_AngularAxis = new QCPPolarAxisAngular(paRuler);
    m_AngularAxis->removeRadialAxis(m_AngularAxis->radialAxis());
    QPen greyPen(TransparentGrey, 3);
    QPen whitePen(Qt::white, 3);
    m_AngularAxis->setBasePen(greyPen);
    m_AngularAxis->setTickPen(greyPen);
    m_AngularAxis->setSubTickPen(greyPen);
    m_AngularAxis->setTickLabels(false);
    m_AngularAxis->setTickLength(10, 10);
    m_AngularAxis->setSubTickLength(5, 5);
    m_AngularAxis->grid()->setAngularPen(QPen(Qt::GlobalColor::transparent)); // no grid

    /*QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    m_AngularAxis->setTicker(textTicker);
    textTicker->addTick(90, "Ntesteest");*/
    /*m_AngularAxis->setTickLabels(true);
    m_AngularAxis->setTickLabelColor(Qt::white);
    m_AngularAxis->setTickLabelPadding(-10);
    m_AngularAxis->setRange(0, 360, Qt::AlignCenter);
    m_AngularAxis->setRangeReversed(true);*/
    /* m_textTicker = m_AngularAxis->tickVectorLabels();*/

    m_AngularAxis->setAngle(90);
    paRuler->plotLayout()->addElement(0, 0, m_AngularAxis);
    paRuler->setBackground(Qt::GlobalColor::transparent);  // transparent background part 1
    paRuler->setAttribute(Qt::WA_OpaquePaintEvent, false); // transparent background part 2

    paRuler->replot();

    // Parameter Interface
    CameraPA->setMaximum(180.00);   // uniqueness of angle (-180 = 180)
    CameraPA->setMinimum(-179.99);
    RotatorAngle->setButtonSymbols(QAbstractSpinBox::NoButtons);
    CameraOffset->setValue(Options::pAOffset());
    CameraOffset->setButtonSymbols(QAbstractSpinBox::NoButtons);
    MountPierside->setCurrentIndex(ISD::Mount::PIER_UNKNOWN);
    MountPierside->setDisabled(true);  // only show pierside for information
}

void RotatorSettings::initRotator(const QString &train, const QSharedPointer<Ekos::CaptureDeviceAdaptor> CaptureDA,
                                  ISD::Rotator *device)
{
    m_CaptureDA = CaptureDA;

    RotatorUtils::Instance()->initRotatorUtils(train);
    m_Rotator = device;
    RotatorName->setText(m_Rotator->getDeviceName());
    updateFlipPolicy(Options::astrometryFlipRotationAllowed());
    updateDerotation(Options::astrometryUseDerotation());
    updateAltAzMode(Options::useAltAz());
    connect(m_CaptureDA.data(), &Ekos::CaptureDeviceAdaptor::adjustRotatorPA, this, &RotatorSettings::adjustPA);
    // Give getState() a second
    QTimer::singleShot(1000, [ = ]
    {
        if (m_CaptureDA->getRotatorAngleState() < IPS_BUSY)
        {
            connect(this, &RotatorSettings::newLog, Ekos::Manager::Instance()->captureModule(), &Ekos::Capture::appendLogText);
            double RAngle = m_CaptureDA->getRotatorAngle();
            updateRotator(RAngle);
            updateZeroPos(RotatorUtils::Instance()->getMountPierside());
            qCInfo(KSTARS_EKOS_CAPTURE()) << "Rotator Settings: Initial raw angle is" << RAngle << ".";
            emit newLog(i18n("Initial rotator angle %1° is read in successfully.", RAngle));
        }
        else
            qCWarning(KSTARS_EKOS_CAPTURE()) << "Rotator Settings: Reading initial raw angle failed.";
    });
}

void RotatorSettings::updateRotator(double RAngle)
{
    RotatorAngle->setValue(RAngle);
    double PAngle = RotatorUtils::Instance()->calcCameraAngle(RAngle, false);
    CameraPA->blockSignals(true); // Prevent reaction coupling via user input
    CameraPA->setValue(PAngle);
    CameraPA->blockSignals(false);
    CameraPASlider->setSliderPosition(PAngle * 100); // Prevent rounding to integer
    updateGauge(RAngle);
}

void RotatorSettings::updateGauge(double RAngle)
{
    rotatorGauge->setValue(-RAngle); // display in viewing direction
    CurrentRotatorAngle->setText(QString::number(RAngle, 'f', 2));
    paGauge->setValue(-(RotatorUtils::Instance()->calcCameraAngle(RAngle, false)));
}

void RotatorSettings::updateZeroPos(ISD::Mount::PierSide Pierside)
{
    double RAngle = 0;
    if (Pierside == ISD::Mount::PIER_UNKNOWN && !Options::useAltAz())
        MountPierside->setStyleSheet("QComboBox {border: 1px solid red;}");
    else
        MountPierside->setStyleSheet("QComboBox {}");
    MountPierside->setCurrentIndex(Pierside);
    if (Pierside == ISD::Mount::PIER_WEST)
        rotatorGauge->setNullPosition(QRoundProgressBar::PositionTop);
    else if (Pierside == ISD::Mount::PIER_EAST)
        rotatorGauge->setNullPosition(QRoundProgressBar::PositionBottom);
    if (Options::astrometryFlipRotationAllowed()) // Preserve rotator raw angle
        RAngle = RotatorAngle->value();
    else // Preserve camera position angle
    {
        RAngle = RotatorUtils::Instance()->calcRotatorAngle(CameraPA->value());
        activateRotator(RAngle);
    }
    updateGauge(RAngle);
    updateRotator(RAngle);
}

// signal from drawTelescopeSymbols() (->sykmapabstract.cpp) to simulate passive
// rotation of camera for AltAz (cf. parallactic angle)
void RotatorSettings::adjustPA(double DeltaAngle, bool adjustFOV)
{
    if (m_CaptureDA->getRotatorAngleState() < IPS_BUSY)
    {
        double RAngle = RotatorAngle->value();
        double PAngle = KSUtils::rangePA(CameraPA->value() + DeltaAngle);
        CameraPA->blockSignals(true); // do NOT change rotator angle!
        CameraPASlider->blockSignals(true); // dito!
        CameraPA->setValue(PAngle);
        CameraPASlider->setSliderPosition(PAngle * 100);
        CameraPA->blockSignals(false);
        CameraPASlider->blockSignals(false);
        double OAngle = RotatorUtils::Instance()->calcOffsetAngle(RAngle, PAngle);
        RotatorUtils::Instance()->updateOffset(OAngle);
        CameraOffset->setValue(OAngle);
        paGauge->setValue(-(PAngle)); // Preview cameraPA in gauge
        if (adjustFOV)
            RotatorSettings::syncFOV(PAngle); // Preview FOV´
    }
}

/*void RotatorSettings::updateParallacticAngle(const double DeltaAngle, const double BaseAngle)
{
    double PAngle = CameraPA->value() + DeltaAngle;
    paGauge->setNullPosition(90 - BaseAngle);
    // rotatorGauge->setNullPosition(90 - BaseAngle); // PositionLeft = 0, CCW
    m_AngularAxis->setAngle(BaseAngle + 90);
    paRuler->replot();
    CameraPA->blockSignals(true); // Prevent reaction coupling via user input
    CameraPA->setValue(PAngle);
    CameraPA->blockSignals(false);
    CameraPASlider->setSliderPosition(PAngle * 100); // Prevent rounding to integer

}

void RotatorSettings::updateParallacticAngle(const double DeltaAngle, const double BaseAngle)
{
    Q_UNUSED(DeltaAngle);
    m_CaptureDA->setSimCameraRotation(BaseAngle);
}*/

void RotatorSettings::setFlipPolicy(const int index)
{
    Ekos::OpsAlign *AlignOptions = Ekos::Manager::Instance()->alignModule()->getAlignOptionsModule();
    Ekos::OpsAlign::FlipPriority Priority = static_cast<Ekos::OpsAlign::FlipPriority>(index);
    if (AlignOptions)
        AlignOptions->setFlipPolicy(Priority);
}

void RotatorSettings::updateFlipPolicy(const bool FlipRotationAllowed)
{
    int i = -1;
    if (FlipRotationAllowed)
        i = static_cast<int>(Ekos::OpsAlign::FlipPriority::ROTATOR_ANGLE);
    else
        i = static_cast<int>(Ekos::OpsAlign::FlipPriority::POSITION_ANGLE);
    FlipPolicy->blockSignals(true); // Prevent reaction coupling
    FlipPolicy->setCurrentIndex(i);
    FlipPolicy->blockSignals(false);
}
void RotatorSettings::setDerotation(bool toggled)
{
    Ekos::OpsAlign *AlignOptions = Ekos::Manager::Instance()->alignModule()->getAlignOptionsModule();
    if (AlignOptions)
        AlignOptions->setDerotation(toggled);
}

void RotatorSettings::updateDerotation(const bool toggled)
{
    derotateField->blockSignals(true);
    derotateField->setChecked(toggled);
    derotateField->blockSignals(false);
}

void RotatorSettings::updateAltAzMode(const bool AltAz)
{
    if (AltAz)
    {
        FlipPolicy->setDisabled(true);
        derotateField->setDisabled(false);
        MountPierside->setStyleSheet("QComboBox {}");
    }
    else
    {
        FlipPolicy->setDisabled(false);
        derotateField->setDisabled(true);
        MountPierside->setStyleSheet("QComboBox {border: 1px solid red;}");
    }
}

void RotatorSettings::showAlignOptions()
{
    KConfigDialog * alignSettings = KConfigDialog::exists("alignsettings");
    if (alignSettings)
    {
        alignSettings->setEnabled(true);
        alignSettings->show();
    }
}

void RotatorSettings::activateRotator(double Angle)
{
    m_CaptureDA->setRotatorAngle(Angle);
}

void RotatorSettings::commitRotatorDirection(bool Reverse)
{
    m_CaptureDA->reverseRotator(Reverse);
}

void RotatorSettings::refresh(double PAngle) // Call from setAlignResults() in Module Capture
{
    CameraPA->setValue(PAngle);
    syncFOV(PAngle);
    CameraOffset->setValue(Options::pAOffset());
}

void RotatorSettings::syncFOV(double PA)
{
    for (auto oneFOV : KStarsData::Instance()->getTransientFOVs())
    {
        // Only change the PA for the sensor FOV
        if (oneFOV->objectName() == "sensor_fov")
        {
            // Make sure that it is always displayed
            if (!Options::showSensorFOV())
            {
                Options::setShowSensorFOV(true);
                oneFOV->setProperty("visible", true);
            }

            // JM 2020-10-15
            // While we have the correct Position Angle
            // Because Ekos reads frame TOP-BOTTOM instead of the BOTTOM-TOP approach
            // used by astrometry, the PA is always 180 degree off. To avoid confusion to the user
            // the PA is drawn REVERSED to show the *expected* frame. However, the final PA is
            // the "correct" PA as expected by astrometry.
            //double drawnPA = PA >= 0 ? (PA - 180) : (PA + 180);
            oneFOV->setPA(PA);
            break;
        }
    }
}

void RotatorSettings::derotateCamera(const double DeltaAngle, const double BaseAngle)
{
    emit newLog(i18n("Camera rotated by %1° with base %2°.", DeltaAngle, BaseAngle));
    double RAngle = KSUtils::range360(RotatorAngle->value() + DeltaAngle);
    m_CaptureDA->setRotatorAngle(RAngle);
}
