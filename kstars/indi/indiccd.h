/*  INDI CCD
    Copyright (C) 2012 Jasem Mutlaq <mutlaqja@ikarustech.com>

    This application is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
 */


#ifndef INDICCD_H
#define INDICCD_H

#include "indistd.h"

namespace ISD
{

class CCD : public DeviceDecorator
{
    Q_OBJECT

public:

    CCD(GDInterface *iPtr);
    ~CCD();

    void processSwitch(ISwitchVectorProperty *svp);
    void processText(ITextVectorProperty* tvp);
    void processNumber(INumberVectorProperty *nvp);
    void processLight(ILightVectorProperty *lvp);
    void processBLOB(IBLOB *bp);

    DeviceFamily getType() { return dType;}



    // Common commands
    bool getFrame(int *x, int *y, int *w, int *h);
    bool capture(double exposure);
    bool setFrameType(CCDFrameType fType);
    bool setBinning(int bin_x, int bin_y);
    bool setBinning(CCDBinType binType);

    // If CCD has ST4 port
    bool canGuide();
    bool doPulse(GuideDirection ra_dir, int ra_msecs, GuideDirection dec_dir, int dec_msecs );
    bool doPulse(GuideDirection dir, int msecs );

    // Utitlity functions
    void setCaptureMode(FITSMode mode) { captureMode = mode; }
    void setCaptureFilter(FITSScale fType) { captureFilter = fType; }
    void setISOMode(bool enable) { ISOMode = enable; }
    void setBatchMode(bool enable) { batchMode = enable; }
    void setSeqPrefix(const QString &preFix) { seqPrefix = preFix; }
    void setSeqCount(int count) { seqCount = count; }

    FITSViewer *getViewer() { return fv;}
    int getTabID();

public slots:
    void FITSViewerDestroyed();
    void StreamWindowDestroyed();

private:
    FITSMode captureMode;
    FITSScale captureFilter;
    bool batchMode;
    bool ISOMode;
    QString		seqPrefix;
    int seqCount;
    int focusTabID, guideTabID;
    FITSViewer * fv;
    StreamWG *streamWindow;

};

}
#endif // INDICCD_H
