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
 * @brief The FITSStack class for Live Stacking within the Fitsviewer
 * @author John Evans
 */
class FITSStack : public QObject
{
        Q_OBJECT

    public:
        explicit FITSStack(FITSData *parent);
        virtual ~FITSStack() override;

        /**
         * @brief Prepare FITSStack for the next image sub. Call before addSub.
         */
        void setupNextSub();

        /**
         * @brief add an image sub to the stack.
         * @param imageBuffer is the in-memory buffer
         * @param cvType is the openCV Mat type
         * @param width is image width
         * @param height is image height
         * @param bytesPerPixel
         * @return success
         */
        bool addSub(void *imageBuffer, const int cvType, const int width, const int height, const int bytesPerPixel);

        /**
         * @brief add a master dark or flat.
         * @param dark (or flat)
         * @param image buffer
         * @param width is master width
         * @param height is master height
         * @param bytesPerPixel
         * @param cvType
         */
        void addMaster(const bool dark, void *imageBuffer, const int width, const int height, const int bytesPerPixel, const int cvType);

        /**
         * @brief solverDone called when plate solving completes, so start next action.
         * @param wcsHandle of the solved image
         * @param timedOut (or not)
         * @param success (or not)
         * @param median HFR of stars
         * @param number of stars
         * @return success
         */
        bool solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success, const double hfr, const int numStars);

        /**
         * @brief Perform admin within FITSStack for case where we couldn't add an image.
         */
        void addSubFailed();

        /**
         * @brief Perform an initial stack
         */
        bool stack();

        /**
         * @brief Perform an incremental stack (add new subs to an existing stack)
         */
        bool stackn();

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

        QByteArray getStackedImage() const
        {
            return (m_StackedBuffer) ? *m_StackedBuffer : QByteArray();
        }

        const double &getMeanSubSNR() const
        {
            return m_MeanSubSNR;
        }

        const double &getMinSubSNR() const
        {
            return m_MinSubSNR;
        }

        const double &getMaxSubSNR() const
        {
            return m_MaxSubSNR;
        }

        const double &getStackSNR() const
        {
            return m_StackSNR;
        }

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
        bool checkSub(const int width, const int height, const int bytesPerPixel, const int channels);
        void solveImage(QList<SSolver::Parameters> parameters, double pixscale, int width, int height, int bytesPerPixel);
        void setupSolver(bool extractOnly = false);

        /**
         * @brief Calculate the warp matrix to warp image 2 to image 1 (reference)
         * @param wcs1 WCS structure for image 1 (reference)
         * @param wcs2 WCS structure for image 2
         * @param warp matrix
         * @return success (or not)
         */
        bool calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2, cv::Mat &warp);
        bool convertMatToFITS(const cv::Mat image);

        /**
         * @brief Calibrate the passed in sub
         * @param sub to be calibrated
         * @return success (or not)
         */
        bool calibrateSub(cv::Mat &sub);

        /**
         * @brief Stack the passed in vector of subs
         * @param subs to be stacked
         * @param initial stack (or incremental)
         * @param totalWeight is the weight of the current stack if this is an incremental stack
         * @param stack is returned to the caller
         * @return success (or not)
         */
        bool stackSubs(const QVector<cv::Mat> &subs, const bool initial, float &totalWeight, cv::Mat &stack);

        cv::Mat stackImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights);
        cv::Mat stacknImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights);
        cv::Mat postProcessImage(const cv::Mat image);
        QVector<float> getWeights();
        double getSNR(const cv::Mat image);
        cv::Mat calculatePSF(const cv::Mat &image, int patchSize = 21);
        cv::Mat wienerDeconvolution(const cv::Mat &image, const cv::Mat &psf);
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
        RunningStackImageData m_RunningStackImageData { 0, nullptr, -1.0, 0, 0.0 };

        // SNR of subs
        double m_MeanSubSNR { 0 };
        double m_MinSubSNR { 0 };
        double m_MaxSubSNR { 0 };

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
        // JEE QByteArray m_StackedBuffer;
        std::unique_ptr<QByteArray> m_StackedBuffer { nullptr };

        double m_StackSNR { 0.0 };
        float m_Width { 0.0f };
        float m_Height { 0.0f };
        int m_Channels { 0 };
        int m_BytesPerPixel { 0 };
        int m_CVType { 0 };
};
