/***************************************************************************
                          FITSImage.cpp  -  FITS Image
                             -------------------
    begin                : Thu Jan 22 2004
    copyright            : (C) 2004 by Jasem Mutlaq
    email                : mutlaqja@ikarustech.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Some code fragments were adapted from Peter Kirchgessner's FITS plugin*
 *   See http://members.aol.com/pkirchg for more details.                  *
 ***************************************************************************/

#include <config-kstars.h>

#include "fitsimage.h"

#include <cmath>
#include <cstdlib>

#include <QApplication>
#include <QFile>
#include <QProgressDialog>
#include <KMessageBox>

#ifdef HAVE_WCSLIB
#include <wcshdr.h>
#include <wcsfix.h>
#include <wcs.h>
#include <getwcstab.h>
#endif

#include "ksutils.h"

#define ZOOM_DEFAULT	100.0
#define ZOOM_MIN	10
#define ZOOM_MAX	400
#define ZOOM_LOW_INCR	10
#define ZOOM_HIGH_INCR	50

const int MINIMUM_ROWS_PER_CENTER=3;

#define JM_UPPER_LIMIT  .5

#define LOW_EDGE_CUTOFF_1   50
#define LOW_EDGE_CUTOFF_2   10
#define DIFFUSE_THRESHOLD   0.2
#define MINIMUM_EDGE_LIMIT  3

//#define FITS_DEBUG

bool greaterThan(Edge *s1, Edge *s2)
{
    return s1->width > s2->width;
}

FITSImage::FITSImage(FITSMode fitsMode)
{
    image_buffer = NULL;
    wcs_coord    = NULL;
    fptr = NULL;
    maxHFRStar = NULL;
    tempFile  = false;
    starsSearched = false;
    HasWCS = false;
    mode = fitsMode;
}

FITSImage::~FITSImage()
{
    int status=0;

    delete(image_buffer);

    if (starCenters.count() > 0)
        qDeleteAll(starCenters);

    delete (wcs_coord);

    if (fptr)
    {
        fits_close_file(fptr, &status);

        if (tempFile)
             QFile::remove(filename);

    }
}

