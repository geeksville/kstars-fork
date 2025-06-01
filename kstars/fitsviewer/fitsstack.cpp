/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fitsstack.h"
#include "fitsdata.h"
#include <fits_debug.h>
#include "fitscommon.h"
#include "fitsstardetector.h"
#include "ekos/auxiliary/stellarsolverprofile.h"
#include "ekos/auxiliary/stellarsolverprofileeditor.h"
#include "ekos/auxiliary/solverutils.h"
#include "auxiliary/kspaths.h"
#include "kstars.h"
#include "Options.h"
#include "../auxiliary/robuststatistics.h"
#include "../auxiliary/gslhelpers.h"

#if !defined(KSTARS_LITE) && defined(HAVE_WCSLIB)
#include <wcshdr.h>
#endif

#include <fitsio.h>

// JEE class FITSData;

FITSStack::FITSStack(FITSData *parent) : QObject()
{
    m_Data = parent;
    m_StackData = loadStackData();
}

FITSStack::~FITSStack()
{
    tidyUpInitalStack(nullptr);
    tidyUpRunningStack();
}

void FITSStack::setStackInProgress(bool inProgress)
{
    m_StackInProgress = inProgress;
}

void FITSStack::setInitalStackDone(bool done)
{
    m_InitialStackDone = done;
}

LiveStackData FITSStack::loadStackData()
{
    LiveStackData data;
    data.masterDark = Options::fitsLSMasterDark();
    data.masterFlat = Options::fitsLSMasterFlat();
    data.alignMaster = Options::fitsLSAlignMaster();
    data.alignMethod = static_cast<LiveStackAlignMethod>(Options::fitsLSAlignMethod());
    data.numInMem = Options::fitsLSNumInMem();
    data.weighting = static_cast<LiveStackFrameWeighting>(Options::fitsLSWeighting());
    data.rejection = static_cast<LiveStackRejection>(Options::fitsLSRejection());
    data.lowSigma = Options::fitsLSLowSigma();
    data.highSigma = Options::fitsLSHighSigma();
    data.windzorCutoff = Options::fitsLSWinsorCutoff();
    data.postProcessing = loadStackPPData();
    return data;
}

LiveStackPPData FITSStack::loadStackPPData()
{
    LiveStackPPData data;
    data.deconvolution = Options::fitsLSDeconvolution();
    data.denoiseAmt = Options::fitsLSDenoiseAmt();
    data.sharpenAmt = Options::fitsLSSharpenAmt();
    data.sharpenKernal = Options::fitsLSSharpenKernal();
    data.sharpenSigma = Options::fitsLSSharpenSigma();
    return data;
}

// JEE
// #ifdef HAVE_OPENCV

// Setup the image data structure for later processing
void FITSStack::setupNextSub()
{
    StackImageData imageData;
    imageData.image = cv::Mat();
    imageData.plateSolvedStatus = SOLVED_IN_PROGRESS;
    imageData.wcsprm = nullptr;
    imageData.hfr = -1;
    imageData.numStars = 0;
    m_StackImageData.push_back(imageData);
}

bool FITSStack::addSub(void * imageBuffer, const int cvType, const int width, const int height, const int bytesPerPixel)
{
    try
    {
        int channels = CV_MAT_CN(cvType);
        // Check the image is the correct shape
        if (!checkSub(width, height, bytesPerPixel, channels))
            return false;

        size_t rowLen = width * bytesPerPixel * channels;
        cv::Mat image = cv::Mat(height, width, cvType, imageBuffer, rowLen);
        if (image.empty())
        {
            qCDebug(KSTARS_FITS) << QString("%1 Unable to process image in openCV").arg(__FUNCTION__);
            return false;
        }

        // Convert the Mat to float type for upcoming calcs
        image.convertTo(image, m_CVType);

        double snr = getSNR(image);
        if (snr > 0.0)
        {
            m_MaxSubSNR = std::max(m_MaxSubSNR, snr);
            m_MinSubSNR = (m_MinSubSNR > 0.0) ? std::min(m_MinSubSNR, snr) : snr;
            int subs = m_StackImageData.size();
            if (getInitialStackDone())
                subs += m_RunningStackImageData.numSubs;
            m_MeanSubSNR = (m_MeanSubSNR * subs + snr) / (subs + 1);
        }

        // Setup the image data structure for later processing
        // JEE StackImageData imageData;
        //imageData.image = image.clone();
        //imageData.plateSolvedStatus = SOLVED_IN_PROGRESS;
        //imageData.wcsprm = nullptr;
        //imageData.hfr = -1;
        //imageData.numStars = 0;
        //m_StackImageData.push_back(imageData);

        // Save the image for later processing
        // JEE m_StackImageData.last().image = image.clone();
        m_StackImageData.last().image = image;
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return false;
}

void FITSStack::addMaster(const bool dark, void * imageBuffer, const int width, const int height,
                          const int bytesPerPixel, const int cvType)
{
    try
    {
        if (dark)
            m_MasterDark.release();
        else
            m_MasterFlat.release();

        int channels = CV_MAT_CN(cvType);

        // Check the image is the correct shape
        if (!checkSub(width, height, bytesPerPixel, channels))
            return;

        size_t rowLen = width * bytesPerPixel * channels;
        cv::Mat image = cv::Mat(height, width, cvType, imageBuffer, rowLen);

        // Take a deep copy of the passed in image. Note this ensures the Mat is contiguous
        image.convertTo(image, m_CVType);
        cv::Mat imageClone = image.clone();

        if (dark)
            m_MasterDark = imageClone;
        else
        {
            m_MasterFlat = imageClone;

            // Scale the flat down using the median value
            std::vector<cv::Mat> channels;
            cv::split(m_MasterFlat, channels);

            for (int c = 0; c < channels.size(); c++)
            {
                std::vector<float> values;
                values.assign((float*)channels[c].data, (float*)channels[c].data + channels[c].total());

                float median = Mathematics::RobustStatistics::ComputeLocation(
                                                Mathematics::RobustStatistics::LOCATION_MEDIAN, values);

                if (median <= 0.0f)
                    qCDebug(KSTARS_FITS) << QString("%1 Unable to calculate median of Master flat channel %2")
                                                .arg(__FUNCTION__).arg(c);
                else
                {
                    channels[c] /= median;
                    // Make sure no zero or very small values that will later give problems when dividing by the flat
                    cv::max(channels[c], 0.1f, channels[c]);
                }
            }
            cv::merge(channels, m_MasterFlat);
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
}

// Check that the passed in sub or master is the same shape as the others
bool FITSStack::checkSub(const int width, const int height, const int bytesPerPixel, const int channels)
{
    try
    {
        if (m_Width == 0)
            m_Width = width;
        else if (m_Width != width)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Images have inconsistent widths").arg(__FUNCTION__);
            return false;
        }

        if (m_Height == 0)
            m_Height = height;
        else if (m_Height != height)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Images have inconsistent heights").arg(__FUNCTION__);
            return false;
        }

        if (m_Channels == 0)
            m_Channels = channels;
        else if (m_Channels != channels)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Images have inconsistent channels").arg(__FUNCTION__);
            return false;
        }

        if (m_BytesPerPixel == 0)
            m_BytesPerPixel = bytesPerPixel;
        else if (m_BytesPerPixel != bytesPerPixel)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Images have inconsistent bytes per pixel").arg(__FUNCTION__);
            return false;
        }

        // Now setup the target CVTYPE for use in stacking calculations - use 32bit floating
        if (m_CVType == 0)
            m_CVType = CV_MAKETYPE(CV_32F, channels);
        else if (m_CVType != CV_MAKETYPE(CV_32F, channels))
        {
            qCDebug(KSTARS_FITS) << QString("%1 Images have inconsistent CVTypes").arg(__FUNCTION__);
            return false;
        }
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return false;
}

