#include "ImageAnalysisRGB.h"
#include "resource.h"


namespace ImageUtils
{
#define ROW(pImage, width, y) &pImage[width * sizeof(RGBTRIPLE) * y]
#define ROWCOL(pImage, width, x, y) &pImage[(width * sizeof(RGBTRIPLE) * y) + (x*sizeof(RGBTRIPLE))]

#define RGB_BLACK {0, 0, 0}
#define RGB_WHITE {255, 255, 255}
#define RGB_RED {0, 0, 255}
#define RGB_GREEN {0, 255, 0}
#define RGB_BLUE {255, 0, 0}

    
    static inline void AdjustMinMax(INTRGBTRIPLE& newMin, INTRGBTRIPLE& newMax, INTRGBTRIPLE& min, INTRGBTRIPLE& max)
    {
        if (newMax.red > max.red) max.red = newMax.red;
        if (newMin.red < min.red) min.red = newMin.red;

        if (newMax.green > max.green) max.green = newMax.green;
        if (newMin.green < min.green) min.green = newMin.green;

        if (newMax.blue > max.blue) max.blue = newMax.blue;
        if (newMin.blue < min.blue) min.blue = newMin.blue;
    }
    
    void ImageAnalysisRGB::NormalizeRGB(INTRGBTRIPLE& iValue, int iOrigRange, int iMinOrig, int iNewRange, int iMinNew)
    {
        iValue.red = (int) NormalizeValue(iValue.red, iOrigRange, iMinOrig, iNewRange, iMinNew);
        iValue.green = (int)NormalizeValue(iValue.green, iOrigRange, iMinOrig, iNewRange, iMinNew);
        iValue.blue = (int)NormalizeValue(iValue.blue, iOrigRange, iMinOrig, iNewRange, iMinNew);
    }

    ImageAnalysisRGB::ImageAnalysisRGB(int iImageWidth, int iImageHeight, AnalysisOpts opts)
        : ImageAnalysis(iImageWidth, iImageHeight, opts)
    {
        CheckAllocatedMemory();

        m_piHistogram = new INTRGBTRIPLE[UCHAR_MAX];
    }

    ImageAnalysisRGB::~ImageAnalysisRGB()
    {
        if (m_piHistogram)
            delete[] m_piHistogram;

        if (m_ppResults)
        {
            for (int i = 0; i < m_iPrevPartitions; i++)
                delete[] m_ppResults[i];

            delete[] m_ppResults;
            delete[] m_piNumResults;
        }
    }

