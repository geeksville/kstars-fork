/*
    KStars UI tests for capture workflows (re-focus, dithering, guiding, ...)

    SPDX-FileCopyrightText: 2021 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_ekos_capture_workflow_advanced.h"

#include "kstars_ui_tests.h"
#include "ui_calibrationoptions.h"
#include "indicom.h"
#include "Options.h"
#include "skymapcomposite.h"
#include "ekos/capture/capture.h"


TestEkosCaptureWorkflowAdvanced::TestEkosCaptureWorkflowAdvanced(QObject *parent) :
    TestEkosCaptureWorkflowAdvanced::TestEkosCaptureWorkflowAdvanced("Internal", parent) {}

TestEkosCaptureWorkflowAdvanced::TestEkosCaptureWorkflowAdvanced(QString guider,
        QObject *parent)  : QObject(parent)
{
    m_CaptureHelper = new TestEkosCaptureHelper(guider);
    m_CaptureHelper->m_GuiderDevice = "CCD Simulator";
}

void TestEkosCaptureWorkflowAdvanced::initTestCase()
{
    KVERIFY_EKOS_IS_HIDDEN();
    // limit guiding pulses to ensure that guiding deviations lead to aborted capture
    Options::setRAMaximumPulseArcSec(5.0);
    Options::setDECMaximumPulseArcSec(5.0);

    QStandardPaths::setTestModeEnabled(true);

    // wait for all module settings updated
    m_CaptureHelper->waitForSettingsUpdated = true;

}

void TestEkosCaptureWorkflowAdvanced::cleanupTestCase()
{
    // nothing to do since we start the INDI service for each test case
}

void TestEkosCaptureWorkflowAdvanced::init()
{
    // reset calibration
    Options::setCalibrationPreActionIndex(ACTION_NONE);

    KTRY_OPEN_EKOS();
    KVERIFY_EKOS_IS_OPENED();
    // disable reset jobs warning
    KMessageBox::saveDontShowAgainYesNo("reset_job_status_warning", KMessageBox::ButtonCode::No);
}

void TestEkosCaptureWorkflowAdvanced::cleanup()
{
    m_CaptureHelper->cleanup();
    QVERIFY(m_CaptureHelper->shutdownEkosProfile());
    KTRY_CLOSE_EKOS();
    KVERIFY_EKOS_IS_HIDDEN();
}

void TestEkosCaptureWorkflowAdvanced::testGuidingDeviationSuspendingCapture()
{
    // default initialization
    QVERIFY(m_CaptureHelper->prepareTestCase());

    const double deviation_limit = 2.0;
    // switch to capture module
    Ekos::Capture *capture = Ekos::Manager::Instance()->captureModule();
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    // set guide deviation guard to < 2"
    KTRY_SET_CHECKBOX(capture, enforceGuideDeviation, true);
    KTRY_SET_DOUBLESPINBOX(capture, guideDeviation, deviation_limit);

    // add target to path to emulate the behavior of the scheduler
    QString imagepath = m_CaptureHelper->getImageLocation()->path() + "/test";
    // build a LRGB sequence
    KTRY_CAPTURE_ADD_LIGHT(30.0, 1, 5.0, "Luminance", "test", imagepath);
    KTRY_CAPTURE_ADD_LIGHT(30.0, 1, 5.0, "Red", "test", imagepath);
    KTRY_CAPTURE_ADD_LIGHT(30.0, 1, 5.0, "Green", "test", imagepath);
    KTRY_CAPTURE_ADD_LIGHT(30.0, 1, 5.0, "Blue", "test", imagepath);

    // set a position in the west
    SkyPoint *target = new SkyPoint();
    target->setAz(270.0);
    target->setAlt(KStarsData::Instance()->geo()->lat()->Degrees() / 2.0);
    // translate to equatorial coordinates
    const dms lst = KStarsData::Instance()->geo()->GSTtoLST(KStarsData::Instance()->clock()->utc().gst());
    const dms *lat = KStarsData::Instance()->geo()->lat();
    target->HorizontalToEquatorial(&lst, lat);

    m_CaptureHelper->slewTo(target->ra().Hours(), target->dec().Degrees(), true);

    // clear calibration to ensure proper guiding
    KTRY_CLICK(Ekos::Manager::Instance()->guideModule(), clearCalibrationB);

    // start guiding
    m_CaptureHelper->startGuiding(2.0);

    // start capture
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_CAPTURING);
    KTRY_CLICK(capture, startB);
    // wait until capturing starts
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 10000);
    // wait for settling
    QTest::qWait(2000);
    // create a guide drift
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_SUSPENDED);
    Ekos::Manager::Instance()->mountModule()->doPulse(RA_INC_DIR, 2000, DEC_INC_DIR, 2000);
    qCInfo(KSTARS_EKOS_TEST()) << "Sent 2000ms RA+DEC guiding pulses.";
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 30000);
    // expect that capturing continues
    QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getCaptureStatus() == Ekos::CAPTURE_CAPTURING, 60000);
    // verify that capture starts only once
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_PROGRESS);
    QTest::qWait(20000);
    QVERIFY2(m_CaptureHelper->expectedCaptureStates.size() > 0, "Multiple capture starts.");
}

void TestEkosCaptureWorkflowAdvanced::testGuidingDeviationAbortCapture()
{
    // default initialization
    QVERIFY(m_CaptureHelper->prepareTestCase());

    const double deviation_limit = 2.0;
    // switch to capture module
    Ekos::Capture *capture = Ekos::Manager::Instance()->captureModule();
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    // set guide deviation guard to < 2"
    KTRY_SET_CHECKBOX(capture, enforceGuideDeviation, true);
    KTRY_SET_DOUBLESPINBOX(capture, guideDeviation, deviation_limit);

    // add target to path to emulate the behavior of the scheduler
    QString imagepath = m_CaptureHelper->getImageLocation()->path() + "/test";
    // build a simple 5xm_CaptureHelper->L sequence
    KTRY_CAPTURE_ADD_LIGHT(45.0, 5, 5.0, "Luminance", "", imagepath);
    // set Dubhe as target and slew there
    SkyObject *target = KStars::Instance()->data()->skyComposite()->findByName("Dubhe");
    m_CaptureHelper->slewTo(target->ra().Hours(), target->dec().Degrees(), true);

    // clear calibration to ensure proper guiding
    KTRY_CLICK(Ekos::Manager::Instance()->guideModule(), clearCalibrationB);

    // start guiding
    m_CaptureHelper->startGuiding(2.0);

    // start capture
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_CAPTURING);
    KTRY_CLICK(capture, startB);
    // wait until capturing starts
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 10000);
    // wait for settling
    QTest::qWait(2000);
    // create a guide drift
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_SUSPENDED);
    Ekos::Manager::Instance()->mountModule()->doPulse(RA_INC_DIR, 2000, DEC_INC_DIR, 2000);
    qCInfo(KSTARS_EKOS_TEST()) << "Sent 2000ms RA+DEC guiding pulses.";
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    // wait that capturing gets suspended
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 15000);
    // abort capturing
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_ABORTED);
    KTRY_CLICK(capture, startB);
    // check that it has been aborted
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 10000);
    // wait that the guiding deviation is below the limit and
    // verify that capture does not start
    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_PROGRESS);
    QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getGuideDeviation() < deviation_limit, 60000);

    QTest::qWait(20000);
    QVERIFY2(m_CaptureHelper->expectedCaptureStates.size() > 0, "Capture has been restarted although aborted.");
}

void TestEkosCaptureWorkflowAdvanced::testInitialGuidingLimitCapture()
{
    // default initialization
    QVERIFY(m_CaptureHelper->prepareTestCase());

    const double deviation_limit = 2.0;
    QFETCH(double, exptime);
    // switch to capture module
    Ekos::Capture *capture = Ekos::Manager::Instance()->captureModule();
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    // set start guide deviation guard to < 2" but disable the other one
    KTRY_SET_CHECKBOX(capture, enforceStartGuiderDrift, true);
    KTRY_SET_DOUBLESPINBOX(capture, startGuideDeviation, deviation_limit);
    // create sequence with 10 sec delay
    QVERIFY(m_CaptureHelper->prepareCapture(0, 0, 0, 10));
    // set Dubhe as target and slew there
    SkyObject *target = KStars::Instance()->data()->skyComposite()->findByName("Dubhe");
    m_CaptureHelper->slewTo(target->ra().Hours(), target->dec().Degrees(), true);

    // start guiding
    m_CaptureHelper->startGuiding(2.0);

    for (int i = 1; i <= 2; i++)
    {
        // wait intially 5 seconds
        if (i == 1)
            QTest::qWait(5000);

        // prepare to expect that capturing will start
        m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_CAPTURING);

        // ensure that guiding is running
        QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getGuidingStatus() == Ekos::GUIDE_GUIDING, 10000);

        // create a guide drift
        Ekos::Manager::Instance()->mountModule()->doPulse(RA_INC_DIR, 2000, DEC_INC_DIR, 2000);
        qCInfo(KSTARS_EKOS_TEST()) << "Sent 2000ms RA+DEC guiding pulses.";

        // wait until guide deviation is present
        QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getGuideDeviation() > deviation_limit, 15000);

        if (i == 1)
        {
            // start capture but expect it being suspended first
            KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
            KTRY_CLICK(capture, startB);
        }
        // verify that capturing does not start before the guide deviation is below the limit
        QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getGuideDeviation() >= deviation_limit, 60000);
        // wait 3 seconds and then ensure that capture did not start
        QTest::qWait(3000);
        QTRY_VERIFY(m_CaptureHelper->expectedCaptureStates.size() > 0);
        // wait until guiding deviation is below the limit
        QTRY_VERIFY_WITH_TIMEOUT(m_CaptureHelper->getGuideDeviation() < deviation_limit, 60000);
        // wait until capturing starts
        KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 30000);
        if (i < 2)
        {
            // in the first iteration wait until the capture completes
            m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_IMAGE_RECEIVED);
            KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 1500 * exptime);
        }
    }
}


//void TestEkosCaptureWorkflow::testPreMountAndDomePark()
//{
//    // use the light panel simulator
//    m_CaptureHelper->m_LightPanelDevice = "Light Panel Simulator";
//    // use the dome simulator
//    m_CaptureHelper->m_DomeDevice = "Dome Simulator";
//    // default initialization
//    QVERIFY(m_CaptureHelper->prepareTestCase());

//    // QSKIP("Observatory refactoring needs to be completed until this test can be activated.");

//    // switch to capture module
//    Ekos::Capture *capture = Ekos::Manager::Instance()->captureModule();
//    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);

//    // use a test directory for flats
//    QString imagepath = getImageLocation()->path() + "/test";

//    // switch capture type to flat so that we can set the calibration
//    KTRY_SET_COMBO(capture, captureTypeS, "Flat");

//    // select internal flat light, pre-mount and but not pre-dome park
//    KTRY_SELECT_FLAT_METHOD(flatDeviceSourceC, true, false);
//    // determine frame type
//    QFETCH(QString, frametype);
//    // build a simple 1xL sequence
//    KTRY_CAPTURE_ADD_FRAME(frametype, 2, 1, 2.0, "Luminance", imagepath);

//    // start the sequence
//    // m_CaptureHelper->expectedDomeStates.append(ISD::Dome::DOME_PARKED);
//    m_CaptureHelper->expectedMountStates.append(ISD::Mount::MOUNT_PARKED);
//    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_CAPTURING);
//    m_CaptureHelper->expectedCaptureStates.append(Ekos::CAPTURE_COMPLETE);
//    KTRY_CLICK(capture, startB);
//    // check if mount has reached the expected position
//    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedMountStates, 30000);
//    // check if dome has reached the expected position
//    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedDomeStates, 30000);
//    // check if one single flat is captured
//    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(m_CaptureHelper->expectedCaptureStates, 60000);
//}



/* *********************************************************************************
 *
 * Test data
 *
 * ********************************************************************************* */

void TestEkosCaptureWorkflowAdvanced::testInitialGuidingLimitCapture_data()
{
    m_CaptureHelper->prepareTestData(20.0, {"Luminance:5"});
}

//void TestEkosCaptureWorkflow::testPreMountAndDomePark_data()
//{
//    testWallSource_data();
//}


/* *********************************************************************************
 *
 * Main function
 *
 * ********************************************************************************* */

QTEST_KSTARS_WITH_GUIDER_MAIN(TestEkosCaptureWorkflowAdvanced)