bool FITSImage::loadFITS ( const QString &inFilename, QProgressDialog *progress )
{
    int status=0, nulval=0, anynull=0;
    long fpixel[2], nelements, naxes[2];
    char error_status[512];

    qDeleteAll(starCenters);
    starCenters.clear();

    if (mode == FITS_NORMAL && progress)
    {
        progress->setLabelText(i18n("Please hold while loading FITS file..."));
        progress->setWindowTitle(i18n("Loading FITS"));
    }

    if (mode == FITS_NORMAL && progress)
        progress->setValue(30);

    if (fptr)
    {

        fits_close_file(fptr, &status);

        if (tempFile)
            QFile::remove(filename);
    }

    filename = inFilename;

    if (filename.contains("/tmp/"))
        tempFile = true;
    else
        tempFile = false;

    filename.remove("file://");


    if (fits_open_image(&fptr, filename.toAscii(), READONLY, &status))
    {
        fits_report_error(stderr, status);
        fits_get_errstatus(status, error_status);
        if (progress)
            KMessageBox::error(0, i18n("Could not open file %1 (fits_get_img_param). Error %2", filename, QString::fromUtf8(error_status)), i18n("FITS Open"));
        return false;
    }

    if (mode == FITS_NORMAL && progress)
        if (progress->wasCanceled())
            return false;

    if (mode == FITS_NORMAL && progress)
        progress->setValue(40);


    if (fits_get_img_param(fptr, 2, &(stats.bitpix), &(stats.ndim), naxes, &status))
    {
        fits_report_error(stderr, status);
        fits_get_errstatus(status, error_status);

        if (progress)
            KMessageBox::error(0, i18n("FITS file open error (fits_get_img_param): %1", QString::fromUtf8(error_status)), i18n("FITS Open"));
        return false;
    }

    if (stats.ndim < 2)
    {
        if (progress)
            KMessageBox::error(0, i18n("1D FITS images are not supported in KStars."), i18n("FITS Open"));
        return false;
    }

    if (naxes[0] == 0 || naxes[1] == 0)
    {
        if (progress)
            KMessageBox::error(0, i18n("Image has invalid dimensions %1x%2", naxes[0], naxes[1]), i18n("FITS Open"));
        return false;
    }


    if (fits_get_img_type(fptr, &data_type, &status))
    {
        fits_report_error(stderr, status);
        fits_get_errstatus(status, error_status);

        if (progress)
            KMessageBox::error(0, i18n("FITS file open error (fits_get_img_type): %1", QString::fromUtf8(error_status)), i18n("FITS Open"));
        return false;
    }

    if (mode == FITS_NORMAL && progress)
        if (progress->wasCanceled())
            return false;

    if (mode == FITS_NORMAL && progress)
        progress->setValue(60);

    stats.dim[0] = naxes[0];
    stats.dim[1] = naxes[1];

    delete (image_buffer);
    image_buffer = NULL;

    image_buffer = new float[stats.dim[0] * stats.dim[1]];

    if (image_buffer == NULL)
    {
        qDebug() << "Not enough memory for image_buffer";
        return false;
    }
    if (mode == FITS_NORMAL && progress)
    {
        if (progress->wasCanceled())
        {
        delete (image_buffer);
        return false;
        }
    }

    if (mode == FITS_NORMAL && progress)
        progress->setValue(70);

    nelements = stats.dim[0] * stats.dim[1];
    fpixel[0] = 1;
    fpixel[1] = 1;

    qApp->processEvents();

    if (fits_read_2d_flt(fptr, 0, nulval, naxes[0], naxes[0], naxes[1], image_buffer, &anynull, &status))
    {
        fprintf(stderr, "fits_read_pix error\n");
        fits_report_error(stderr, status);
        return false;
    }

    if (mode == FITS_NORMAL && progress)
    {
        if (progress->wasCanceled())
        {
            delete (image_buffer);
            return false;
        }
    }

    calculateStats();

    if (mode == FITS_NORMAL && progress)
        progress->setValue(80);

    //currentWidth  = stats.dim[0];
   // currentHeight = stats.dim[1];

    qApp->processEvents();

    if (mode == FITS_NORMAL)
    {
        checkWCS();

        if (progress)
            progress->setValue(90);
    }

    if (mode == FITS_NORMAL && progress)
    {
        if (progress->wasCanceled())
        {
            delete (image_buffer);
            return false;
        }
    }

    if (mode == FITS_NORMAL && progress)
        progress->setValue(100);

    starsSearched = false;

    return true;

}

