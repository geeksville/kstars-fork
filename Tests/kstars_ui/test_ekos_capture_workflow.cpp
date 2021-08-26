/*
    KStars UI tests for capture workflows (re-focus, dithering, guiding, ...)

    SPDX-FileCopyrightText: 2021 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_ekos_capture_workflow.h"

#include "kstars_ui_tests.h"
#include "indi/guimanager.h"
#include "Options.h"


TestEkosCaptureWorkflow::TestEkosCaptureWorkflow(QObject *parent) : QObject(parent)
{
    m_CaptureHelper = new TestEkosCaptureHelper();
}

bool TestEkosCaptureWorkflow::executeFocusing()
{
    // switch to focus module
    Ekos::Focus *focus = Ekos::Manager::Instance()->focusModule();
    KWRAP_SUB(KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(focus, 1000));

    // run initial focus sequence
    expectedFocusStates.append(Ekos::FOCUS_COMPLETE);
    KTRY_CLICK_SUB(focus, startFocusB);
    KWRAP_SUB(KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(expectedFocusStates, 180000));
    // success
    return true;
}

void TestEkosCaptureWorkflow::testCaptureRefocus()
{
    Ekos::Manager *manager = Ekos::Manager::Instance();
    QVERIFY(prepareCapture());
    QVERIFY(executeFocusing());

    // start capturing, expect focus after first captured frame
    Ekos::Capture *capture = manager->captureModule();
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    expectedCaptureStates.append(Ekos::CAPTURE_IMAGE_RECEIVED);
    expectedCaptureStates.append(Ekos::CAPTURE_FOCUSING);
    KTRY_CLICK(capture, startB);
    // focusing must have started 60 secs + exposure time + 10 secs delay
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(expectedCaptureStates, 60000 + 10000 + 1000*capture->captureExposureN->value());
}

void TestEkosCaptureWorkflow::testCaptureRefocusAbort()
{
    Ekos::Manager *manager = Ekos::Manager::Instance();
    QVERIFY(prepareCapture());
    QVERIFY(executeFocusing());

    // start capturing, expect focus after first captured frame
    Ekos::Capture *capture = manager->captureModule();
    KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000);
    expectedFocusStates.append(Ekos::FOCUS_PROGRESS);
    KTRY_CLICK(capture, startB);
    // focusing must have started 60 secs + exposure time + 10 secs delay
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(expectedFocusStates, 60000 + 10000 + 1000*capture->captureExposureN->value());
    // now abort capturing
    expectedFocusStates.append(Ekos::FOCUS_ABORTED);
    capture->abort();
    // expect that focusing aborts
    KVERIFY_EMPTY_QUEUE_WITH_TIMEOUT(expectedFocusStates, 10000);
}


/* *********************************************************************************
 *
 * Test data
 *
 * ********************************************************************************* */

void TestEkosCaptureWorkflow::testCaptureRefocus_data()
{
    prepareTestData(31.0, "Luminance:3");
}

void TestEkosCaptureWorkflow::testCaptureRefocusAbort_data()
{
    prepareTestData(31.0, "Luminance:3");
}


/* *********************************************************************************
 *
 * Test infrastructure
 *
 * ********************************************************************************* */

void TestEkosCaptureWorkflow::initTestCase()
{
    KVERIFY_EKOS_IS_HIDDEN();
    KTRY_OPEN_EKOS();
    KVERIFY_EKOS_IS_OPENED();
    // start the profile
    QVERIFY(m_CaptureHelper->startEkosProfile());
    m_CaptureHelper->init();
    QStandardPaths::setTestModeEnabled(true);

    QFileInfo test_dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation), "test");
    destination = new QTemporaryDir(test_dir.absolutePath());
    QVERIFY(destination->isValid());
    QVERIFY(destination->autoRemove());

    QVERIFY(prepareTestCase());
}

void TestEkosCaptureWorkflow::cleanupTestCase()
{
    // disconnect to the capture process to receive capture status changes
    disconnect(Ekos::Manager::Instance()->alignModule(), &Ekos::Align::newImage, this,
               &TestEkosCaptureWorkflow::imageReceived);
    // disconnect to the alignment process to receive align status changes
    disconnect(Ekos::Manager::Instance()->mountModule(), &Ekos::Mount::newStatus, this,
               &TestEkosCaptureWorkflow::telescopeStatusChanged);

    m_CaptureHelper->cleanup();
    QVERIFY(m_CaptureHelper->shutdownEkosProfile());
    KTRY_CLOSE_EKOS();
    KVERIFY_EKOS_IS_HIDDEN();
}

