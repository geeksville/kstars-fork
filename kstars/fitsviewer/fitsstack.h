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
#include "opencv2/calib3d.hpp"
#include "opencv2/xphoto.hpp"

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

        bool addImage(void *imageBuffer, int cvType, double pixscale, int width, int height, int bytesPerPixel);

        // Plate solving done so update
        bool solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success, const double hfr, const int numStars);

        // Stack the images and return the stack
        bool stack();
        bool stackn();

        void addMaster(bool dark, void *imageBuffer, int width, int height, int bytesPerPixel);

        void redoPostProcessStack();

        const bool &getStackInProgress() const
        {
            return m_StackInProgress;
        }

        void setInitalStackDone(bool done);

        const bool &getInitialStackDone() const
        {
            return m_InitialStackDone;
        }

        void setStackInProgress(bool inProgress);

        const LiveStackData &getStackData() const
        {
            return m_StackData;
        }

        const QByteArray &getStackedImage() const
        {
            return m_StackedBuffer;
        }

        //bool loadData(const QSharedPointer<FITSData> &data, FITSMode mode = FITS_NORMAL, FITSScale filter = FITS_NONE);

    signals:
        void stackChanged();

    public slots:
    private:      
        typedef enum
        {
            SOLVED_IN_PROGRESS,
            SOLVED_FAILED,
            SOLVED_OK
        } PlateSolveStatus;

        // Get the current user stack options
        LiveStackData loadStackData();
        LiveStackPPData loadStackPPData();

        // Used for solving an image.
        void solveImage(QList<SSolver::Parameters> parameters, double pixscale, int width, int height, int bytesPerPixel);
        void setupSolver(bool extractOnly = false);
        bool readyToStack();
        cv::Mat calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2);
        bool convertMatToFITS(const cv::Mat image, QByteArray &fitsBuffer);
        bool calibrateSub(cv::Mat & sub);
        cv::Mat stackSubs(const QVector<cv::Mat> &subs, const bool initial, float &totalWeight);
        cv::Mat stackImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights);
        cv::Mat stacknImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights);
        cv::Mat postProcessImage(const cv::Mat image);
        QVector<float> getWeights();
        void setupRunningStack(struct wcsprm * wcsprm, const int numSubs, const float totalWeight);
        void updateRunningStack(const int numSubs, const float totalWeight);
        void tidyUpInitalStack(struct wcsprm * refWCS);
        void tidyUpRunningStack();

        //void extractorDone(bool timedOut, bool success, const FITSImage::Solution &solution, double elapsedSeconds);
        //void initSolverUI();
        //void setupProfiles(int profileIndex);
        //int getProfileIndex(int moduleIndex);
        //void setProfileIndex(int moduleIndex, int profileIndex);

        FITSData *m_Data;
        QSharedPointer<SolverUtils> m_Solver;
        bool m_ReadyToStack { false };

        // QVector<QString> m_ImagePaths;
        //QVector<cv::Mat> m_Images;
        //QVector<PlateSolveStatus> m_PlateSolvedStatus;
        //QVector<struct wcsprm *> m_Wcsprm;
        //QVector<double> m_HFR;
        //QVector<int> m_NumStars;
        typedef struct
        {
            cv::Mat image;
            PlateSolveStatus plateSolvedStatus;
            struct wcsprm * wcsprm;
            double hfr;
            int numStars;
        } StackImageData;
        QVector<StackImageData> m_StackImageData;

        typedef struct
        {
            int numSubs;
            struct wcsprm * ref_wcsprm;
            double ref_hfr;
            int ref_numStars;
            float totalWeight;
        } RunningStackImageData;
        RunningStackImageData m_RunningStackImageData { 0, nullptr, -1.0, 0, 0.0};

        // Stack status
        bool m_StackInProgress { false };
        bool m_InitialStackDone { false };

        // Stack data user options
        LiveStackData m_StackData;

        // Calibration
        cv::Mat m_MasterDark;
        cv::Mat m_MasterFlat;

        // Aligning
        // Stacking
        cv::Mat m_StackedImage32F;
        cv::Mat m_SigmaClip32FC4;
        QByteArray m_StackedBuffer;
        float m_Width { 0.0f };
        float m_Height { 0.0f };
};