int FITSImage::saveFITS( const QString &newFilename )
{
    int status=0;
    long fpixel[2], nelements;
    fitsfile *new_fptr;

    nelements = stats.dim[0] * stats.dim[1];
    fpixel[0] = 1;
    fpixel[1] = 1;


    /* Create a new File, overwriting existing*/
    if (fits_create_file(&new_fptr, newFilename.toAscii(), &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    /* Copy ALL contents */
    if (fits_copy_file(fptr, new_fptr, 1, 1, 1, &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    /* close current file */
    if (fits_close_file(fptr, &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    if (tempFile)
    {
        QFile::remove(filename);
        tempFile = false;
    }

    filename = newFilename;

    fptr = new_fptr;

    // For color images, we return for now.
    if (stats.ndim > 2)
        return status;

    /* Write Data */
    if (fits_write_pix(fptr, TFLOAT, fpixel, nelements, image_buffer, &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    /* Write keywords */

    // Minimum
    if (fits_update_key(fptr, TDOUBLE, "DATAMIN", &(stats.min), "Minimum value", &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    // Maximum
    if (fits_update_key(fptr, TDOUBLE, "DATAMAX", &(stats.max), "Maximum value", &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    // ISO Date
    if (fits_write_date(fptr, &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    QString history = QString("Modified by KStars on %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"));
    // History
    if (fits_write_history(fptr, history.toAscii(), &status))
    {
        fits_report_error(stderr, status);
        return status;
    }

    return status;
}


int FITSImage::calculateMinMax(bool refresh)
{
    int status, nfound=0;
    long npixels;

    status = 0;

    if (refresh == false)
    {
        if (fits_read_key_dbl(fptr, "DATAMIN", &(stats.min), NULL, &status) ==0)
            nfound++;

        if (fits_read_key_dbl(fptr, "DATAMAX", &(stats.max), NULL, &status) == 0)
            nfound++;

        // If we found both keywords, no need to calculate them, unless they are both zeros
        if (nfound == 2 && !(stats.min == 0 && stats.max ==0))
            return 0;
    }

    npixels  = stats.dim[0] * stats.dim[1];         /* number of pixels in the image */
    stats.min= 1.0E30;
    stats.max= -1.0E30;

    for (int i=0; i < npixels; i++)
    {
        if (image_buffer[i] < stats.min) stats.min = image_buffer[i];
        else if (image_buffer[i] > stats.max) stats.max = image_buffer[i];
    }

    //qDebug() << "DATAMIN: " << stats.min << " - DATAMAX: " << stats.max;
    return 0;
}


void FITSImage::calculateStats(bool refresh)
{
    calculateMinMax(refresh);
    // #1 call average, average is used in std deviation
    stats.average = average();
    // #2 call std deviation
    stats.stddev  = stddev();

    if (refresh && markStars)
        // Let's try to find star positions again after transformation
        starsSearched = false;

}

double FITSImage::average()
{
    double sum=0;
    int row=0;
    int width   = stats.dim[0];
    int height  = stats.dim[1];

    if (!image_buffer) return -1;

    for (int i= 0 ; i < height; i++)
    {
        row = (i * width);
        for (int j= 0; j < width; j++)
            sum += image_buffer[row+j];
    }

    return (sum / (width * height ));

}

double FITSImage::stddev()
{

    int row=0;
    double lsum=0;
    int width   = stats.dim[0];
    int height  = stats.dim[1];

    if (!image_buffer) return -1;

    for (int i= 0 ; i < height; i++)
    {
        row = (i * width);
        for (int j= 0; j < width; j++)
        {
            lsum += (image_buffer[row + j] - stats.average) * (image_buffer[row+j] - stats.average);
        }
    }

    return (sqrt(lsum/(width * height - 1)));


}

void FITSImage::setFITSMinMax(double newMin,  double newMax)
{
    stats.min = newMin;
    stats.max = newMax;
}

int FITSImage::getFITSRecord(QString &recordList, int &nkeys)
{
    char *header;
    int status=0;

    if (fits_hdr2str(fptr, 0, NULL, 0, &header, &nkeys, &status))
    {
        fits_report_error(stderr, status);
        free(header);
        return -1;
    }

    recordList = QString(header);

    free(header);

    return 0;
}

bool FITSImage::checkCollision(Edge* s1, Edge*s2)
{
    int dis; //distance

    int diff_x=s1->x - s2->x;
    int diff_y=s1->y - s2->y;

    dis = abs( sqrt( diff_x*diff_x + diff_y*diff_y));
    dis -= s1->width/2;
    dis -= s2->width/2;

    if (dis<=0) //collision
    return true;

    //no collision
    return false;
}


/*** Find center of stars and calculate Half Flux Radius */
void FITSImage::findCentroid(int initStdDev, int minEdgeWidth)
{
    double threshold=0;
    double avg = 0;
    double sum=0;
    double min=0;
    int pixelRadius =0;
    int pixVal=0;
    int badPix=0;

    double JMIndex = histogram->getJMIndex();
    int badPixLimit=0;

    QList<Edge*> edges;

    while (initStdDev >= 1)
    {
       if (JMIndex > DIFFUSE_THRESHOLD)
       {
           threshold = stats.max - stats.stddev* (MINIMUM_STDVAR - initStdDev);
           min =stats.min;
           badPixLimit=10;
       }
       else
       {
           threshold = (stats.max - stats.min)/2.0 + stats.min  + stats.stddev* (MINIMUM_STDVAR - initStdDev);
           if ( (stats.max - stats.min)/2.0 > (stats.average+stats.stddev*5))
               threshold = stats.average+stats.stddev*initStdDev;
           min = stats.min;
           badPixLimit =2;
       }


       threshold -= stats.min;

       #ifdef FITS_DEBUG
       qDebug() << "JMIndex: " << JMIndex << endl;
       qDebug() << "The threshold level is " << threshold << endl;
       #endif

    // Detect "edges" that are above threshold
    for (int i=0; i < stats.dim[1]; i++)
    {
        pixelRadius = 0;

        for(int j=0; j < stats.dim[0]; j++)
        {
            pixVal = image_buffer[j+(i*stats.dim[0])] - min;

            // If pixel value > threshold, let's get its weighted average
            if ( pixVal >= threshold || (sum > 0 && badPix <= badPixLimit))
            {
                if (pixVal < threshold)
                    badPix++;
                else
                   badPix=0;

                avg += j * pixVal;
                sum += pixVal;
                pixelRadius++;

            }
            // Value < threshold but avg exists
            else if (sum > 0)
            {

                // We found a potential centroid edge
                if (pixelRadius >= (minEdgeWidth - (MINIMUM_STDVAR - initStdDev)))
                {
                    float center = avg/sum;
                    if (center > 0)
                    {
                        int i_center = round(center);

                        Edge *newEdge = new Edge();

                        newEdge->x          = center;
                        newEdge->y          = i;
                        newEdge->scanned    = 0;
                        newEdge->val        = image_buffer[i_center+(i*stats.dim[0])] - min;
                        newEdge->width      = pixelRadius;
                        newEdge->HFR        = 0;
                        newEdge->sum        = sum;

                        edges.append(newEdge);
                    }

                }

                // Reset
                badPix = 0;
                avg=0;
                sum=0;
                pixelRadius=0;


            }
         }
     }

    #ifdef FITS_DEBUG
    qDebug() << "Total number of edges found is: " << edges.count() << endl;
    #endif

    if (edges.count() >= MINIMUM_EDGE_LIMIT)
        break;

      qDeleteAll(edges);
      edges.clear();
      initStdDev--;
    }

    int cen_count=0;
    int cen_x=0;
    int cen_y=0;
    int cen_v=0;
    int cen_w=0;
    int width_sum=0;

    // Let's sort edges, starting with widest
    qSort(edges.begin(), edges.end(), greaterThan);

    // Now, let's scan the edges and find the maximum centroid vertically
    for (int i=0; i < edges.count(); i++)
    {
        #ifdef FITS_DEBUG
        qDebug() << "# " << i << " Edge at (" << edges[i]->x << "," << edges[i]->y << ") With a value of " << edges[i]->val  << " and width of "
         << edges[i]->width << " pixels. with sum " << edges[i]->sum << endl;
        #endif

        // If edge scanned already, skip
        if (edges[i]->scanned == 1)
        {
            #ifdef FITS_DEBUG
            qDebug() << "Skipping check for center " << i << " because it was already counted" << endl;
            #endif
            continue;
        }

        #ifdef FITS_DEBUG
        qDebug() << "Invetigating edge # " << i << " now ..." << endl;
        #endif

        // Get X, Y, and Val of edge
        cen_x = edges[i]->x;
        cen_y = edges[i]->y;
        cen_v = edges[i]->sum;
        cen_w = edges[i]->width;

        float avg_x = 0;
        float avg_y = 0;

        sum = 0;
        cen_count=0;

        // Now let's compare to other edges until we hit a maxima
        for (int j=0; j < edges.count();j++)
        {
            if (edges[j]->scanned)
                continue;

            if (checkCollision(edges[j], edges[i]))
            {
                if (edges[j]->sum >= cen_v)
                {
                    cen_v = edges[j]->sum;
                    cen_w = edges[j]->width;
                }

                edges[j]->scanned = 1;
                cen_count++;

                avg_x += edges[j]->x * edges[j]->val;
                avg_y += edges[j]->y * edges[j]->val;
                sum += edges[j]->val;

                continue;
            }

        }

        int cen_limit = (MINIMUM_ROWS_PER_CENTER - (MINIMUM_STDVAR - initStdDev));

        if (edges.count() < LOW_EDGE_CUTOFF_1)
        {
            if (edges.count() < LOW_EDGE_CUTOFF_2)
                cen_limit = 1;
            else
                cen_limit = 2;
        }

    #ifdef FITS_DEBUG
    qDebug() << "center_count: " << cen_count << " and initstdDev= " << initStdDev << " and limit is "
             << cen_limit << endl;
    #endif

        if (cen_limit < 1)
            continue;

        // If centroid count is within acceptable range
        //if (cen_limit >= 2 && cen_count >= cen_limit)
        if (cen_count >= cen_limit)
        {
            // We detected a centroid, let's init it
            Edge *rCenter = new Edge();

            rCenter->x = avg_x/sum;
            rCenter->y = avg_y/sum;
            width_sum += rCenter->width;
            rCenter->width = cen_w;

           #ifdef FITS_DEBUG
           qDebug() << "Found a real center with number with (" << rCenter->x << "," << rCenter->y << ")" << endl;

           //qDebug() << "Profile for this center is:" << endl;
           //for (int i=edges[rc_index]->width/2; i >= -(edges[rc_index]->width/2) ; i--)
              // qDebug() << "#" << i << " , " << image_buffer[(int) round(rCenter->x-i+(rCenter->y*stats.dim[0]))] - stats.min <<  endl;

           #endif

            // Calculate Total Flux From Center, Half Flux, Full Summation
            double TF=0;
            double HF=0;
            double FSum=0;

            cen_x = (int) round(rCenter->x);
            cen_y = (int) round(rCenter->y);

            if (cen_x < 0 || cen_x > stats.dim[0] || cen_y < 0 || cen_y > stats.dim[1])
                continue;


            // Complete sum along the radius
            //for (int k=0; k < rCenter->width; k++)
            for (int k=rCenter->width/2; k >= -(rCenter->width/2) ; k--)
                FSum += image_buffer[cen_x-k+(cen_y*stats.dim[0])] - min;

            // Half flux
            HF = FSum / 2.0;

            // Total flux starting from center
            TF = image_buffer[cen_y * stats.dim[0] + cen_x] - min;

            int pixelCounter = 1;

            // Integrate flux along radius axis until we reach half flux
            for (int k=1; k < rCenter->width/2; k++)
            {
                TF += image_buffer[cen_y * stats.dim[0] + cen_x + k] - min;
                TF += image_buffer[cen_y * stats.dim[0] + cen_x - k] - min;

                if (TF >= HF)
                {
                    #ifdef FITS_DEBUG
                    qDebug() << "Stopping at TF " << TF << " after #" << k << " pixels." << endl;
                    #endif
                    break;
                }

                pixelCounter++;
            }

            // Calculate weighted Half Flux Radius
            rCenter->HFR = pixelCounter * (HF / TF);
            // Store full flux
            rCenter->val = FSum;

            #ifdef FITS_DEBUG
            qDebug() << "HFR for this center is " << rCenter->HFR << " pixels and the total flux is " << FSum << endl;
            #endif
             starCenters.append(rCenter);

        }
    }

    if (starCenters.count() > 1 && mode != FITS_FOCUS)
    {
        float width_avg = width_sum / starCenters.count();

        float lsum =0, sdev=0;

        foreach(Edge *center, starCenters)
            lsum += (center->width - width_avg) * (center->width - width_avg);

        sdev = (sqrt(lsum/(starCenters.count() - 1))) * 4;

        // Reject stars > 4 * stddev
        foreach(Edge *center, starCenters)
            if (center->width > sdev)
                starCenters.removeOne(center);

        //foreach(Edge *center, starCenters)
            //qDebug() << center->x << "," << center->y << "," << center->width << "," << center->val << endl;

    }

    // Release memory
    qDeleteAll(edges);
}

double FITSImage::getHFR(HFRType type)
{
    // This method is less susceptible to noise
    // Get HFR for the brightest star only, instead of averaging all stars
    // It is more consistent.
    // TODO: Try to test this under using a real CCD.

    if (starCenters.size() == 0)
        return -1;

    if (type == HFR_MAX)
    {
         maxHFRStar = NULL;
         int maxVal=0;
         int maxIndex=0;
     for (int i=0; i < starCenters.count() ; i++)
        {
            if (starCenters[i]->val > maxVal)
            {
                maxIndex=i;
                maxVal = starCenters[i]->val;

            }
        }

        maxHFRStar = starCenters[maxIndex];
        return starCenters[maxIndex]->HFR;
    }

    double FSum=0;
    double avgHFR=0;

    // Weighted average HFR
    for (int i=0; i < starCenters.count() ; i++)
    {
        avgHFR += starCenters[i]->val * starCenters[i]->HFR;
        FSum   += starCenters[i]->val;
    }

    if (FSum != 0)
    {
        //qDebug() << "Average HFR is " << avgHFR / FSum << endl;
        return (avgHFR / FSum);
    }
    else
        return -1;

}

void FITSImage::applyFilter(FITSScale type, float *image, double min, double max)
{
    if (type == FITS_NONE || histogram == NULL)
        return;

    double coeff=0;
    float val=0,bufferVal =0;

    if (image == NULL)
        image = image_buffer;

    int width = stats.dim[0];
    int height = stats.dim[1];

    if (min == -1)
        min = stats.min;
    if (max == -1)
        max = stats.max;

    switch (type)
    {
    case FITS_AUTO:
    case FITS_LINEAR:
        for (int i=0; i < height; i++)
            for (int j=0; j < width; j++)
            {
                bufferVal = image[i * width + j];
                if (bufferVal < min) bufferVal = min;
                else if (bufferVal > max) bufferVal = max;
                image[i * width + j] = bufferVal;

            }
        break;

    case FITS_LOG:
        coeff = max / log(1 + max);

        for (int i=0; i < height; i++)
            for (int j=0; j < width; j++)
            {
                bufferVal = image[i * width + j];
                if (bufferVal < min) bufferVal = min;
                else if (bufferVal > max) bufferVal = max;
                val = (coeff * log(1 + bufferVal));
                if (val < min) val = min;
                else if (val > max) val = max;
                image[i * width + j] = val;
            }
        break;

    case FITS_SQRT:
        coeff = max / sqrt(max);

        for (int i=0; i < height; i++)
            for (int j=0; j < width; j++)
            {
                bufferVal = (int) image[i * width + j];
                if (bufferVal < min) bufferVal = min;
                else if (bufferVal > max) bufferVal = max;
                val = (int) (coeff * sqrt(bufferVal));
                image[i * width + j] = val;
            }
        break;

    case FITS_AUTO_STRETCH:
    {
       min = stats.average - stats.stddev;
       //if (min < 0)
           //min =0;
       //max = histogram->getMeanStdDev()*3 / histogram->getBinWidth() + min;
       max = stats.average + stats.stddev * 3;

         for (int i=0; i < height; i++)
            for (int j=0; j < width; j++)
            {
               bufferVal = image[i * width + j];
               if (bufferVal < min) bufferVal = min;
               else if (bufferVal > max) bufferVal = max;
               image[i * width + j] = bufferVal;
             }
       }
       break;

     case FITS_HIGH_CONTRAST:
     {
        //min = stats.average - stats.stddev;
        min = stats.average + stats.stddev;
        if (min < 0)
            min =0;
        //max = histogram->getMeanStdDev()*3 / histogram->getBinWidth() + min;
        max = stats.average + stats.stddev * 3;

          for (int i=0; i < height; i++)
             for (int j=0; j < width; j++)
             {
                bufferVal = image[i * width + j];
                if (bufferVal < min) bufferVal = min;
                else if (bufferVal > max) bufferVal = max;
                image[i * width + j] = bufferVal;
              }
        }
        break;

     case FITS_EQUALIZE:
     {
        QVarLengthArray<int, INITIAL_MAXIMUM_WIDTH> cumulativeFreq = histogram->getCumulativeFreq();
        coeff = 255.0 / (height * width);

        for (int i=0; i < height; i++)
            for (int j=0; j < width; j++)
            {
                bufferVal = (int) (image[i * width + j] - min) * histogram->getBinWidth();

                if (bufferVal >= cumulativeFreq.size())
                    bufferVal = cumulativeFreq.size()-1;

                val = (int) (coeff * cumulativeFreq[bufferVal]);

                image[i * width + j] = val;
            }
     }
     break;

     case FITS_HIGH_PASS:
        min = stats.average;
        for (int i=0; i < height; i++)
           for (int j=0; j < width; j++)
           {
              bufferVal = image[i * width + j];
              if (bufferVal < min) bufferVal = min;
              else if (bufferVal > max) bufferVal = max;
              image[i * width + j] = bufferVal;
            }
        break;


    default:
        return;
        break;
    }

    calculateStats(true);
}

void FITSImage::subtract(float *dark_buffer)
{
    for (int i=0; i < stats.dim[0]*stats.dim[1]; i++)
    {
        image_buffer[i] -= dark_buffer[i];
        if (image_buffer[i] < 0)
            image_buffer[i] = 0;
    }

    calculateStats(true);
}

int FITSImage::findStars()
{
    if (histogram == NULL)
        return -1;

    if (starsSearched == false)
    {
        qDeleteAll(starCenters);
        starCenters.clear();

        if (histogram->getJMIndex() < JM_UPPER_LIMIT)
        {
             findCentroid();
             getHFR();
        }
    }

    starsSearched = true;

    return starCenters.count();

}

void FITSImage::getCenterSelection(int *x, int *y)
{
    if (starCenters.count() == 0)
        return;

    Edge *pEdge = new Edge();
    pEdge->x = *x;
    pEdge->y = *y;
    pEdge->width = 1;

    foreach(Edge *center, starCenters)
        if (checkCollision(pEdge, center))
        {
            *x = center->x;
            *y = center->y;
            break;
        }

    delete (pEdge);
}

void FITSImage::checkWCS()
{
#ifdef HAVE_WCSLIB

    int status=0;
    char *header;
    int nkeyrec, nreject, nwcs, stat[2];
    double imgcrd[2], phi, pixcrd[2], theta, world[2];
    struct wcsprm *wcs=0;
    int width=getWidth();
    int height=getHeight();

    if (fits_hdr2str(fptr, 1, NULL, 0, &header, &nkeyrec, &status))
    {
        fits_report_error(stderr, status);
        return;
    }

    if ((status = wcspih(header, nkeyrec, WCSHDR_all, -3, &nreject, &nwcs, &wcs)))
    {
      fprintf(stderr, "wcspih ERROR %d: %s.\n", status, wcshdr_errmsg[status]);
      return;
    }

    free(header);

    if (wcs == 0)
    {
      //fprintf(stderr, "No world coordinate systems found.\n");
      return;
    }

    // FIXME: Call above goes through EVEN if no WCS is present, so we're adding this to return for now.
    if (wcs->crpix[0] == 0)
        return;

    HasWCS = true;

    if ((status = wcsset(wcs)))
    {
      fprintf(stderr, "wcsset ERROR %d: %s.\n", status, wcs_errmsg[status]);
      return;
    }

    delete (wcs_coord);

    wcs_coord = new wcs_point[width*height];

    wcs_point *p = wcs_coord;

    for (int i=0; i < height; i++)
    {
        for (int j=0; j < width; j++)
        {
            pixcrd[0]=j;
            pixcrd[1]=i;

            if ((status = wcsp2s(wcs, 1, 2, &pixcrd[0], &imgcrd[0], &phi, &theta, &world[0], &stat[0])))
            {
                  fprintf(stderr, "wcsp2s ERROR %d: %s.\n", status,
                  wcs_errmsg[status]);
            }
            else
            {
                p->ra  = world[0];
                p->dec = world[1];

                p++;
            }
       }
    }
#endif

}