bool TestEkosCaptureWorkflow::prepareTestCase()
{
    Ekos::Manager * const ekos = Ekos::Manager::Instance();

    // set logging defaults for alignment
    Options::setVerboseLogging(false);
    Options::setLogToFile(false);

    // set mount defaults
    KWRAP_SUB(QTRY_VERIFY_WITH_TIMEOUT(ekos->mountModule() != nullptr, 5000));
    // set primary scope to my favorite FSQ-85 with QE 0.73 Reducer
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->mountModule(), primaryScopeFocalIN, 339.0);
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->mountModule(), primaryScopeApertureIN, 85.0);
    // set guide scope to a Tak 7x50
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->mountModule(), guideScopeFocalIN, 170.0);
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->mountModule(), guideScopeApertureIN, 50.0);
    // save values
    KTRY_CLICK_SUB(ekos->mountModule(), saveB);
    // disable meridian flip
    KTRY_SET_CHECKBOX_SUB(ekos->mountModule(), meridianFlipCheckBox, false);
    // switch to focus module
    KWRAP_SUB(QTRY_VERIFY_WITH_TIMEOUT(ekos->focusModule() != nullptr, 5000));
    // set exposure time to 5 sec
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->focusModule(), exposureIN, 5.0);
    // use full field
    KTRY_SET_CHECKBOX_SUB(ekos->focusModule(), useFullField, true);
    // use iterative algorithm
    KTRY_SET_COMBO_SUB(ekos->focusModule(), focusAlgorithmCombo, "Polynomial");
    // step size 5000
    KTRY_SET_SPINBOX_SUB(ekos->focusModule(), stepIN, 5000);
    // set max travel to 100000
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->focusModule(), maxTravelIN, 100000);
    // set low accuracy to 10%
    KTRY_SET_DOUBLESPINBOX_SUB(ekos->focusModule(), toleranceIN, 10.0);

    // close INDI window
    GUIManager::Instance()->close();

    // connect to the mount process to receive status changes
    connect(ekos->mountModule(), &Ekos::Mount::newStatus, this,
            &TestEkosCaptureWorkflow::telescopeStatusChanged, Qt::UniqueConnection);
    // connect to the capture process
    connect(ekos->captureModule(), &Ekos::Capture::newStatus, this,
            &TestEkosCaptureWorkflow::captureStatusChanged, Qt::UniqueConnection);
    // connect to the focus process
    connect(ekos->focusModule(), &Ekos::Focus::newStatus, this,
            &TestEkosCaptureWorkflow::focusStatusChanged, Qt::UniqueConnection);

    // preparation successful
    return true;
}

void TestEkosCaptureWorkflow::init() {
    // reset counters
    image_count  = 0;
    // clear image directory
    QVERIFY(m_CaptureHelper->getImageLocation()->removeRecursively());
}

void TestEkosCaptureWorkflow::cleanup() {
    Ekos::Manager::Instance()->focusModule()->abort();
    Ekos::Manager::Instance()->captureModule()->abort();
}


bool TestEkosCaptureWorkflow::prepareCapture()
{
    QFETCH(double, exptime);
    QFETCH(QString, sequence);
    // test data
    // switch to capture module
    Ekos::Capture *capture = Ekos::Manager::Instance()->captureModule();
    KWRAP_SUB(KTRY_SWITCH_TO_MODULE_WITH_TIMEOUT(capture, 1000));

    // add target to path to emulate the behavior of the scheduler
    QString imagepath = getImageLocation()->path() + "/test";

    // create the destination for images
    qCInfo(KSTARS_EKOS_TEST) << "FITS path: " << imagepath;

    // create capture sequences
    KTRY_SET_CHECKBOX_SUB(capture, limitRefocusS, true);
    KTRY_SET_SPINBOX_SUB(capture, limitRefocusN, 1);
    KVERIFY_SUB(m_CaptureHelper->fillCaptureSequences(target, sequence, exptime, imagepath));

    // everything successfully completed
    return true;
}

void TestEkosCaptureWorkflow::prepareTestData(double exptime, QString sequence)
{
    QTest::addColumn<double>("exptime");             /*!< exposure time */
    QTest::addColumn<QString>("sequence");           /*!< list of filters */

    QTest::newRow(QString("seq=%1").arg(sequence).toStdString().c_str()) << exptime << sequence;
}

QDir *TestEkosCaptureWorkflow::getImageLocation()
{
    if (imageLocation == nullptr || imageLocation->exists())
        imageLocation = new QDir(destination->path() + "/images");

    return imageLocation;
}

/* *********************************************************************************
 *
 * Slots for catching state changes
 *
 * ********************************************************************************* */

void TestEkosCaptureWorkflow::telescopeStatusChanged(ISD::Telescope::Status status)
{
    m_TelescopeStatus = status;
    // check if the new state is the next one expected, then remove it from the stack
    if (!expectedTelescopeStates.isEmpty() && expectedTelescopeStates.head() == status)
        expectedTelescopeStates.dequeue();
}

void TestEkosCaptureWorkflow::captureStatusChanged(Ekos::CaptureState status)
{
    m_CaptureStatus = status;
    // check if the new state is the next one expected, then remove it from the stack
    if (!expectedCaptureStates.isEmpty() && expectedCaptureStates.head() == status)
        expectedCaptureStates.dequeue();
}

void TestEkosCaptureWorkflow::focusStatusChanged(Ekos::FocusState status)
{
    m_FocusStatus = status;
    // check if the new state is the next one expected, then remove it from the stack
    if (!expectedFocusStates.isEmpty() && expectedFocusStates.head() == status)
        expectedFocusStates.dequeue();
}

void TestEkosCaptureWorkflow::imageReceived(FITSView *view)
{
    Q_UNUSED(view);
    captureStatusChanged(Ekos::CAPTURE_COMPLETE);
    image_count++;
}

/* *********************************************************************************
 *
 * Main function
 *
 * ********************************************************************************* */

QTEST_KSTARS_MAIN(TestEkosCaptureWorkflow)