    void ImageAnalysisRGB::CheckAllocatedMemory()
    {
        if (m_iPrevPartitions != m_opts.aoiPartitions)
        {
            if (m_ppResults)
            {
                for (int i = 0; i < m_iPrevPartitions; i++)
                    delete[] m_ppResults[i];

                delete[] m_ppResults;
                delete[] m_piNumResults;
            }

            m_ppResults = new INTRGBTRIPLE * [m_opts.aoiPartitions];
            m_piNumResults = new int[m_opts.aoiPartitions];

            for (int i = 0; i < m_opts.aoiPartitions; i++)
            {
                int xStart = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * i);
                int xEnd = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * (i + 1));

                m_piNumResults[i] = xEnd - xStart;
                m_ppResults[i] = new INTRGBTRIPLE[m_piNumResults[i]];
            }
        }

        for (int i = 0; i < m_opts.aoiPartitions; i++)
            memset(m_ppResults[i], 0, m_piNumResults[i] * sizeof(INTRGBTRIPLE));

        m_iPrevPartitions = m_opts.aoiPartitions;
    }

    HRESULT ImageAnalysisRGB::ComputeIntensity(BYTE* pImage, int iAoiMinY, int iAoiMaxY)
    {
        RGBTRIPLE* pRGB = NULL;

        for (int y = iAoiMinY; y < iAoiMaxY; y++)
        {
            pRGB = (RGBTRIPLE*)ROW(pImage, m_iImageWidth, y);

            for (int i = 0; i < m_opts.aoiPartitions; i++)
            {
                int xStart = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * i);

                for (int j = 0; j < m_piNumResults[i]; j++)
                {
                    m_ppResults[i][j].red += pRGB[j + xStart].rgbtRed;
                    m_ppResults[i][j].green += pRGB[j + xStart].rgbtGreen;
                    m_ppResults[i][j].blue += pRGB[j + xStart].rgbtBlue;
                }
            }
        }

        return NOERROR;
    }

    HRESULT ImageAnalysisRGB::ComputeIntensity(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemory();
        CheckPointer(pImage, E_POINTER);

        ComputeIntensity(pImage, iAoiMinY, iAoiMaxY);

        DrawAOI(pImage);
        Normalize(0, (UCHAR_MAX - 1) * m_opts.aoiHeight, iAoiMinY, iAoiMaxY);
        PlotValues(pImage);

        return NOERROR;
    }

    HRESULT ImageAnalysisRGB::ComputeAverage(BYTE* pImage, int iAoiMinY, int iAoiMaxY)
    {
        int iNumValues = iAoiMaxY - iAoiMinY;

        ComputeIntensity(pImage, iAoiMinY, iAoiMaxY);

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            for (int j = 0; j < m_piNumResults[i]; j++)
            {
                m_ppResults[i][j].red /= iNumValues;
                m_ppResults[i][j].green /= iNumValues;
                m_ppResults[i][j].blue /= iNumValues;
            }
        }

        return NOERROR;
    }

    HRESULT ImageAnalysisRGB::ComputeAverage(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemory();
        CheckPointer(pImage, E_POINTER);

        ComputeAverage(pImage, iAoiMinY, iAoiMaxY);

        DrawAOI(pImage);
        Normalize(0, UCHAR_MAX - 1, iAoiMinY, iAoiMaxY);
        PlotValues(pImage);

        return NOERROR;
    }

    HRESULT ImageAnalysisRGB::ComputeHistogramLocal(BYTE* pImage)
    {
        RGBTRIPLE* pRGB = NULL;
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemory();
        CheckPointer(pImage, E_POINTER);

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            int xStart = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * i);
            int xEnd = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * (i + 1));

            memset(m_piHistogram, 0, UCHAR_MAX * sizeof(INTRGBTRIPLE));

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
            {
                pRGB = (RGBTRIPLE*)ROW(pImage, m_iImageWidth, y);

                for (int x = xStart; x < xEnd; x++)
                {
                    m_piHistogram[pRGB[x].rgbtRed].red += 1;
                    m_piHistogram[pRGB[x].rgbtGreen].green += 1;
                    m_piHistogram[pRGB[x].rgbtBlue].blue += 1;
                }
            }

            INTRGBTRIPLE min, max;
            ComputeMinMax(m_piHistogram, UCHAR_MAX, min, max);

            // normalize
            for (int j = 0; j < UCHAR_MAX; j++)
            {
                m_piHistogram[j].red    = (int)NormalizeValue(m_piHistogram[j].red, max.red - min.red, min.red, iAoiMaxY - iAoiMinY, iAoiMinY);
                m_piHistogram[j].green  = (int)NormalizeValue(m_piHistogram[j].green, max.green - min.green, min.green, iAoiMaxY - iAoiMinY, iAoiMinY);
                m_piHistogram[j].blue   = (int)NormalizeValue(m_piHistogram[j].blue, max.blue - min.blue, min.blue, iAoiMaxY - iAoiMinY, iAoiMinY);
            }

            ScaleGraph(m_piHistogram, UCHAR_MAX, m_ppResults[i], m_piNumResults[i]);
        }

        DrawAOI(pImage);
        PlotValues(pImage);

        return NOERROR;
    }
    
    void ImageAnalysisRGB::ComputeMinMax(INTRGBTRIPLE* pValues, int iNumValues, INTRGBTRIPLE& min, INTRGBTRIPLE& max)
    {
        min.red = min.green = min.blue = INT_MAX;
        max.red = max.green = max.blue = 0;

        for (int i = 0; i < iNumValues; i++)
            AdjustMinMax(pValues[i], pValues[i], min, max);
    }

    void ImageAnalysisRGB::Normalize(int iOrigMin, int iOrigMax, int iRangeMin, int iRangeMax)
    {
        int iNewRange = iRangeMax - iRangeMin;

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            for (int j = 0; j < m_piNumResults[i]; j++)
                NormalizeRGB(m_ppResults[i][j], iOrigMax - iOrigMin, iOrigMin, iNewRange, iRangeMin);
        }
    }
    

    void ImageAnalysisRGB::ScaleGraph(const INTRGBTRIPLE* input, int inSize, INTRGBTRIPLE* output, int outSize)
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

    void ImageAnalysisRGB::Blackout(BYTE* pImage)
    {
        int iNumPixels = m_iImageWidth * m_iImageHeight;

        for (int i = 0; i < iNumPixels; i++)
            ((RGBTRIPLE*)pImage)[i] = RGB_BLACK;
    }
    
    void ImageAnalysisRGB::DrawAOI(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;
        RGBTRIPLE* pRgb;
        RGBTRIPLE aoiColor;

        if (m_opts.blackout)
        {
            Blackout(pImage);
            aoiColor = RGB_WHITE;
        }
        else
        {
            aoiColor = RGB_BLACK;
        }

        // Draw the bottom line of our area of interest
        pRgb = (RGBTRIPLE*)ROW(pImage, m_iImageWidth, iAoiMinY);

        for (int x = 0; x < m_iImageWidth; x++)
            pRgb[x] = aoiColor;

        // Draw the top line of our area of interest
        pRgb = (RGBTRIPLE*)ROW(pImage, m_iImageWidth, iAoiMaxY);

        for (int x = 0; x < m_iImageWidth; x++)
            pRgb[x] = aoiColor;

        // draw the partition lines
        for (int i = 1; i < m_opts.aoiPartitions; i++)
        {
            int x = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * i);

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
                ((RGBTRIPLE*)ROW(pImage, m_iImageWidth, y))[x] = aoiColor;
        }
    }

    void ImageAnalysisRGB::DrawLine(BYTE* pImage, int x0, int y0, int x1, int y1, RGBTRIPLE color)
    {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            // Set the current pixel to red
            RGBTRIPLE* pixel = (RGBTRIPLE*)(ROWCOL(pImage, m_iImageWidth, x0, y0));
            *pixel = color;

            // Check if we've reached the end point
            if (x0 == x1 && y0 == y1) break;

            int e2 = 2 * err;

            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }

            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void ImageAnalysisRGB::PlotValues(BYTE* pImage)
    {
        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            int xStart = (int) ((float)m_iImageWidth / m_opts.aoiPartitions * i);

            for (int j = 0; j < m_piNumResults[i]; j++)
            {
                ((RGBTRIPLE*)ROW((pImage), m_iImageWidth, m_ppResults[i][j].red))[j + xStart] = RGB_RED;
                ((RGBTRIPLE*)ROW((pImage), m_iImageWidth, m_ppResults[i][j].green))[j + xStart] = RGB_GREEN;
                ((RGBTRIPLE*)ROW((pImage), m_iImageWidth, m_ppResults[i][j].blue))[j + xStart] = RGB_BLUE;

                if (m_opts.connectValues && j > 0)
                {
                    DrawLine(pImage + xStart*3, j - 1, m_ppResults[i][j - 1].red, j, m_ppResults[i][j].red, RGB_RED);
                    DrawLine(pImage + xStart*3, j - 1, m_ppResults[i][j - 1].green, j, m_ppResults[i][j].green, RGB_GREEN);
                    DrawLine(pImage + xStart*3, j - 1, m_ppResults[i][j - 1].blue, j, m_ppResults[i][j].blue, RGB_BLUE);
                }
            }
        }
    }
}