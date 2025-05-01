/*
    SPDX-FileCopyrightText: 2025 John Evans <john.e.evans.email@googlemail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fitsstack.h"
#include <fits_debug.h>
#include "fitscommon.h"
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

class FITSData;

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
    data.denoiseAmt = Options::fitsLSDenoiseAmt();
    data.sharpenAmt = Options::fitsLSSharpenAmt();
    data.sharpenKernal = Options::fitsLSSharpenKernal();
    data.sharpenSigma = Options::fitsLSSharpenSigma();
    return data;
}

// JEE
// #ifdef HAVE_OPENCV
bool FITSStack::addImage(void * imageBuffer, int cvType, double pixScale, int width, int height, int bytesPerPixel)
{
    try
    {
        // JEE CV datatypes conversion - hardcoded for now
        size_t rowLen = width * bytesPerPixel;
        cv::Mat image = cv::Mat(height, width, cvType, imageBuffer, rowLen);
        if (image.empty())
        {
            qCDebug(KSTARS_FITS) << QString("%1 Unable to process image in openCV").arg(__FUNCTION__);
            return false; // JEE Think about just skipping here
        }

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

        // Setup the image data structure for later processing
        StackImageData imageData;
        imageData.image = image.clone();
        imageData.plateSolvedStatus = SOLVED_IN_PROGRESS;
        imageData.wcsprm = nullptr;
        imageData.hfr = -1;
        imageData.numStars = 0;
        m_StackImageData.push_back(imageData);
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return false;
}

void FITSStack::addMaster(bool dark, void * imageBuffer, int width, int height, int bytesPerPixel)
{
    try
    {
        if (dark)
            m_MasterDark.release();
        else
            m_MasterFlat.release();

        // JEE CV datatypes conversion - hardcoded for now
        size_t rowLen = width * bytesPerPixel;
        cv::Mat image = cv::Mat(height, width, CV_MAKETYPE(CV_32F, 1), imageBuffer, rowLen);
        /* JEE if (image.empty())
        {
            qCDebug(KSTARS_FITS) << QString("%1 Unable to process master in openCV").arg(__FUNCTION__);
            return false; // JEE Think about just skipping here
        }*/

        if (m_Width == 0)
            m_Width = width;
        else if (m_Width != width)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Master has inconsistent widths").arg(__FUNCTION__);
            return;
        }

        if (m_Height == 0)
            m_Height = height;
        else if (m_Height != height)
        {
            qCDebug(KSTARS_FITS) << QString("%1 Master has inconsistent heights").arg(__FUNCTION__);
            return;
        }

        // Take a deep copy of the passed in image
        if (dark)
            m_MasterDark = image.clone();
        else
        {
            m_MasterFlat = image.clone();

            // Scale the flat down using the median value
            // JEE Hard coding float for now
            std::vector<float> values;
            values.reserve(image.rows * image.cols); // Reserve space for efficiency

            // Copy pixel data to the vector
            if (image.isContinuous())
                // Continuous matrix, so copy all data at once
                std::memcpy(values.data(), image.data, values.size() * sizeof(float));
            else
            {
                // Copy row by row
                size_t rowSize = image.cols * image.channels();
                for (int i = 0; i < image.rows; i++)
                    std::memcpy(values.data() + (i * rowSize), image.ptr(i), rowSize * sizeof(float));
            }

            // Calculate the median value
            float median = Mathematics::RobustStatistics::ComputeLocation(Mathematics::RobustStatistics::LOCATION_MEDIAN, values);

            // Scale the flat to the median
            if (median > 0.0)
                m_MasterFlat /= median;
            else
                qCDebug(KSTARS_FITS) << QString("%1 Unable to calculate median of Master flat").arg(__FUNCTION__);
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
}

