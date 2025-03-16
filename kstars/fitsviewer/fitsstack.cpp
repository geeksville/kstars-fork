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
}

FITSStack::~FITSStack()
{
}

// JEE
// #ifdef HAVE_OPENCV
bool FITSStack::addImage(void * imageBuffer, double pixScale, int width, int height, int bytesPerPixel)
{
    try
    {
        // JEE CV datatypes conversion - hardcoded for now
        size_t rowLen = width * bytesPerPixel;
        cv::Mat image = cv::Mat(height, width, CV_MAKETYPE(CV_16U, 1), imageBuffer, rowLen);
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

        // Take a deep copy of the passed in image
        m_Images.push_back(image.clone());
        m_PlateSolvedStatus.push_back(SOLVED_IN_PROGRESS);
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return false;
}

bool FITSStack::addMaster(bool dark, void * imageBuffer, int width, int height, int bytesPerPixel)
{
    try
    {
        // JEE CV datatypes conversion - hardcoded for now
        size_t rowLen = width * bytesPerPixel;
        cv::Mat image = cv::Mat(height, width, CV_MAKETYPE(CV_32F, 1), imageBuffer, rowLen);
        if (image.empty())
        {
            qCDebug(KSTARS_FITS) << QString("%1 Unable to process master in openCV").arg(__FUNCTION__);
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

        // Take a deep copy of the passed in image
        if (dark)
            m_MasterDark = image.clone();
        else
        {
            m_MasterFlat = image.clone();

            // Get the median value

            // Flatten the matrix into a 1D vector
            // JEE Hard coding float for now
            std::vector<float> values;
            values.reserve(image.rows * image.cols); // Reserve space for efficiency

            for (int y = 0; y < image.rows; y++)
                for (int x = 0; x < image.cols; x++)
                    values.push_back(image.at<float>(y, x));

            std::sort(values.begin(), values.end());

            // Scale the flat to the median
            int n = values.size();
            if (n > 0)
            {
                float median = (n % 2 == 0) ? (values[n / 2 - 1] + values[n / 2]) / 2.0 : values[n / 2.0];
                if (median > 0.0)
                    m_MasterFlat /= median;
            }
        }
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
    }
    return false;
}

// Update plate solving status
bool FITSStack::solverDone(const wcsprm * wcsHandle, const bool timedOut, const bool success)
{
    if (timedOut || !success)
    {
        m_PlateSolvedStatus.last() = SOLVED_FAILED;
        return false;
    }

    m_PlateSolvedStatus.last() = SOLVED_OK;

    // Take a deep copy of the WCS state for alignment purposes
    struct wcsprm * wcsCopy = new struct wcsprm;
    wcsCopy->flag = -1; // Allocate space
    int status = 0;
    if ((status = wcssub(1, wcsHandle, 0x0, 0x0, wcsCopy)) != 0)
    {
        m_PlateSolvedStatus.last() = SOLVED_FAILED;
        qCDebug(KSTARS_FITS) << QString("wcssub error processing %1 %2").arg(status).arg(wcs_errmsg[status]);
        return false;
    }
    else if ((status = wcsset(wcsCopy)) != 0)
    {
        m_PlateSolvedStatus.last() = SOLVED_FAILED;
        qCDebug(KSTARS_FITS) << QString("wcsset error processing %1 %2").arg(status).arg(wcs_errmsg[status]);
        return false;
    }

    m_Wcsprm.push_back(wcsCopy);
    return true;
}

// Return whether
bool FITSStack::readyToStack()
{
    for (int i = 0; i < m_PlateSolvedStatus.size(); i++)
    {
        if (m_PlateSolvedStatus[i] == SOLVED_IN_PROGRESS)
            return false;
    }
    return true;
}

bool FITSStack::stack()
{
    if (!readyToStack())
        return false;

    try
    {
        int ref = -1, stackCount = 0;
        cv::Mat image8U1, refImage, refImage8U1, stackedImage, stackedImage32F;
        QVector<cv::Mat> m_Aligned;
        for(int image = 0; image < m_Images.size(); image++)
        {
            if (m_PlateSolvedStatus[image] != SOLVED_OK)
                continue;

            // Calibrate sub
            if (!calibrateSub(m_Images[image]))
                continue;

            if (ref < 0)
            {                
                // JEE for now we'll make the first sub the reference image
                ref = image;
                refImage = m_Images[image];
                refImage.convertTo(refImage8U1, CV_MAKETYPE(CV_8U, 1));
                m_Images[image].convertTo(stackedImage32F, CV_MAKETYPE(CV_32F, 1));
                m_Aligned.push_back(stackedImage32F.clone());
                // Debug stuff
                // cv::namedWindow("Raw Image", cv::WINDOW_NORMAL);
                //cv::moveWindow(win_name, 200, 300);
                //cv::imshow("Raw Image", image);
                //cv::resizeWindow("Raw Image", 300, 200);
                //cv::Mat result;
                //double minX, maxX;
                //cv::namedWindow("Equalized Image", cv::WINDOW_NORMAL);
                //cv::moveWindow(win_name, 200, 300);
                //cv::minMaxIdx(image, &minX, &maxX);
                //cv::convertScaleAbs(image, result, 255.0 / maxX, 0);
                //cv::equalizeHist(result, result);
                //cv::imshow("Equalized Image", result);
                //cv::resizeWindow("Equalized Image", 300, 200);
                //cv::waitKey(0);
            }
            else
            {
                // Align this image to the reference image
                cv::Mat warpedImage, warpedImage32F;
                cv::Mat warp = calcWarpMatrix(m_Wcsprm[ref], m_Wcsprm[image]);
                cv::warpPerspective(m_Images[image], warpedImage, warp, m_Images[image].size(), cv::INTER_LANCZOS4);
                warpedImage.convertTo(warpedImage32F, CV_MAKETYPE(CV_32F, 1));
                stackedImage32F += warpedImage32F;
                m_Aligned.push_back(warpedImage32F.clone());

                // JEE try transforming equalised histogram version
                /*double minX, maxX;
                cv::Mat imageEH, refImageEH;
                cv::minMaxIdx(image8U1, &minX, &maxX);
                cv::convertScaleAbs(image8U1, imageEH, 255.0 / maxX, 0);
                cv::equalizeHist(imageEH, imageEH);
                cv::minMaxIdx(refImage8U1, &minX, &maxX);
                cv::convertScaleAbs(refImage8U1, refImageEH, 255.0 / maxX, 0);
                cv::equalizeHist(refImageEH, refImageEH);*/

                /*warpedImage.convertTo(warpedImage32F, CV_MAKETYPE(CV_32F, 1));
                double thisWeight = 1.0 / (1.0 + image);
                double stackWeight = 1.0 - thisWeight;
                QString s;
                s = QString("warpedImage32F %1 %2 %3").arg(warpedImage32F.type()).arg(warpedImage32F.size().width).arg(warpedImage32F.size().height);
                qCDebug(KSTARS_FITS) << s;
                s = QString("stackedImage %1 %2 %3").arg(stackedImage.type()).arg(stackedImage.size().width).arg(stackedImage.size().height);
                qCDebug(KSTARS_FITS) << s;
                cv::addWeighted(warpedImage32F, thisWeight, stackedImage, stackWeight, 0.0, newStackedImage);
                //newStackedImage = (stackedImage * weight) + (warpedImage16F * (1.0 - weight));
                stackedImage = newStackedImage;*/
            }
            stackCount++;
        }
        //if (stackCount > 0)
        //    stackedImage32F /= stackCount;
        bool windsor = true;
        stackedImage32F = stackImagesSigmaClipping(m_Aligned, windsor);
        stackedImage32F.convertTo(stackedImage, CV_MAKETYPE(CV_16U, 1));        
        convertMatToFITS(stackedImage, m_StackedImage);

        //cv::imshow("Stacked Image", stackedImage32F);
        //cv::waitKey(0);
        //cv::destroyAllWindows();

        //cv::Size size = stackedImage.size();
        //m_StackedImage = QByteArray((char *) stackedImage.data, size.width * size.height);
        return true;
    }
    catch (const cv::Exception &ex)
    {
        QString s1 = ex.what();
        qCDebug(KSTARS_FITS) << QString("openCV exception %1 called from %2").arg(ex.what()).arg(__FUNCTION__);
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

// Calculate the warp matrix to align image2 to image1
bool FITSStack::calibrateSub(cv::Mat & sub)
{
    try
    {
        // Dark subtraction
        if (!m_MasterDark.empty())
            sub - m_MasterDark;

        // Flat calibration
        if (!m_MasterFlat.empty() && m_MedianMasterFlat > 0.0)
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

// Function to stack images using standard and Windsorized Sigma Clipping
cv::Mat FITSStack::stackImagesSigmaClipping(const QVector<cv::Mat> &images, bool windsor)
{
    double sigmaLow = 3.0;    // Lower sigma threshold for clipping
    double sigmaHigh = 3.0;   // Upper sigma threshold for clipping
    double winsorizePercentile = 0.1; // Winsorize 10% of the data

    int rows = images[0].rows;
    int cols = images[0].cols;
    cv::Mat finalImage = cv::Mat::zeros(rows, cols, CV_64F);

    // Iterate over each pixel location
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            std::vector<double> pixelValues;

            // Collect pixel values from all images at (i, j)
            for (const auto &img : images)
                pixelValues.push_back(static_cast<double>(img.at<float_t>(i, j)));

            double pixelValue;
            if (windsor)
                // Apply Winsorized Sigma Clipping
                pixelValue = winsorizedSigmaClipping(pixelValues, sigmaLow, sigmaHigh, winsorizePercentile);
            else
                // Regular sigma clipping
                pixelValue = Mathematics::RobustStatistics::ComputeLocation(
                    Mathematics::RobustStatistics::LOCATION_SIGMACLIPPING, pixelValues, sigmaHigh);
            finalImage.at<double>(i, j) = pixelValue;
        }
    }

    // Convert the final image back to 8-bit format
    cv::Mat finalImage32f;
    finalImage.convertTo(finalImage32f, CV_32F);

    return finalImage32f;
}

// Function to perform Winsorized Sigma Clipping
double FITSStack::winsorizedSigmaClipping(std::vector<double> data, double sigmaLow, double sigmaHigh, double winsorizePercentile)
{
    // Sort the data for percentile calculation
    std::sort(data.begin(), data.end());

    // Calculate the Winsorized bounds
    size_t lowerIndex = static_cast<size_t>(winsorizePercentile * data.size());
    size_t upperIndex = static_cast<size_t>((1.0 - winsorizePercentile) * data.size());
    double lowerBound = data[lowerIndex];
    double upperBound = data[upperIndex];

    // Winsorize the data (replace outliers with bounds)
    for (double& value : data) {
        if (value < lowerBound) value = lowerBound;
        if (value > upperBound) value = upperBound;
    }

    // Perform sigma clipping
    double mean = Mathematics::RobustStatistics::ComputeLocation(Mathematics::RobustStatistics::LOCATION_MEAN, data);
    double var = Mathematics::RobustStatistics::ComputeWeight(Mathematics::RobustStatistics::SCALE_VARIANCE, data);
    double stdDev = std::sqrt(var);
    double lowerThreshold = mean - sigmaLow * stdDev;
    double upperThreshold = mean + sigmaHigh * stdDev;

    std::vector<double> cleanedData;
    for (double value : data)
    {
        double cleanedValue = std::min(std::max(value, lowerThreshold), upperThreshold);
        cleanedData.push_back(cleanedValue);
    }

    return Mathematics::RobustStatistics::ComputeLocation(Mathematics::RobustStatistics::LOCATION_MEAN, cleanedData);
}