// Update plate solving status
bool FITSStack::solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success, const double hfr, const int numStars)
{
    if (m_StackImageData.size() <= 0)
    {
        // This shouldn't happen
        qCDebug(KSTARS_FITS) << "Solver done called but no m_StackImageData";
        return false;
    }

    if (timedOut || !success)
    {
        m_StackImageData.last().plateSolvedStatus = SOLVED_FAILED;
        return false;
    }

    m_StackImageData.last().plateSolvedStatus = SOLVED_OK;

    // Take a deep copy of the WCS state for alignment purposes
    struct wcsprm * wcsCopy = new struct wcsprm;
    wcsCopy->flag = -1; // Allocate space
    int status = 0;
    if ((status = wcssub(1, wcsHandle, 0x0, 0x0, wcsCopy)) != 0)
    {
        m_StackImageData.last().plateSolvedStatus = SOLVED_FAILED;
        qCDebug(KSTARS_FITS) << QString("wcssub error processing %1 %2").arg(status).arg(wcs_errmsg[status]);
        return false;
    }
    else if ((status = wcsset(wcsCopy)) != 0)
    {
        m_StackImageData.last().plateSolvedStatus = SOLVED_FAILED;
        qCDebug(KSTARS_FITS) << QString("wcsset error processing %1 %2").arg(status).arg(wcs_errmsg[status]);
        return false;
    }

    m_StackImageData.last().wcsprm = wcsCopy;
    m_StackImageData.last().hfr = hfr;
    m_StackImageData.last().numStars = numStars;
    return true;
}

// Couldn't add an image to be stacked for some reason so complete the admin needed
void FITSStack::addSubFailed()
{
    if (m_StackImageData.size() <= 0)
    {
        // This shouldn't happen
        qCDebug(KSTARS_FITS) << "addSubFailed called but no m_StackImageData";
        return;
    }

    m_StackImageData.last().plateSolvedStatus = SOLVED_FAILED;
}

// Perform the initial stack
bool FITSStack::stack()
{
    try
    {
        int ref = -1;
        cv::Mat finalImage;
        QVector<cv::Mat> m_Aligned;
        for(int i = 0; i < m_StackImageData.size(); i++)
        {
            if (m_StackImageData[i].plateSolvedStatus != SOLVED_OK)
                continue;

            // JEE
            //cv::Mat image32F;
            //m_StackImageData[i].image.convertTo(image32F, m_CVType);

            // Calibrate sub
            if (!calibrateSub(m_StackImageData[i].image))
                continue;

            if (ref < 0)
            {
                // First image is the reference to which others are aligned
                ref = i;
                // JEE m_Aligned.push_back(m_StackImageData[i].image.clone());
                m_Aligned.push_back(m_StackImageData[i].image);
            }
            else
            {
                // Align this image to the reference image
                cv::Mat warp, warpedImage;
                if (calcWarpMatrix(m_StackImageData[ref].wcsprm, m_StackImageData[i].wcsprm, warp))
                {
                    cv::warpPerspective(m_StackImageData[i].image, warpedImage, warp, m_StackImageData[i].image.size(), cv::INTER_LANCZOS4);
                    m_Aligned.push_back(warpedImage);
                }
            }
        }
        // Stack the aligned subs
        float totalWeight = 0.0;
        if (stackSubs(m_Aligned, true, totalWeight, m_StackedImage32F))
        {
            // Perform any post stacking processing such as sharpening / denoising
            finalImage = postProcessImage(m_StackedImage32F);
            m_StackSNR = getSNR(finalImage);
            convertMatToFITS(finalImage);
        }

        if (m_StackData.numInMem <= m_StackImageData.size())
            // We've completed the initial stack so move to incremental stacking as new subs arrive
            setupRunningStack(m_StackImageData[ref].wcsprm, m_Aligned.size(), totalWeight);

        setStackInProgress(false);
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        setStackInProgress(false);
        return false;
    }
}

