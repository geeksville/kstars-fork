/*  Ekos Focus tool
    Copyright (C) 2012 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */

#ifndef FOCUS_H
#define FOCUS_H

#include "focus.h"
#include "capture.h"

#include "ui_focus.h"


#include "indi/indistd.h"
#include "indi/indifocuser.h"

class KPlotObject;

namespace Ekos
{

struct HFRPoint
{
    int pos;
    double HFR;
};

class Focus : public QWidget, public Ui::Focus
{

    Q_OBJECT

public:
    Focus();
    ~Focus();

    void setFocuser(ISD::GDInterface *newFocuser);
    void addCCD(ISD::GDInterface *newCCD);
    void focuserDisconnected();

    typedef enum { FOCUS_NONE, FOCUS_IN, FOCUS_OUT } FocusDirection;
    typedef enum { FOCUS_MANUAL, FOCUS_AUTO, FOCUS_LOOP } FocusType;

    void appendLogText(const QString &);
    void clearLog();
    QString getLogText() { return logText.join("\n"); }

public slots:

    /* Focus */
    void startFocus();
    void stopFocus();
    void capture();
    void startLooping();

    void checkCCD(int CCDNum);

    void FocusIn(int ms=-1);
    void FocusOut(int ms=-1);

    void toggleAutofocus(bool enable);

    void newFITS(IBLOB *bp);
    void processFocusProperties(INumberVectorProperty *nvp);

signals:
        void newLog();

private:

    void getAbsFocusPosition();
    void autoFocusAbs(double currentHFR);
    void autoFocusRel(double currentHFR);

    void resetButtons();


    /* Focus */
    ISD::Focuser *currentFocuser;
    ISD::CCD *currentCCD;

    QList<ISD::CCD *> CCDs;

    Ekos::Capture *captureP;

    FocusDirection lastFocusDirection;
    FocusType focusType;

    double HFR;
    int pulseDuration;
    bool canAbsMove;
    int absIterations;

    bool inAutoFocus, inFocusLoop;

    double absCurrentPos;
    double pulseStep;
    double absMotionMax, absMotionMin;
    int HFRInc;
    int HFRDec;
    bool reverseDir;

    QStringList logText;

    QList<HFRPoint *> HFRPoints;
};

}

#endif  // Focus_H
