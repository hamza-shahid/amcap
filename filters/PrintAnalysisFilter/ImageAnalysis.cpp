#include "ImageAnalysis.h"
#include "resource.h"

#include <wxdebug.h>



namespace ImageAnalysis
{
#define ROW(pImage, width, y) &pImage[width * 3 * y]
#define ROWCOL(pImage, width, x, y) &pImage[(width * 3 * y) + (x*3)]

#define RGB_BLACK {0, 0, 0}
#define RGB_WHITE {255, 255, 255}
#define RGB_RED {0, 0, 255}
#define RGB_GREEN {0, 255, 0}
#define RGB_BLUE {255, 0, 0}

    static void ScaleGraph(const INTRGBTRIPLE* input, int inSize, INTRGBTRIPLE* output, int outSize)
    {
        float fScaleFactor = (float)inSize / outSize;

        for (int i = 0; i < outSize; i++)
        {
            float fPos = i * fScaleFactor;
            int idx = (int)fPos;
            float fFraction = fPos - idx;

            // Handle boundary conditions
            int next_idx = (idx < inSize - 1) ? idx + 1 : idx;

            // Linear interpolation
            output[i].red = (int)((1 - fFraction) * input[idx].red + fFraction * input[next_idx].red);
            output[i].green = (int)((1 - fFraction) * input[idx].green + fFraction * input[next_idx].green);
            output[i].blue = (int)((1 - fFraction) * input[idx].blue + fFraction * input[next_idx].blue);
        }
    }

    static HRESULT Blackout(BYTE* pData, int numPixels)
    {
        for (int i = 0; i < numPixels; i++)
            ((RGBTRIPLE*)pData)[i] = RGB_BLACK;

        return NOERROR;
    }

    static void DrawAOI(BYTE* pImage, int imageWidth, int imageHeight, int aoiHeight, int aoiPartitions, BOOL blackout)
    {
        int aoiMinY = (imageHeight - aoiHeight) / 2;
        int aoiMaxY = aoiMinY + aoiHeight;
        RGBTRIPLE* pRgb;
        RGBTRIPLE aoiColor;

        if (blackout)
        {
            Blackout(pImage, imageWidth * imageHeight);
            aoiColor = RGB_WHITE;
        }
        else
        {
            aoiColor = RGB_BLACK;
        }

        // Draw the bottom line of our area of interest
        pRgb = (RGBTRIPLE*)ROW(pImage, imageWidth, aoiMinY);

        for (int x = 0; x < imageWidth; x++)
            pRgb[x] = aoiColor;

        // Draw the top line of our area of interest
        pRgb = (RGBTRIPLE*)ROW(pImage, imageWidth, aoiMaxY);

        for (int x = 0; x < imageWidth; x++)
            pRgb[x] = aoiColor;

        // draw the partition lines
        for (int i = 1; i < aoiPartitions; i++)
        {
            int x = ((float)imageWidth / aoiPartitions * i);

            for (int y = aoiMinY; y < aoiMaxY; y++)
                ((RGBTRIPLE*)ROW(pImage, imageWidth, y))[x] = aoiColor;
        }
    }

    static HRESULT ComputeMinMax(INTRGBTRIPLE* pValues, int size, INTRGBTRIPLE& min, INTRGBTRIPLE& max)
    {
        CheckPointer(pValues, E_POINTER);

        min.red = min.green = min.blue = INT_MAX;
        max.red = max.green = max.blue = 0;

        for (int i = 0; i < size; i++)
        {
            if (pValues[i].red > max.red) max.red = pValues[i].red;
            if (pValues[i].red < min.red) min.red = pValues[i].red;

            if (pValues[i].green > max.green) max.green = pValues[i].green;
            if (pValues[i].green < min.green) min.green = pValues[i].green;

            if (pValues[i].blue > max.blue) max.blue = pValues[i].blue;
            if (pValues[i].blue < min.blue) min.blue = pValues[i].blue;
        }

        return NOERROR;
    }

    static HRESULT NormalizeLocal(INTRGBTRIPLE* pValues, int size, int aoiMinY, int aoiMaxY, int aoiPartitions)
    {
        INTRGBTRIPLE min, max;

        CheckPointer(pValues, E_POINTER);

        for (int i = 0; i < aoiPartitions; i++)
        {
            int xStart = ((float)size / aoiPartitions * i);
            int xEnd = ((float)size / aoiPartitions * (i + 1));

            min.red = min.green = min.blue = INT_MAX;
            max.red = max.green = max.blue = 0;

            // compute min max for the partition
            for (int x = xStart; x < xEnd; x++)
            {
                if (pValues[x].red > max.red) max.red = pValues[x].red;
                if (pValues[x].red < min.red) min.red = pValues[x].red;

                if (pValues[x].green > max.green) max.green = pValues[x].green;
                if (pValues[x].green < min.green) min.green = pValues[x].green;

                if (pValues[x].blue > max.blue) max.blue = pValues[x].blue;
                if (pValues[x].blue < min.blue) min.blue = pValues[x].blue;
            }

            // normalize 
            for (int x = xStart; x < xEnd; x++)
            {
                pValues[x].red = (max.red - min.red) ? (((float)pValues[x].red - min.red) / (max.red - min.red)) * (aoiMaxY - aoiMinY) + aoiMinY : aoiMinY;
                pValues[x].green = (max.green - min.green) ? (((float)pValues[x].green - min.green) / (max.green - min.green)) * (aoiMaxY - aoiMinY) + aoiMinY : aoiMinY;
                pValues[x].blue = (max.blue - min.blue) ? (((float)pValues[x].blue - min.blue) / (max.blue - min.blue)) * (aoiMaxY - aoiMinY) + aoiMinY : aoiMinY;

                if (pValues[x].green > 255 || pValues[x].green < 0)
                    pValues[x].green = 255;
            }
        }

        return NOERROR;
    }

