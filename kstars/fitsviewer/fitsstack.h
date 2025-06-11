/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "fitscommon.h"
#include "ekos/auxiliary/solverutils.h"

#include <QObject>
#include <QPointer>

#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xphoto.hpp"

#include <wcs.h>

namespace Ekos
{
class StellarSolverProfileEditor;
}

/**
 * @brief The FITSStack class for Live Stacking within the Fitsviewer
 *        The functionality is as follows:
 *        1. A directory is selected and any subs already in the directory are stacked
 *          a) Each sub may be calibrated with a dark and / or a flat
 *          b) A reference frame is chosen as master for alignment.
 *          c) All subs are aligned to the master by plate solving each subs and using
 *             WCS for alignment.
 *          d) Subs are stacked. Modes are basic addition of pixel values or more complex
 *             statistical processing like sigma clipping or winsorized sigma clipping
 *          e) Basic post-processing options such as sharpening and denoising are offered
 *        2. The stack is displayed in the FITSViewer.
 *        3. As new subs are added to the directory these are detected by FitsDirWatcher
 *        4. The new sub(s) are added to the already existing stack.
 *        5. The new stack is post-processed and displayed.
 *
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

        /**
         * @brief Redo post-processing on the stack
         */
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

        void setBayerPattern(QString bayerPattern);

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

        /**
         * @brief Load a structure of user options from Options
         * @return user options
         */
        LiveStackData loadStackData();

        /**
         * @brief Load a structure of post-processing user options from Options
         * @return user options
         */
        LiveStackPPData loadStackPPData();

        /**
         * @brief Check that a new image is consistent with previous images in size, datatype, etc
         * @return success (or not)
         */
        bool checkSub(const int width, const int height, const int bytesPerPixel, const int channels);

        /**
         * @brief Calculate the warp matrix to warp image 2 to image 1 (reference)
         * @param wcs1 WCS structure for image 1 (reference)
         * @param wcs2 WCS structure for image 2
         * @param warp matrix
         * @return success (or not)
         */
        bool calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2, cv::Mat &warp);

        /**
         * @brief Convert passed in Mat to FITS format
         * @param image
         * @return success (or not)
         */
        bool convertMatToFITS(const cv::Mat &image);

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

        /**
         * @brief Stack the passed in vector of subs using Sigma Clipping
         * @param subs to be stacked
         * @param weights of each sub for the stack
         * @return stack is returned to the caller
         */
        cv::Mat stackSubsSigmaClipping(const QVector<cv::Mat> &subs, const QVector<float> &weights);

        /**
         * @brief Stack the passed in vector of subs to an existing stack using Sigma Clipping
         * @param subs to be stacked
         * @param weights of each sub for the stack
         * @return stack
         */
        cv::Mat stacknSubsSigmaClipping(const QVector<cv::Mat> &subs, const QVector<float> &weights);

        /**
         * @brief Post process the passed in stack
         * @param Image to process
         * @return Processed image
         */
        cv::Mat postProcessImage(const cv::Mat &image);

        /**
         * @brief Return the weights for each sub for the stacking process
         * @return weights
         */
        QVector<float> getWeights();

        /**
         * @brief Return the Signal-To-Noise ratio of the passed in image
         * @param image
         * @return SNR (0.0 on failure)
         */
        double getSNR(const cv::Mat &image);

        /**
         * @brief Return a PSF for stars (upto 20) in the passed in image
         * @param image
         * @param patchSize is the patch region around a star - multiple stars are ignored. Must be odd
         * @return PSF (empty Mat on failure)
         */
        cv::Mat calculatePSF(const cv::Mat &image, int patchSize = 21);

        /**
         * @brief Apply Wiener deconvolution to the passed in image, using psf
         * @param image
         * @param psf (previously calculated by calculatePSF)
         * @return deconvolved image (empty Mat on failure)
         */
        cv::Mat wienerDeconvolution(const cv::Mat &image, const cv::Mat &psf);

        /**
         * @brief Called to transition initial stack data to running stack
         * @param Reference frame WCS
         * @param numSubs in the initial stack
         * @param totalWeight of subs processed so far
         */
        void setupRunningStack(struct wcsprm * refWCS, const int numSubs, const float totalWeight);

        /**
         * @brief Called to update running stack data
         * @param numSubs processed so far
         * @param totalWeight of subs processed so far
         */
        void updateRunningStack(const int numSubs, const float totalWeight);

        /**
         * @brief Tidy up initial stack data, e.g. free heap
         * @param refWCS
         */
        void tidyUpInitalStack(struct wcsprm * refWCS);

        /**
         * @brief Tidy up running stack data, e.g. free heap
         */
        void tidyUpRunningStack();

        FITSData *m_Data;
        QSharedPointer<SolverUtils> m_Solver;
        bool m_ReadyToStack { false };
        QString m_BayerPattern;

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
        QVector<cv::Mat> m_SigmaClip32FC4;
        QSharedPointer<QByteArray> m_StackedBuffer { nullptr };

        double m_StackSNR { 0.0 };
        float m_Width { 0.0f };
        float m_Height { 0.0f };
        int m_Channels { 0 };
        int m_BytesPerPixel { 0 };
        int m_CVType { 0 };
};