// Add 'n' new images to pre-existing stack
bool FITSStack::stackn()
{
    try
    {
        cv::Mat finalImage, finalImage32F;
        QVector<cv::Mat> m_Aligned;
        for(int i = 0; i < m_StackImageData.size(); i++)
        {
            if (m_StackImageData[i].plateSolvedStatus != SOLVED_OK)
                continue;

            // Calibrate sub
            if (!calibrateSub(m_StackImageData[i].image))
                continue;

            // Align this image to the reference image
            cv::Mat warp, warpedImage, warpedImage32F;
            if (calcWarpMatrix(m_RunningStackImageData.ref_wcsprm, m_StackImageData[i].wcsprm, warp))
            {
                cv::warpPerspective(m_StackImageData[i].image, warpedImage, warp, m_StackImageData[i].image.size(), cv::INTER_LANCZOS4);
                warpedImage.convertTo(warpedImage32F, CV_MAKETYPE(CV_32F, m_Channels));
                m_Aligned.push_back(warpedImage32F.clone());
                qCDebug(KSTARS_FITS) << QString("JEE m_Aligned %1 %2").arg(m_Aligned.last().depth()).arg(__FUNCTION__);
            }
        }
        // Stack the aligned subs
        float totalWeight = m_RunningStackImageData.totalWeight;
        if (stackSubs(m_Aligned, false, totalWeight, m_StackedImage32F))
        {
            // Perform any post stacking processing such as sharpening / denoising
            finalImage = postProcessImage(m_StackedImage32F);
            m_StackSNR = getSNR(finalImage);
            convertMatToFITS(finalImage);
        }
        updateRunningStack(m_Aligned.size(), totalWeight);
        setStackInProgress(false);
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        setStackInProgress(false);
        return false;
    }
    return true;
}

// Calculate the warp matrix to align image2 to image1
bool FITSStack::calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2, cv::Mat &warp)
{
    try
    {
        double X = m_Width - 1.0;
        double Y = m_Height - 1.0;

        // Define corners and centre of image 1 in pixels
        std::vector<cv::Point2d> corners1;
        corners1.push_back(cv::Point2d(0.0, 0.0));
        corners1.push_back(cv::Point2d(X, 0.0));
        corners1.push_back(cv::Point2d(X, Y));
        corners1.push_back(cv::Point2d(0.0, Y));
        corners1.push_back(cv::Point2d(X / 2.0, Y / 2.0));

        // Convert pix points to world coordinates of image 1
        double imgcrd[2], phi, theta, world[2], pixcrd[2];
        int status, stat[2];
        std::vector<cv::Point2d> worldCoords1;
        for (int i = 0; i < corners1.size(); i++)
        {
            pixcrd[0] = corners1[i].x;
            pixcrd[1] = corners1[i].y;
            if ((status = wcsp2s(wcs1, 1, 2, pixcrd, imgcrd, &phi, &theta, world, stat)) != 0)
                qCDebug(KSTARS_FITS) << QString("WCS wcsp2s error %1: %2").arg(status).arg(wcs_errmsg[status]);
            worldCoords1.push_back(cv::Point2d(world[0], world[1]));
        }

        // Convert world coordinates to pixel coordinates in image 2
        std::vector<cv::Point2d> corners2;
        for (int i = 0; i < worldCoords1.size(); i++)
        {
            world[0] = worldCoords1[i].x;
            world[1] = worldCoords1[i].y;
            if ((status = wcss2p(wcs2, 1, 2, world, &phi, &theta, imgcrd, pixcrd, stat)) != 0)
                qCDebug(KSTARS_FITS) << QString("WCS wcss2p error %1: %2").arg(status).arg(wcs_errmsg[status]);
            corners2.push_back(cv::Point2d(pixcrd[0], pixcrd[1]));
        }

        // Compute the homography matrix using OpenCV to go from image 2 to image 1 (reference)
        warp = cv::findHomography(corners2, corners1, 0);
        // JEE warpMatrix = cv::findHomography(corners2, corners1, cv::RANSAC, 1.0);
        if (warp.empty())
        {
            qCDebug(KSTARS_FITS) << QString("openCV findHomography warp matrix empty");
            return false;
        }
        // JEE cv::Ptr<cv::Formatter> fmt = cv::Formatter::get(cv::Formatter::FMT_DEFAULT);
        // std::cout << fmt->format(warp) << std::endl;
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        return false;
    }
}