    static HRESULT Normalize(INTRGBTRIPLE* pValues, int size, INTRGBTRIPLE min, INTRGBTRIPLE max, int rangeMin, int rangeMax)
    {
        CheckPointer(pValues, E_POINTER);

        for (int i = 0; i < size; i++)
        {
            pValues[i].red = (((float)pValues[i].red - min.red) / (max.red - min.red)) * (rangeMax - rangeMin) + rangeMin;
            pValues[i].green = (((float)pValues[i].green - min.green) / (max.green - min.green)) * (rangeMax - rangeMin) + rangeMin;
            pValues[i].blue = (((float)pValues[i].blue - min.blue) / (max.blue - min.blue)) * (rangeMax - rangeMin) + rangeMin;
        }

        return NOERROR;
    }

    static void DrawLine(BYTE* pImage, int imageWidth, int x0, int y0, int x1, int y1, RGBTRIPLE color)
    {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            // Set the current pixel to red
            RGBTRIPLE* pixel = (RGBTRIPLE*)(ROWCOL(pImage, imageWidth, x0, y0));
            *pixel = color;

            // Check if we've reached the end point
            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;

            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
    }

    static void PlotValues(BYTE* pImage, int imageWidth, INTRGBTRIPLE* pValues, int size, BOOL connect)
    {
        for (int x = 0; x < size; x++)
        {
            ((RGBTRIPLE*)ROW(pImage, imageWidth, pValues[x].red))[x] = RGB_RED;
            ((RGBTRIPLE*)ROW(pImage, imageWidth, pValues[x].green))[x] = RGB_GREEN;
            ((RGBTRIPLE*)ROW(pImage, imageWidth, pValues[x].blue))[x] = RGB_BLUE;

            if (connect && x > 0)
            {
                DrawLine(pImage, imageWidth, x - 1, pValues[x - 1].red, x, pValues[x].red, RGB_RED);
                DrawLine(pImage, imageWidth, x - 1, pValues[x - 1].green, x, pValues[x].green, RGB_GREEN);
                DrawLine(pImage, imageWidth, x - 1, pValues[x - 1].blue, x, pValues[x].blue, RGB_BLUE);
            }
        }
    }

    void ComputeIntensity(BYTE* pImage, int imageWidth, int aoiMinY, int aoiMaxY, INTRGBTRIPLE* pIntensities)
    {
        RGBTRIPLE* pRGB = NULL;
        
        for (int y = aoiMinY; y < aoiMaxY; y++)
        {
            pRGB = (RGBTRIPLE*)ROW(pImage, imageWidth, y);

            for (int x = 0; x < imageWidth; x++)
            {
                pIntensities[x].red += pRGB[x].rgbtRed;
                pIntensities[x].green += pRGB[x].rgbtGreen;
                pIntensities[x].blue += pRGB[x].rgbtBlue;
            }
        }

    }

    HRESULT ComputeIntensity(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts)
    {
        int numValues = imageWidth;
        INTRGBTRIPLE* pIntensities = new INTRGBTRIPLE[numValues]();
        int aoiMinY = (imageHeight / 2) - (opts.aoiHeight / 2);
        int aoiMaxY = aoiMinY + opts.aoiHeight;
        INTRGBTRIPLE min, max;

        CheckPointer(pImage, E_POINTER);

        ComputeIntensity(pImage, imageWidth, aoiMinY, aoiMaxY, pIntensities);

        DrawAOI(pImage, imageWidth, imageHeight, opts.aoiHeight, opts.aoiPartitions, opts.blackout);
        ComputeMinMax(pIntensities, numValues, min, max);
        Normalize(pIntensities, numValues, min, max, aoiMinY, aoiMaxY);
        PlotValues(pImage, imageWidth, pIntensities, numValues, opts.connectValues);

        delete[] pIntensities;

        return NOERROR;
    }