// Update plate solving status
bool FITSStack::solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success, const double hfr, const int numStars)
{
    if (timedOut || !success)
    {
        m_StackImageData.last().plateSolvedStatus = SOLVED_FAILED;
        return false;
    }

    // JEE error handling on plate solve fails needs to improve. Make structure of QVectors

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

// JEE do we need this function
bool FITSStack::readyToStack()
{
    for (int i = 0; i < m_StackImageData.size(); i++)
    {
        if (m_StackImageData[i].plateSolvedStatus == SOLVED_IN_PROGRESS)
            return false;
    }
    return true;
}

bool FITSStack::stack()
{
    // JEE if (!readyToStack())
    //    return false;

    try
    {
        int ref = -1;
        cv::Mat finalImage;
        QVector<cv::Mat> m_Aligned;
        for(int i = 0; i < m_StackImageData.size(); i++)
        {
            if (m_StackImageData[i].plateSolvedStatus != SOLVED_OK)
                continue;

            // Calibrate sub
            if (!calibrateSub(m_StackImageData[i].image))
                continue;

            if (ref < 0)
            {                
                // JEE for now we'll make the first sub the reference image
                cv::Mat image32F;
                ref = i;
                m_StackImageData[i].image.convertTo(image32F, CV_MAKETYPE(CV_32F, 1));
                m_Aligned.push_back(image32F.clone());
            }
            else
            {
                // Align this image to the reference image
                cv::Mat warpedImage, warpedImage32F;
                cv::Mat warp = calcWarpMatrix(m_StackImageData[ref].wcsprm, m_StackImageData[i].wcsprm);
                cv::warpPerspective(m_StackImageData[i].image, warpedImage, warp, m_StackImageData[i].image.size(), cv::INTER_LANCZOS4);
                warpedImage.convertTo(warpedImage32F, CV_MAKETYPE(CV_32F, 1));
                m_Aligned.push_back(warpedImage32F.clone());
            }
        }
        // Stack the aligned subs
        float totalWeight = 0.0;
        m_StackedImage32F = stackSubs(m_Aligned, true, totalWeight);

        // Perform any post stacking processing such as sharpening / denoising
        finalImage = postProcessImage(m_StackedImage32F);
        convertMatToFITS(finalImage, m_StackedBuffer);

        if (m_StackData.numInMem <= m_StackImageData.size())
            // We've completed the initial stack so move to incremental stacking as new subs arrive
            setupRunningStack(m_StackImageData[ref].wcsprm, m_Aligned.size(), totalWeight);

        setStackInProgress(false);
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
        setStackInProgress(false);
        return false;
    }
    return true;
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
            cv::Mat warpedImage, warpedImage32F;
            cv::Mat warp = calcWarpMatrix(m_RunningStackImageData.ref_wcsprm, m_StackImageData[i].wcsprm);
            cv::warpPerspective(m_StackImageData[i].image, warpedImage, warp, m_StackImageData[i].image.size(), cv::INTER_LANCZOS4);
            warpedImage.convertTo(warpedImage32F, CV_MAKETYPE(CV_32F, 1));
            m_Aligned.push_back(warpedImage32F.clone());
        }
        // Stack the aligned subs
        float totalWeight = m_RunningStackImageData.totalWeight;
        m_StackedImage32F = stackSubs(m_Aligned, false, totalWeight);

        // Perform any post stacking processing such as sharpening / denoising
        finalImage = postProcessImage(m_StackedImage32F);
        convertMatToFITS(finalImage, m_StackedBuffer);
        updateRunningStack(m_Aligned.size(), totalWeight);
        setStackInProgress(false);
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
        setStackInProgress(false);
        return false;
    }
    return true;
}

// Calculate the warp matrix to align image2 to image1
cv::Mat FITSStack::calcWarpMatrix(struct wcsprm * wcs1, struct wcsprm * wcs2)
{
    cv::Mat warpMatrix;
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
        warpMatrix = cv::findHomography(corners2, corners1, 0);
        if (warpMatrix.empty())
            qCDebug(KSTARS_FITS) << QString("openCV findHomography warp matrix empty");
        cv::Ptr<cv::Formatter> fmt = cv::Formatter::get(cv::Formatter::FMT_DEFAULT);
        std::cout << fmt->format(warpMatrix) << std::endl;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }

    return warpMatrix;
}