// Calibrate the passed in sub with an associated Dark (if available) and / or Flat (if available)
bool FITSStack::calibrateSub(cv::Mat &sub)
{
    try
    {
        if (sub.empty())
            return false;

        // Dark subtraction (make sure no negative pixels)
        if (!m_MasterDark.empty())
        {
            sub -= m_MasterDark;
            cv::max(sub, 0.0f, sub);
        }

        // Flat calibration
        if (!m_MasterFlat.empty())
            sub /= m_MasterFlat;
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return false;
}

// Stack the vector of subs
bool FITSStack::stackSubs(const QVector<cv::Mat> &subs, const bool initial, float &totalWeight, cv::Mat &stack)
{
    try
    {
        if (subs.size() <= 0)
            return false;

        QVector<float> weights = getWeights();

        if (m_StackData.rejection == LS_STACKING_REJ_SIGMA || m_StackData.rejection == LS_STACKING_REJ_WINDZOR)
        {
            if (initial)
                stack = stackImagesSigmaClipping(subs, weights);
            else
                stack = stacknImagesSigmaClipping(subs, weights);
        }
        else
        {
            // Add the pixels weighted per sub based on user setting. Then divide by the total weight
            // If its an initial stack then just use subs, if not then include the existing partial stack
            if (initial)
            {
                totalWeight = 0.0;
                stack = cv::Mat::zeros(subs[0].rows, subs[0].cols, m_CVType);
            }
            else
            {
                totalWeight = m_RunningStackImageData.totalWeight;
                stack = m_StackedImage32F * totalWeight;
            }

            for (int sub = 0; sub < subs.size(); sub++)
            {
                stack += subs[sub] * weights[sub];
                totalWeight += weights[sub];
            }
            stack /= totalWeight;
        }
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        return false;
    }
}

QVector<float> FITSStack::getWeights()
{
    QVector<float> weights(m_StackImageData.size());

    for (int i = 0; i < weights.size(); i++)
    {
        switch (m_StackData.weighting)
        {
            case LS_STACKING_EQUAL:
                weights[i] = 1.0;
                break;
            case LS_STACKING_HFR:
                if (m_StackImageData[i].hfr > 0.0)
                    weights[i] = 1.0 / m_StackImageData[i].hfr;
                else
                    weights[i] = 1.0;
                break;
            case LS_STACKING_NUM_STARS:
                if (m_StackImageData[i].numStars > 0)
                    weights[i] = m_StackImageData[i].numStars;
                else
                    weights[i] = 1.0;
                break;
            default:
                qCDebug(KSTARS_FITS) << QString("Error calculating weights in %1").arg(__FUNCTION__);
                weights[i] = 1.0;
        }
    }
    return weights;
}

// Function to stack images using standard or Windsorized Sigma Clipping
cv::Mat FITSStack::stackImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights)
{
    try
    {
        int rows = images[0].rows;
        int cols = images[0].cols;
        int numImages = images.size();
        cv::Mat finalImage = cv::Mat::zeros(rows, cols, CV_32F);
        float *finalImagePtr;
        // Setup structure for future sigma clipping
        m_SigmaClip32FC4 = cv::Mat::zeros(rows, cols, CV_32FC4);
        cv::Vec4f *sigmaClipPtr;

        bool continuous = finalImage.isContinuous() && m_SigmaClip32FC4.isContinuous();
        for (int i = 0; i < numImages; i++)
        {
            if (continuous)
                continuous = images[i].isContinuous();
            if (!continuous)
                break;
        }

        if (continuous)
        {
            // All images are continuous so we can treat as 1D arrays to speed things up
            cols *= rows;
            rows = 1;
        }

        // Pre-allocate vector for pixel values
        typedef struct
        {
            float value;
            float weight;
        } PixelValue;
        std::vector<PixelValue> pixelValues(numImages);
        std::vector<float> values(numImages);

        // Process each pixel position
        std::vector<const float *> imagesPtrs(numImages);
        for (int y = 0; y < rows; y++)
        {
            // Update imagePtrs and finalImagePtr for current y
            for (int i = 0; i < numImages; i++)
                imagesPtrs[i] = images[i].ptr<float>(y);

            finalImagePtr = finalImage.ptr<float>(y);
            sigmaClipPtr = m_SigmaClip32FC4.ptr<cv::Vec4f>(y);

            for (int x = 0; x < cols; x++)
            {
                // Collect values for this pixel from all images
                for (int image = 0; image < numImages; image++)
                {
                    pixelValues[image].value = imagesPtrs[image][x];
                    pixelValues[image].weight = weights[image];
                    values[image] = imagesPtrs[image][x];
                }

                std::sort(pixelValues.begin(), pixelValues.end(), [](const PixelValue &a, const PixelValue &b)
                {
                    return (a.value < b.value);
                });

                // Process this pixel stack
                float pixelValue = 0.0;

                if (m_StackData.rejection == LS_STACKING_REJ_WINDZOR)
                {
                    // Winsorize the data
                    float median = Mathematics::RobustStatistics::ComputeLocation(
                                                Mathematics::RobustStatistics::LOCATION_MEDIAN, values);
                    auto const stddev = std::sqrt(Mathematics::RobustStatistics::ComputeScale(
                                                Mathematics::RobustStatistics::SCALE_VARIANCE, values));

                    float lower = std::max(0.0, median - (stddev * m_StackData.windzorCutoff));
                    float upper = median + (stddev * m_StackData.windzorCutoff);

                    for (int i = 0; i < pixelValues.size(); i++)
                    {
                        if (pixelValues[i].value < lower)
                        {
                            pixelValues[i].value = lower;
                            values[i] = lower;
                        }
                        else if (pixelValues[i].value > upper)
                        {
                            pixelValues[i].value = upper;
                            values[i] = upper;
                        }
                    }
                }

                // Now process the winsorized data
                float median = Mathematics::RobustStatistics::ComputeLocation(
                                                Mathematics::RobustStatistics::LOCATION_MEDIAN, values);
                if (pixelValues.size() <= 3)
                    // For small samples just use median
                    pixelValue = median;
                else
                {
                    // Sigma clipping
                    auto const stddev = std::sqrt(Mathematics::RobustStatistics::ComputeScale(
                                                Mathematics::RobustStatistics::SCALE_VARIANCE, values));

                    // Get the lower and upper bounds
                    float lower = std::max(0.0, median - (stddev * m_StackData.lowSigma));
                    float upper = median + (stddev * m_StackData.highSigma);

                    float sum = 0.0, weightSum = 0.0;
                    for (int i = 0; i < pixelValues.size(); i++)
                    {
                        if (pixelValues[i].value < lower)
                            continue;
                        if (pixelValues[i].value > upper)
                            break;
                        if (pixelValues[i].value > 0.0)
                        {
                            sum += pixelValues[i].value * pixelValues[i].weight;
                            weightSum += pixelValues[i].weight;
                        }
                    }

                    if (weightSum > 0.0)
                        pixelValue = sum / weightSum;

                    // Now store intermediate calcs from this process, necessary for processing new subs
                    cv::Vec4f sigmaClip;
                    sigmaClip[0] = lower;
                    sigmaClip[1] = upper;
                    sigmaClip[2] = sum;
                    sigmaClip[3] = weightSum;
                    sigmaClipPtr[x] = sigmaClip;
                }
                finalImagePtr[x] = pixelValue;
            }
        }
        return finalImage;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        // JEE is there a better way to do this?
        cv::Mat dummy;
        return dummy;
    }
}