    HRESULT ComputeAverage(BYTE* pImage, int imageWidth, int aoiMinY, int aoiMaxY, INTRGBTRIPLE* pAverages)
    {
        ComputeIntensity(pImage, imageWidth, aoiMinY, aoiMaxY, pAverages);

        for (int x = 0; x < imageWidth; x++)
        {
            pAverages[x].red /= (aoiMaxY - aoiMinY);
            pAverages[x].green /= (aoiMaxY - aoiMinY);
            pAverages[x].blue /= (aoiMaxY - aoiMinY);
        }

        return NOERROR;
    }

    HRESULT ComputeAverage(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts)
    {
        int numValues = imageWidth;
        INTRGBTRIPLE* pAverages = new INTRGBTRIPLE[numValues]();
        int aoiMinY = (imageHeight / 2) - (opts.aoiHeight / 2);
        int aoiMaxY = aoiMinY + opts.aoiHeight;
        INTRGBTRIPLE min, max;

        CheckPointer(pImage, E_POINTER);

        ComputeAverage(pImage, imageWidth, aoiMinY, aoiMaxY, pAverages);
        DrawAOI(pImage, imageWidth, imageHeight, opts.aoiHeight, opts.aoiPartitions, opts.blackout);
        
        if (opts.effect == IDC_MEAN_LOCAL)
        {
            NormalizeLocal(pAverages, numValues, aoiMinY, aoiMaxY, opts.aoiPartitions);
        }
        else
        {
            ComputeMinMax(pAverages, numValues, min, max);
            Normalize(pAverages, numValues, min, max, aoiMinY, aoiMaxY);
        }

        PlotValues(pImage, imageWidth, pAverages, numValues, opts.connectValues);

        delete[] pAverages;

        return NOERROR;
    }

    HRESULT ComputeHistogram(BYTE* pImage, int imageWidth, int yStart, int yEnd, INTRGBTRIPLE* pHistogram)
    {
        RGBTRIPLE* pRGB = NULL;

        CheckPointer(pImage, E_POINTER);
        CheckPointer(pHistogram, E_POINTER);

        for (int y = yStart; y < yEnd; y++)
        {
            pRGB = (RGBTRIPLE*)ROW(pImage, imageWidth, y);

            for (int x = 0; x < imageWidth; x++)
            {
                pHistogram[pRGB[x].rgbtRed].red += 1;
                pHistogram[pRGB[x].rgbtGreen].green += 1;
                pHistogram[pRGB[x].rgbtBlue].blue += 1;
            }
        }

        return NOERROR;
    }

    HRESULT ComputeHistogramLocal(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts)
    {
        RGBTRIPLE* pRGB = NULL;
        INTRGBTRIPLE** ppiHistogram;
        INTRGBTRIPLE* piHistogram;
        int* piHistSize;

        CheckPointer(pImage, E_POINTER);

        int aoiMinY = (imageHeight / 2) - (opts.aoiHeight / 2);
        int aoiMaxY = aoiMinY + opts.aoiHeight;

        piHistogram = new INTRGBTRIPLE[UCHAR_MAX];
        ppiHistogram = new INTRGBTRIPLE * [opts.aoiPartitions];
        piHistSize = new int[opts.aoiPartitions];

        for (int i = 0; i < opts.aoiPartitions; i++)
        {
            int xStart = ((float)imageWidth / opts.aoiPartitions * i);
            int xEnd = ((float)imageWidth / opts.aoiPartitions * (i+1));

            piHistSize[i] = xEnd - xStart;
            ppiHistogram[i] = new INTRGBTRIPLE[piHistSize[i]];
           
            memset(piHistogram, 0, UCHAR_MAX * sizeof(INTRGBTRIPLE));

            for (int y = aoiMinY; y < aoiMaxY; y++)
            {
                pRGB = (RGBTRIPLE*)ROW(pImage, imageWidth, y);
                
                for (int x = xStart; x < xEnd; x++)
                {
                    piHistogram[pRGB[x].rgbtRed].red += 1;
                    piHistogram[pRGB[x].rgbtGreen].green += 1;
                    piHistogram[pRGB[x].rgbtBlue].blue += 1;
                }
            }
            
            ScaleGraph(piHistogram, UCHAR_MAX, ppiHistogram[i], piHistSize[i]);
        }

        DrawAOI(pImage, imageWidth, imageHeight, opts.aoiHeight, opts.aoiPartitions, opts.blackout);
        
        INTRGBTRIPLE min, max;
        int iOffset = 0;

        for (int i = 0; i < opts.aoiPartitions; i++)
        {
            ComputeMinMax(ppiHistogram[i], piHistSize[i], min, max);
            Normalize(ppiHistogram[i], piHistSize[i], min, max, aoiMinY, aoiMaxY);
            PlotValues(pImage + iOffset, imageWidth, ppiHistogram[i], piHistSize[i], opts.connectValues);

            iOffset += piHistSize[i] * 3;
        }
        
        for (int i = 0; i < opts.aoiPartitions; i++)
            delete[] ppiHistogram[i];

        delete[] ppiHistogram;
        delete[] piHistSize;
        delete[] piHistogram;

        return NOERROR;
    }
}