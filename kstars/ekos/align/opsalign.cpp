/*
    SPDX-FileCopyrightText: 2017 Jasem Mutlaq <mutlaqja@ikarustech.com>
    SPDX-FileCopyrightText: 2017 Robert Lancaster <rlancaste@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "opsalign.h"

#include "align.h"
#include "fov.h"
#include "kstars.h"
#include "ksnotification.h"
#include "Options.h"
#include "kspaths.h"
#include "ekos/auxiliary/rotatorutils.h"
#include "ekos/auxiliary/stellarsolverprofile.h"

#include <KConfigDialog>
#include <QProcess>
#include <ekos_align_debug.h>

namespace Ekos
{
OpsAlign::OpsAlign(Align *parent) : QWidget(KStars::Instance())
{
    setupUi(this);

    m_AlignModule = parent;

    //Get a pointer to the KConfigDialog
    m_ConfigDialog = KConfigDialog::exists("alignsettings");

    editSolverProfile->setIcon(QIcon::fromTheme("document-edit"));
    editSolverProfile->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    connect(editSolverProfile, &QAbstractButton::clicked, this, [this]
    {
        emit needToLoadProfile(kcfg_SolveOptionsProfile->currentText());
    });

    reloadOptionsProfiles();   

    connect(m_ConfigDialog->button(QDialogButtonBox::Apply), SIGNAL(clicked()), SIGNAL(settingsUpdated()));
    connect(m_ConfigDialog->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SIGNAL(settingsUpdated()));
    connect(m_ConfigDialog->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SIGNAL(settingsUpdated()));
    connect(estimateThreshold, &QPushButton::clicked, this, [this]
    {
        double MaxAngle = 0;
        MaxAngle = RotatorUtils::Instance()->calcDerotationThreshold();
        if (MaxAngle > 0)
            kcfg_AstrometryDerotationThreshold->setValue(MaxAngle);
    });

}

void OpsAlign::setFlipPolicy(const Ekos::OpsAlign::FlipPriority Priority)
{
    if (Priority == Ekos::OpsAlign::ROTATOR_ANGLE)
        kcfg_AstrometryFlipRotationAllowed->setChecked(true);
    else if (Priority == Ekos::OpsAlign::POSITION_ANGLE)
        FlipRotationNotAllowed->setChecked(true);
    OpsAlign::update();
    emit m_ConfigDialog->button(QDialogButtonBox::Apply)->click();
}

void OpsAlign::setDerotation(bool toggled)
{
    if (toggled)
        kcfg_AstrometryUseDerotation->setChecked(true);
    else
        kcfg_AstrometryUseDerotation->setChecked(false);
    OpsAlign::update();
    emit m_ConfigDialog->button(QDialogButtonBox::Apply)->click();
}

void OpsAlign::setAltAzMode(const bool AltAz)
{
    if (AltAz)
    {
        EQmount->setDisabled(true);
        EQmount->setHidden(true);
        ALTAZmount->setDisabled(false);
        ALTAZmount->setHidden(false);
    }
    else
    {
        EQmount->setDisabled(false);
        EQmount->setHidden(false);
        ALTAZmount->setDisabled(true);
        ALTAZmount->setHidden(true);
    }
    OpsAlign::update();
    // emit m_ConfigDialog->button(QDialogButtonBox::Apply)->click();
}

void OpsAlign::reloadOptionsProfiles()
{
    QString savedOptionsProfiles = QDir(KSPaths::writableLocation(
                                            QStandardPaths::AppLocalDataLocation)).filePath("SavedAlignProfiles.ini");

    if(QFile(savedOptionsProfiles).exists())
        optionsList = StellarSolver::loadSavedOptionsProfiles(savedOptionsProfiles);
    else
        optionsList = getDefaultAlignOptionsProfiles();
    int currentIndex = kcfg_SolveOptionsProfile->currentIndex();
    kcfg_SolveOptionsProfile->clear();
    for(auto &param : optionsList)
        kcfg_SolveOptionsProfile->addItem(param.listName);

    if (currentIndex >= 0)
    {
        kcfg_SolveOptionsProfile->setCurrentIndex(currentIndex);
        Options::setSolveOptionsProfile(currentIndex);
    }
    else
        kcfg_SolveOptionsProfile->setCurrentIndex(Options::solveOptionsProfile());
}
}