// Function to stack n images to an existing stack using Sigma Clipping
cv::Mat FITSStack::stacknImagesSigmaClipping(const QVector<cv::Mat> &images, const QVector<float> weights)
{
    try
    {
        int rows = images[0].rows;
        int cols = images[0].cols;
        int numImages = images.size();
        cv::Mat finalImage = m_StackedImage32F;
        float *finalImagePtr;
        cv::Vec4f *sigmaClipPtr;

        if (images.size() != weights.size())
            return finalImage;

        bool continuous = finalImage.isContinuous() && m_SigmaClip32FC4.isContinuous();
        for (int i = 0; i < numImages; i++)
        {
            if (continuous)
                continuous = images[i].isContinuous();
            if (!continuous)
                break;
        }

        // If all images are continuous so we can treat as 1D arrays to speed things up
        if (continuous)
        {
            cols *= rows;
            rows = 1;
        }

        // Process each pixel position
        std::vector<const float *> imagesPtrs(numImages);
        for (int y = 0; y < rows; y++)
        {
            // Update pointers for current y
            for (int i = 0; i < numImages; i++)
                imagesPtrs[i] = images[i].ptr<float>(y);

            finalImagePtr = finalImage.ptr<float>(y);
            sigmaClipPtr = m_SigmaClip32FC4.ptr<cv::Vec4f>(y);

            for (int x = 0; x < cols; x++)
            {
                // Get the sigma clip data from the current stack
                cv::Vec4f sigmaClip = sigmaClipPtr[x];
                float lower = sigmaClip[0];
                float upper = sigmaClip[1];
                float sum = sigmaClip[2];
                float weightSum = sigmaClip[3];

                // Process each image
                for (int image = 0; image < numImages; image++)
                {
                    float pixel = imagesPtrs[image][x];
                    if (pixel >= lower && pixel <= upper)
                    {
                        sum += pixel * weights[image];
                        weightSum += weights[image];
                    }
                }

                // Update image pixel with new value
                finalImagePtr[x] = sum / weightSum;

                // Save the new intermediate results for next time
                sigmaClip[2] = sum;
                sigmaClip[3] = weightSum;
                sigmaClipPtr[x] = sigmaClip;
            }
        }
        return finalImage;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
        // JEE is there a better way to do this?
        cv::Mat dummy;
        return dummy;
    }
}

