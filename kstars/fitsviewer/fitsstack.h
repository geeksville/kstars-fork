/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "fitscommon.h"
#include "ekos/auxiliary/solverutils.h"

#include <QObject>
#include <QPointer>

// JEE
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/video/tracking.hpp"
#include <opencv2/calib3d.hpp>

// JEE Need to change as won't compile if no WCS
#ifdef HAVE_WCSLIB
#include <wcs.h>
#endif

namespace Ekos
{
class StellarSolverProfileEditor;
}

/**
 * @brief The FITSStack holds routines for Live Stacking within the Fitsviewer
 * @author John Evans
 */
class FITSStack : public QObject
{
        Q_OBJECT

    public:
        explicit FITSStack(FITSData *parent);
        virtual ~FITSStack() override;

        // JEE
        bool addImage(void *imageBuffer, double pixscale, int width, int height, int bytesPerPixel);

        // Plate solving done so update
        bool solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success);

        // Stack the images and return the stack
        bool stack();

        bool addMaster(bool dark, void *imageBuffer, int width, int height, int bytesPerPixel);

        QByteArray const &getStackedImage() const
        {
            return m_StackedImage;
        }

        //bool loadData(const QSharedPointer<FITSData> &data, FITSMode mode = FITS_NORMAL, FITSScale filter = FITS_NONE);

    public slots:
    protected:
    private:
        // Used for solving an image.
        void solveImage(QList<SSolver::Parameters> parameters, double pixscale, int width, int height, int bytesPerPixel);
        void setupSolver(bool extractOnly = false);
        bool readyToStack();
        cv::Mat calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2);
        bool convertMatToFITS(const cv::Mat image, QByteArray &fitsBuffer);
        bool calibrateSub(cv::Mat & sub);
        cv::Mat stackImagesSigmaClipping(const QVector<cv::Mat> &images, bool windsor);
        double winsorizedSigmaClipping(std::vector<double> data, double sigmaLow, double sigmaHigh, double winsorizePercentile);

        //void extractorDone(bool timedOut, bool success, const FITSImage::Solution &solution, double elapsedSeconds);
        //void initSolverUI();
        void setupProfiles(int profileIndex);
        int getProfileIndex(int moduleIndex);
        void setProfileIndex(int moduleIndex, int profileIndex);

        FITSData *m_Data;
        QSharedPointer<SolverUtils> m_Solver;
        bool m_ReadyToStack { false };

        QByteArray m_StackedImage;
        float m_Width { 0.0f };
        float m_Height { 0.0f };

        typedef enum
        {
            SOLVED_IN_PROGRESS,
            SOLVED_FAILED,
            SOLVED_OK
        } PlateSolveStatus;
        QVector<QString> m_ImagePaths;
        QVector<cv::Mat> m_Images;
        QVector<PlateSolveStatus> m_PlateSolvedStatus;
        QVector<struct wcsprm *> m_Wcsprm;
        cv::Mat m_MasterDark;
        cv::Mat m_MasterFlat;
        float m_MedianMasterFlat { 0.0 };
};