// Calibrate the passed in sub with an associated Dark (if available) and / or flat (if available)
bool FITSStack::calibrateSub(cv::Mat & sub)
{
    try
    {
        // Dark subtraction
        if (!m_MasterDark.empty())
            sub - m_MasterDark;

        // Flat calibration
        if (!m_MasterFlat.empty())
            sub / m_MasterFlat;
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return false;
}

// Stack sub
cv::Mat FITSStack::stackSubs(const QVector<cv::Mat> &subs, const bool initial, float &totalWeight)
{
    if (subs.size() <= 0)
    {
        cv::Mat stack;
        return stack;
    }
    int rows = subs[0].rows;
    int cols = subs[0].cols;

    cv::Mat stack = cv::Mat::zeros(rows, cols, CV_MAKETYPE(CV_32F, 1));
    try
    {
        if (subs.size() <= 0)
            return stack;

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
            totalWeight = (initial) ? 0.0 : m_RunningStackImageData.totalWeight;
            if (!initial)
                stack = m_StackedImage32F * m_RunningStackImageData.totalWeight;

            for (int sub = 0; sub < subs.size(); sub++)
            {
                qCDebug(KSTARS_FITS) << QString("JEE sub %1=%2 weight %3 stack %4").arg(sub).arg(subs[sub].at<float>(1000,1000)).arg(weights[sub]).arg(stack.at<float>(1000,1000));
                stack += subs[sub] * weights[sub];
                totalWeight += weights[sub];
            }
            stack /= totalWeight;
            qCDebug(KSTARS_FITS) << QString("JEE stack %1").arg(stack.at<float>(1000,1000));
        }
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return stack;
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
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
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
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
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
        // Convert from 32F to 16U as functions require 16U.
        cv::Mat image;
        image32F.convertTo(image, CV_MAKETYPE(CV_16U, 1));

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
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return finalImage;
}

void FITSStack::redoPostProcessStack()
{
    // Get the current user options for post processing
    m_StackData.postProcessing = loadStackPPData();

    cv::Mat finalImage;
    finalImage = postProcessImage(m_StackedImage32F);
    convertMatToFITS(finalImage, m_StackedBuffer);
    emit stackChanged();
}

bool FITSStack::convertMatToFITS(const cv::Mat image, QByteArray &fitsBuffer)
{
    // Check if the image is valid
    if (image.empty())
    {
        // JEE do something
        return false;
    }

    int width = image.size().width;
    int height = image.size().height;

    //This section sets up the FITS File
    fitsfile *fptr = nullptr;
    int status = 0;
    long fpixel = 1, naxis = 2, nelements, exposure;
    long naxes[2] = { width, height };
    char error_status[512] = {0};

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

    //Note, this is made up.  If you want the actual exposure time, you have to request it from PHD2
    //exposure = 1;
    //fits_update_key(fptr, TLONG, "EXPOSURE", &exposure, "Total Exposure Time", &status);

    //Then it converts from base64 to a QByteArray
    //Then it creates a datastream from the QByteArray to the pixel array for the FITS File
    nelements = width * height;
    //QByteArray converted = QByteArray((char *) image.data, nelements);

    //This finishes up and closes the FITS file
    if (fits_write_img(fptr, TUSHORT, fpixel, nelements, image.data, &status))
    {
        fits_get_errstatus(status, error_status);
        qCDebug(KSTARS_FITS) << "fits_write_img failed " << status;
        status = 0;
        fits_close_file(fptr, &status);
        free(fits_buffer);
        return false;
    }

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

    fitsBuffer = QByteArray(reinterpret_cast<char *>(fits_buffer), fits_buffer_size);
    //m_Data->setExtension(QString("fits"));
    free(fits_buffer);
    return true;
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
        if (!m_StackImageData[i].image.empty())
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