cv::Mat FITSStack::postProcessImage(cv::Mat image32F)
{
    cv::Mat finalImage;
    try
    {
        // Firstly perform deconvolution (if requested). Calculate psf from stars in the image then use this for deconvolution
        cv::Mat image;
        if (m_StackData.postProcessing.deconvolution)
        {
            cv::Mat greyImage32F, deconvolved;
            if (image32F.channels() == 1)
                greyImage32F = image32F;
            else
                cv::cvtColor(image32F, greyImage32F, cv::COLOR_BGR2GRAY);

            cv::Mat psf = calculatePSF(greyImage32F);
            if (!psf.empty())
            {
                deconvolved = wienerDeconvolution(greyImage32F, psf);
                if (!deconvolved.empty())
                    deconvolved.convertTo(image, CV_MAKETYPE(CV_16U, 1));
            }
        }

        if (image.empty())
            // Convert from 32F to 16U as following functions require 16U.
        {
            // First, find the range of the float data
            double minVal, maxVal;
            cv::minMaxLoc(image32F, &minVal, &maxVal);

            // Then scale to use full 16-bit range
            double scale = 65535.0 / maxVal;
            image32F.convertTo(image, CV_16U, scale);

            //image32F.convertTo(image, CV_MAKETYPE(CV_16U, 1));
        }

        cv::Mat sharpenedImage;

        // Sharpen using Unsharp Mask
        double sharpenAmount = m_StackData.postProcessing.sharpenAmt;
        if (sharpenAmount <= 0.0)
            sharpenedImage = image;
        else
        {
            cv::Mat blurredImage;
            int sharpenKernal = m_StackData.postProcessing.sharpenKernal;
            double sharpenSigma = m_StackData.postProcessing.sharpenSigma;
            cv::GaussianBlur(image, blurredImage, cv::Size(sharpenKernal, sharpenKernal), sharpenSigma);
            cv::addWeighted(image, 1.0 + sharpenAmount, blurredImage, -sharpenAmount, 0, sharpenedImage);
        }

        // Denoise
        double denoiseAmount = m_StackData.postProcessing.denoiseAmt;
        if (denoiseAmount <= 0.0)
            finalImage = sharpenedImage;
        else
        {
            std::vector<float> amount;
            amount.push_back(denoiseAmount);
            cv::fastNlMeansDenoising(sharpenedImage, finalImage, amount, 7, 21, cv::NORM_L1);
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return finalImage;
}

// Calculate psf for the passed in image from stars in the image
cv::Mat FITSStack::calculatePSF(const cv::Mat &image, int patchSize)
{
    cv::Mat psf;
    try
    {
        // JEE CV_Assert(patchSize % 2 == 1); // Must be odd
        //cv::Mat gray;
        //if (image.channels() == 3)
        //    cvtColor(image, gray, COLOR_BGR2GRAY);
        //else
        //    gray = image.clone();

        QList<Edge *> starCenters = m_Data->getStarCenters();

        QVector<cv::Mat> starPatches;
        int halfPatch = patchSize / 2;

        for (int i = 0; i < starCenters.size(); i++)
        {
            bool keepStar = true;

            // Ignore stars near the edge of the image
            float minx = starCenters[i]->x - halfPatch;
            float maxx = starCenters[i]->x + halfPatch;
            float miny = starCenters[i]->y - halfPatch;
            float maxy = starCenters[i]->y + halfPatch;

            if (minx < 0 || miny < 0 || maxx >= image.cols || maxy >= image.rows)
                continue;

            // Ignore stars near each other as they'll create a complicated PSF
            for (int j = 0; j < starCenters.size(); j++)
            {
                if (i == j)
                    continue;
                if (starCenters[j]->x >= minx && starCenters[j]->x <= maxx &&
                    starCenters[j]->y >= miny && starCenters[j]->y <= maxy)
                {
                    // Star j lies in star i's patch so ignore star i
                    keepStar = false;
                    break;
                }
            }

            if (keepStar)
            {
                cv::Rect roi(minx, miny, patchSize, patchSize);
                cv::Mat patch = image(roi).clone();
                // Normalise the patch so we're adding together stars of similar brightness
                patch /= cv::sum(patch);
                starPatches.push_back(patch);
            }

            // Limit the number of star patches
            if (starPatches.size() >= 20)
                break;
        }

        if (starPatches.empty())
            qCDebug(KSTARS_FITS) << QString("No valid stars for PSF estimation in %1").arg(__FUNCTION__);
        else
        {
            psf = cv::Mat::zeros(patchSize, patchSize, CV_32F);
            for (const auto &patch : starPatches)
                psf += patch;

            // Normalise PSF to unit energy
            psf /= cv::sum(psf);
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return psf;
}

// Wiener deconvolution assumes Gaussian noise and can be calculated using a single pass
// Lucy-Richardson deconvolution assumes Poisson noise and needs to be done iteratively.
// For now we'll try Wiener
cv::Mat FITSStack::wienerDeconvolution(const cv::Mat &image, const cv::Mat &psf)
{
    cv::Mat result;
    try
    {
        // Check inputs
        if (image.type() != CV_MAKETYPE(CV_32F, 1) || psf.type() != CV_MAKETYPE(CV_32F, 1))
            return result;

        //cv::Mat normImage;
        //cv::normalize(image, normImage, 0, 1, cv::NORM_MINMAX);

        // Pad the image to the optimum size for FFT
        cv::Mat imagePadded;
        int m = cv::getOptimalDFTSize(image.rows);
        int n = cv::getOptimalDFTSize(image.cols);
        cv::copyMakeBorder(image, imagePadded, 0, m - image.rows, 0, n - image.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
        std::cout << "ImagePadded: " << m << " x " << n << std::endl;

        // Centre the PSF in an image of the same size as imagePadded
        // Pad PSF to match input size (centered)
        //cv::Mat psfPadded;
        //int top = (imagePadded.rows - psf.rows) / 2;
        //int bottom = imagePadded.rows - psf.rows - top;
        //int left = (imagePadded.cols - psf.cols) / 2;
        //int right = imagePadded.cols - psf.cols - left;
        //cv::copyMakeBorder(psf, psfPadded, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar::all(0));

        // Centre the PSF in an image of the same size as imagePadded
        cv::Mat psfPadded = cv::Mat::zeros(imagePadded.size(), CV_32F);
        cv::Rect psfROI((psfPadded.cols - psf.cols) / 2, (psfPadded.rows - psf.rows) / 2, psf.cols, psf.rows);
        psf.copyTo(psfPadded(psfROI));

        // Move the PSF from the centre to the corners
        int cx = psfPadded.cols / 2;
        int cy = psfPadded.rows / 2;

        // Create quadrants
        cv::Mat q0(psfPadded, cv::Rect(0, 0, cx, cy)); // Top-Left
        cv::Mat q1(psfPadded, cv::Rect(cx, 0, cx, cy)); // Top-Right
        cv::Mat q2(psfPadded, cv::Rect(0, cy, cx, cy)); // Bottom-Left
        cv::Mat q3(psfPadded, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

        // Swap diagonally opposite quadrants
        cv::Mat tmp;
        q0.copyTo(tmp);
        q3.copyTo(q0);
        tmp.copyTo(q3);

        q1.copyTo(tmp);
        q2.copyTo(q1);
        tmp.copyTo(q2);

        // Take FFTs
        cv::Mat imgFFT, psfFFT;
        cv::dft(imagePadded, imgFFT, cv::DFT_COMPLEX_OUTPUT);
        double minVal, maxVal;
        cv::minMaxLoc(imgFFT, &minVal, &maxVal);
        std::cout << "Image FFT range: " << minVal << " to " << maxVal << std::endl;

        cv::dft(psfPadded, psfFFT, cv::DFT_COMPLEX_OUTPUT);
        cv::minMaxLoc(psfFFT, &minVal, &maxVal);
        std::cout << "psfPadded FFT range: " << minVal << " to " << maxVal << std::endl;

        // Conjugate multiplication (|psf|² = psf* · psf)
        cv::Mat psfPower;
        cv::mulSpectrums(psfFFT, psfFFT, psfPower, 0, true);
        cv::minMaxLoc(psfPower, &minVal, &maxVal);
        std::cout << "psfPower image range: " << minVal << " to " << maxVal << std::endl;

        // We only need the real channel of psfPower
        //cv::Mat psfPowerArray[2];
        //cv::split(psfPowerComp, psfPowerArray);

        // Get the noise
        // Flatten image into a single row for easy sorting
        cv::Mat sorted;
        image.reshape(0, 1).copyTo(sorted);
        cv::sort(sorted, sorted, cv::SORT_ASCENDING);

        // Median
        float median = sorted.at<float>(sorted.cols / 2);

        // Absolute deviation (MAD) from median
        cv::Mat absDiff;
        cv::absdiff(image, median, absDiff);
        absDiff = absDiff.reshape(0, 1);
        cv::sort(absDiff, absDiff, cv::SORT_ASCENDING);
        float mad = absDiff.at<float>(absDiff.cols / 2);

        // Variance of Gaussian noise
        float varNoise = std::pow((1.4826f * mad), 2.0);

        // Now get the Total Variance of the image
        cv::Scalar imageMean, imageStddev;
        cv::meanStdDev(image, imageMean, imageStddev);
        float varTotal = imageStddev[0] * imageStddev[0];

        // Calculate the signal variance (set a minimum amount)
        float varSignal = std::max((varTotal - varNoise), 1e-6f);

        // Noise to signal
        float NSR = varNoise / varSignal;

        // Wiener Filter
        cv::Mat numer;
        cv::mulSpectrums(imgFFT, psfFFT, numer, 0, true);
        cv::minMaxLoc(numer, &minVal, &maxVal);
        std::cout << "Numer image range: " << minVal << " to " << maxVal << std::endl;

        cv::Mat denom, denomComp[2];
        cv::split(psfPower, denomComp);
        denomComp[0] += NSR; // JEE + 1e-6f
        std::vector<cv::Mat> denomArray = { denomComp[0], denomComp[1] };
        cv::merge(denomArray, denom);

        cv::minMaxLoc(denomComp[0], &minVal, &maxVal);
        std::cout << "denomComp[0] image range: " << minVal << " to " << maxVal << std::endl;
        cv::minMaxLoc(denomComp[1], &minVal, &maxVal);
        std::cout << "denomComp[1] image range: " << minVal << " to " << maxVal << std::endl;
        cv::minMaxLoc(denom, &minVal, &maxVal);
        std::cout << "denom image range: " << minVal << " to " << maxVal << std::endl;

        // Safe division
        cv::Mat mask = denom < 1e-6;
        denom.setTo(1e-6, mask);

        cv::Mat wienerFFT;
        cv::divide(numer, denom, wienerFFT);
        cv::minMaxLoc(wienerFFT, &minVal, &maxVal);
        std::cout << "wienerFFT range: " << minVal << " to " << maxVal << std::endl;

        // Apply the Wiener filter


        cv::Mat resultFFT;
        cv::mulSpectrums(imagePadded, wienerFFT, resultFFT, 0);
        cv::minMaxLoc(resultFFT, &minVal, &maxVal);
        std::cout << "resultFFT range: " << minVal << " to " << maxVal << std::endl;

        // Take the inverse FFT
        cv::dft(resultFFT, result, cv::DFT_INVERSE | cv::DFT_REAL_OUTPUT | cv::DFT_SCALE);
        // JEE cv::normalize(result, result, 0, 255, cv::NORM_MINMAX);
        // result.convertTo(result, CV_8U);
        cv::minMaxLoc(result, &minVal, &maxVal);
        std::cout << "Deconvolved image range: " << minVal << " to " << maxVal << std::endl;

        cv::minMaxLoc(result, &minVal, &maxVal);
        std::cout << "result range: " << minVal << " to " << maxVal << std::endl;

        // Normalize to original image range
        cv::minMaxLoc(image, &minVal, &maxVal);
        result = result * (maxVal - minVal) + minVal;
        cv::minMaxLoc(result, &minVal, &maxVal);
        std::cout << "result range: " << minVal << " to " << maxVal << std::endl;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return result;
}

void FITSStack::redoPostProcessStack()
{
    // Get the current user options for post processing
    m_StackData.postProcessing = loadStackPPData();

    cv::Mat finalImage;
    finalImage = postProcessImage(m_StackedImage32F);
    m_StackSNR = getSNR(finalImage);
    convertMatToFITS(finalImage);
    emit stackChanged();
}

bool FITSStack::convertMatToFITS(const cv::Mat image)
{
    try
    {
        // Check if the image is valid
        if (image.empty() || image.depth() != CV_16U)
            return false;

        int width = image.size().width;
        int height = image.size().height;
        int channels = image.channels();

        //This section sets up the FITS File
        fitsfile *fptr = nullptr;
        int status = 0;
        long fpixel = 1, nelements;
        long naxis = (channels == 1) ? 2 : 3;
        long naxes[3] = { width, height, channels };
        //long naxis = 2;
        //long naxes[2] = { width, height };
        char error_status[512] = { 0 };
        void* fits_buffer = nullptr;
        size_t fits_buffer_size = 0;

        if (fits_create_memfile(&fptr, &fits_buffer, &fits_buffer_size, 4096, realloc, &status))
        {
            fits_get_errstatus(status, error_status);
            qCDebug(KSTARS_FITS()) << "fits_create_memfile failed " << error_status;
            return false;
        }

        if (fits_create_img(fptr, USHORT_IMG, naxis, naxes, &status))
        {
            fits_get_errstatus(status, error_status);
            qCDebug(KSTARS_FITS) << "fits_create_img failed " << error_status;
            status = 0;
            fits_close_file(fptr, &status);
            free(fits_buffer);
            return false;
        }

        if (channels == 3)
        {
            // Add BAYERPAT keyword here
            const char* bayerPattern = "RGGB";
            const char* comment = "Bayer color filter array pattern";

            if (fits_write_key(fptr, TSTRING, "BAYERPAT", (void*)bayerPattern, (char*)comment, &status))
            {
                fits_get_errstatus(status, error_status);
                qCDebug(KSTARS_FITS) << "fits_write_key BAYERPAT failed:" << error_status;
                // This is typically non-critical, so you might choose to continue rather than return false
                status = 0;  // Reset status to continue
            }
        }

        nelements = width * height * channels;

        //if (channels == 1)
        //{
            // Gray image
            cv::Mat contImage;
            if (image.isContinuous())
                contImage = image;
            else
                contImage = image.clone();

            if (fits_write_img(fptr, TUSHORT, fpixel, nelements, contImage.data, &status))
            {
                fits_get_errstatus(status, error_status);
                qCDebug(KSTARS_FITS) << "fits_write_img failed " << status;
                status = 0;
                fits_close_file(fptr, &status);
                free(fits_buffer);
                return false;
            }
        //}
        /*else
        {
            // Multi-channel - need to handle channel interleaving
            // OpenCV uses BGRBGR..., FITS uses planes: BBB...GGG...RRR...
            std::vector<cv::Mat> planes;
            cv::split(image, planes);

            uint16_t *bgrBuffer = new uint16_t[nelements];
            if (bgrBuffer == nullptr)
            {
                qCDebug(KSTARS_FITS) << "Not enough memory for RGB buffer";
                status = 0;
                fits_close_file(fptr, &status);
                free(fits_buffer);
                return false;
            }

            // Copy each channel to its plane in the buffer
            for (int c = 0; c < channels; c++)
            {
                cv::Mat contPlane;
                if (planes[c].isContinuous())
                    contPlane = planes[c];
                else
                    contPlane = planes[c].clone();

                const long planeElements = width * height; // Number of elements, not bytes
                std::memcpy(bgrBuffer + c * planeElements, contPlane.data, planeElements * sizeof(uint16_t));
            }

            if (fits_write_img(fptr, TUSHORT, fpixel, nelements, bgrBuffer, &status))
            {
                fits_get_errstatus(status, error_status);
                qCWarning(KSTARS_FITS) << "fits_write_img RGB failed:" << error_status;
                status = 0;
                fits_flush_file(fptr, &status);
                fits_close_file(fptr, &status);
                free(fits_buffer);
                delete [] bgrBuffer;
                return false;
            }

            delete [] bgrBuffer;
        }*/

        if (fits_flush_file(fptr, &status))
        {
            fits_get_errstatus(status, error_status);
            qCDebug(KSTARS_FITS) << "fits_flush_file failed:" << error_status;
            status = 0;
            fits_close_file(fptr, &status);
            free(fits_buffer);
            return false;
        }

        if (fits_close_file(fptr, &status))
        {
            fits_get_errstatus(status, error_status);
            qCDebug(KSTARS_FITS) << "fits_close_file failed:" << error_status;
            free(fits_buffer);
            return false;
        }

        // JEE fitsBuffer = QByteArray(reinterpret_cast<char *>(fits_buffer), fits_buffer_size);
        m_StackedBuffer.reset(new QByteArray(reinterpret_cast<char *>(fits_buffer), fits_buffer_size));
        free(fits_buffer);
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }
    return false;
}

// Calculate the SNR of the passed in image
double FITSStack::getSNR(const cv::Mat image)
{
    double snr = 0.0;
    try
    {
        if (image.empty())
            return snr;

        if (image.channels() == 1)
        {
            // Grayscale image
            //cv::Scalar mean, stddev;
            //cv::meanStdDev(image, mean, stddev);
            //if (stddev.val[0] > 0.0)
            //    snr = mean.val[0] / stddev.val[0];

            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(image, &minVal, &maxVal, &minLoc, &maxLoc);

            // Get a 100x100 box around the max pixel
            int X = std::min(std::max(0, maxLoc.x - 50), image.size().width - 100);
            int Y = std::min(std::max(0, maxLoc.y - 50), image.size().height - 100);

            // Measure noise in area around brightest star
            cv::Rect noiseRegion(X, Y, 100, 100);
            cv::Scalar noiseMean, noiseStd;
            cv::meanStdDev(image(noiseRegion), noiseMean, noiseStd);
            snr = (maxVal - noiseMean.val[0]) / noiseStd.val[0];
        }
        else if (image.channels() == 3)
        {
            // Color image: split into channels
            std::vector<cv::Mat> channels;
            cv::split(image, channels);

            for (const auto& channel : channels)
            {
                //cv::Scalar mean, stddev;
                //cv::meanStdDev(channel, mean, stddev);
                //if (stddev.val[0] > 0.0)
                //    snr += mean.val[0] / stddev.val[0];

                // JEE
                double minVal, maxVal;
                cv::Point minLoc, maxLoc;
                cv::minMaxLoc(channel, &minVal, &maxVal, &minLoc, &maxLoc);

                // Get a 100x100 box around the max pixel
                int X = std::min(std::max(0, maxLoc.x - 50), image.size().width - 100);
                int Y = std::min(std::max(0, maxLoc.y - 50), image.size().height - 100);

                // Measure noise in area around brightest star
                cv::Rect noiseRegion(X, Y, 100, 100);
                cv::Scalar noiseMean, noiseStd;
                cv::meanStdDev(channel(noiseRegion), noiseMean, noiseStd);
                snr += (maxVal - noiseMean.val[0]) / noiseStd.val[0];
            }
            snr /= 3;
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(s1).arg(__FUNCTION__);
    }

    return snr;
}

// We're done with the original stack so tidy up and keep data necessary to add individual
// subs to the interim stack as they arrive
void FITSStack::setupRunningStack(struct wcsprm * refWCS, const int numSubs, const float totalWeight)
{
    setInitalStackDone(true);
    m_RunningStackImageData.numSubs = numSubs;
    m_RunningStackImageData.ref_wcsprm = refWCS;
    m_RunningStackImageData.ref_hfr = 0; // JEE
    m_RunningStackImageData.ref_numStars = 0; // JEE
    m_RunningStackImageData.totalWeight = totalWeight;
    tidyUpInitalStack(refWCS);
}

void FITSStack::updateRunningStack(const int numSubs, const float totalWeight)
{
    m_RunningStackImageData.numSubs += numSubs;
    m_RunningStackImageData.totalWeight = totalWeight;
    tidyUpInitalStack(nullptr);
}

// Release FITS and openCV memory used in original stack
void FITSStack::tidyUpInitalStack(struct wcsprm * refWCS)
{
    for (int i = 0; i < m_StackImageData.size(); i++)
    {
        if (m_StackImageData[i].wcsprm != nullptr && m_StackImageData[i].wcsprm != refWCS)
        {
            // Don't free up the reference WCS as we'll need that for later processing
            wcsfree(m_StackImageData[i].wcsprm);
            free(m_StackImageData[i].wcsprm);
            m_StackImageData[i].wcsprm = nullptr;
        }
        // JEE if (!m_StackImageData[i].image.empty())
        int refcount = m_StackImageData[i].image.u->refcount;
        qCDebug(KSTARS_FITS) << QString("JEE m_StackImageData[%1].image refcount %2").arg(i).arg(refcount);
        m_StackImageData[i].image.release();
    }
    m_StackImageData.clear();
}

// Release FITS and openCV memory used in the running stack
void FITSStack::tidyUpRunningStack()
{
    if (m_RunningStackImageData.ref_wcsprm != nullptr)
    {
        wcsfree(m_RunningStackImageData.ref_wcsprm);
        free(m_RunningStackImageData.ref_wcsprm);
        m_RunningStackImageData.ref_wcsprm = nullptr;
    }
}
