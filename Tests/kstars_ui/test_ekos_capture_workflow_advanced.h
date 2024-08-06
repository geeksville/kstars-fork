/*
    KStars UI tests for alignment

    SPDX-FileCopyrightText: 2021 Wolfgang Reissenberger <sterne-jaeger@openfuture.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "config-kstars.h"
#include "test_ekos.h"
#include "test_ekos_capture_workflow.h"

#if defined(HAVE_INDI)

#include <QObject>


class TestEkosCaptureWorkflowAdvanced : public QObject
{
    Q_OBJECT

public:
    explicit TestEkosCaptureWorkflowAdvanced(QObject *parent = nullptr);
    explicit TestEkosCaptureWorkflowAdvanced(QString guider, QObject *parent = nullptr);

protected slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

private:
    // helper class
    TestEkosCaptureHelper *m_CaptureHelper = nullptr;


private slots:
    /**
      * @brief Test if capture continues where it had been suspended by a
      * guiding deviation as soon as guiding is back below the deviation threshold
      */
    void testGuidingDeviationSuspendingCapture();

    /**
      * @brief Test if aborting a job suspended due to a guiding deviation
      * remains aborted when the guiding deviation is below the configured threshold.
      */
    void testGuidingDeviationAbortCapture();

    /**
      * @brief Test if a guiding deviation beyond the configured limit blocks the start of
      * capturing until the guiding deviation is below the configured deviation threshold.
      */
    void testInitialGuidingLimitCapture();

    /** @brief Test data for @see testInitialGuidingLimitCapture() */
    void testInitialGuidingLimitCapture_data();

    /**
      * @brief Check mount and dome parking before capturing flats.
      */
    //void testPreMountAndDomePark();

    /** @brief Test data for {@see testFlatPreMountAndDomePark()} */
    //void testPreMountAndDomePark_data();
};

#endif // HAVE_INDI
