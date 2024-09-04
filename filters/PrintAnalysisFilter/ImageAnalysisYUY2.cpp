#include "ImageAnalysisYUY2.h"
#include "resource.h"


namespace ImageUtils
{
#define ROW(pImage, width, y) &pImage[width * sizeof(YUY2PIXEL) * y]
#define ROWCOL(pImage, width, x, y) &pImage[(width * sizeof(YUY2PIXEL) * y) + (x*sizeof(YUY2PIXEL))]

#define YUY2_BLACK {0, 128}
#define YUY2_WHITE {255, 128}
#define YUY2_RED_BLUE {80, 255}

    static inline void AdjustMinMax(INTYUVPIXEL& newMin, INTYUVPIXEL& newMax, INTYUVPIXEL& min, INTYUVPIXEL& max)
    {
        if (newMax.luma > max.luma) max.luma = newMax.luma;
        if (newMin.luma < min.luma) min.luma = newMin.luma;

        if (newMax.Cr > max.Cr) max.Cr = newMax.Cr;
        if (newMin.Cr < min.Cr) min.Cr = newMin.Cr;

        if (newMax.Cb > max.Cb) max.Cb = newMax.Cb;
        if (newMin.Cb < min.Cb) min.Cb = newMin.Cb;
    }

    void ImageAnalysisYUY2::NormalizeYUY2(INTYUY2PIXEL& iValue, int iOrigRange, int iMinOrig, int& iNewRange, int& iMinNew)
    {
        iValue.luma = (int)NormalizeValue(iValue.luma, iOrigRange, iMinOrig, iNewRange, iMinNew);
        
        if (m_opts.grayscaleType == IDC_GRAY_NONE)
            iValue.chroma = (int)NormalizeValue(iValue.chroma, iOrigRange, iMinOrig, iNewRange, iMinNew);
    }

    ImageAnalysisYUY2::ImageAnalysisYUY2(int iImageWidth, int iImageHeight, AnalysisOpts opts)
        : ImageAnalysis(iImageWidth, iImageHeight, opts)
        , m_iPrevHistPartitions(0)
    {
        CheckAllocatedMemory();

        m_piHistogram = new INTYUVPIXEL[UCHAR_MAX];
    }

    ImageAnalysisYUY2::~ImageAnalysisYUY2()
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

        if (m_ppHistogram)
        {
            for (int i = 0; i < m_iPrevHistPartitions; i++)
                delete[] m_ppHistogram[i];

            delete[] m_ppHistogram;
            delete[] m_piNumHistogramResults;
        }
    }

    void ImageAnalysisYUY2::CheckAllocatedMemory()
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

            m_ppResults = new INTYUY2PIXEL * [m_opts.aoiPartitions];
            m_piNumResults = new int[m_opts.aoiPartitions];

            for (int i = 0; i < m_opts.aoiPartitions; i++)
            {
                int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
                int xEnd = (int)((float)m_iImageWidth / m_opts.aoiPartitions * (i + 1));

                // make multiple to 2
                xStart = (xStart >> 1) << 1;
                xEnd = (xEnd >> 1) << 1;

                m_piNumResults[i] = xEnd - xStart;
                m_ppResults[i] = new INTYUY2PIXEL[m_piNumResults[i]];
            }
        }

        for (int i = 0; i < m_opts.aoiPartitions; i++)
            memset(m_ppResults[i], 0, m_piNumResults[i] * sizeof(INTYUY2PIXEL));

        m_iPrevPartitions = m_opts.aoiPartitions;
    }

    void ImageAnalysisYUY2::CheckAllocatedMemoryHistogram()
    
    {
        if (m_iPrevHistPartitions != m_opts.aoiPartitions)
        {
            if (m_ppHistogram)
            {
                for (int i = 0; i < m_iPrevHistPartitions; i++)
                    delete[] m_ppHistogram[i];

                delete[] m_ppHistogram;
                delete[] m_piNumHistogramResults;
            }

            m_ppHistogram = new INTYUVPIXEL * [m_opts.aoiPartitions];
            m_piNumHistogramResults = new int[m_opts.aoiPartitions];

            for (int i = 0; i < m_opts.aoiPartitions; i++)
            {
                int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
                int xEnd = (int)((float)m_iImageWidth / m_opts.aoiPartitions * (i + 1));

                // make multiple to 2
                xStart = (xStart >> 1) << 1;
                xEnd = (xEnd >> 1) << 1;

                m_piNumHistogramResults[i] = xEnd - xStart;
                m_ppHistogram[i] = new INTYUVPIXEL[m_piNumHistogramResults[i]];
            }
        }

        for (int i = 0; i < m_opts.aoiPartitions; i++)
            memset(m_ppHistogram[i], 0, m_piNumHistogramResults[i] * sizeof(INTYUVPIXEL));

        m_iPrevHistPartitions = m_opts.aoiPartitions;
    }

    HRESULT ImageAnalysisYUY2::ComputeIntensity(BYTE* pImage, int iAoiMinY, int iAoiMaxY)
    {
        YUY2PIXEL* pYUV = NULL;

        for (int y = iAoiMinY; y < iAoiMaxY; y++)
        {
            pYUV = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, y);

            for (int i = 0; i < m_opts.aoiPartitions; i++)
            {
                int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);

                for (int j = 0; j < m_piNumResults[i]; j++)
                {
                    m_ppResults[i][j].luma += pYUV[j + xStart].luma;
                    m_ppResults[i][j].chroma += pYUV[j + xStart].chroma;
                }
            }
        }

        return NOERROR;
    }
    
    HRESULT ImageAnalysisYUY2::ComputeIntensity(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemory();
        CheckPointer(pImage, E_POINTER);

        ComputeIntensity(pImage, iAoiMinY, iAoiMaxY);

        GrayScale(pImage);
        DrawAOI(pImage);
        Normalize(0, (UCHAR_MAX - 1) * m_opts.aoiHeight, iAoiMinY, iAoiMaxY);
        PlotValues(pImage);

        return NOERROR;
    }

    HRESULT ImageAnalysisYUY2::ComputeAverage(BYTE* pImage, int iAoiMinY, int iAoiMaxY)
    {
        int iNumValues = iAoiMaxY - iAoiMinY;

        ComputeIntensity(pImage, iAoiMinY, iAoiMaxY);

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            for (int j = 0; j < m_piNumResults[i]; j++)
            {
                m_ppResults[i][j].luma      /= iNumValues;
                m_ppResults[i][j].chroma    /= iNumValues;
            }
        }

        return NOERROR;
    }

    HRESULT ImageAnalysisYUY2::ComputeAverage(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemory();
        CheckPointer(pImage, E_POINTER);

        ComputeAverage(pImage, iAoiMinY, iAoiMaxY);

        GrayScale(pImage);
        DrawAOI(pImage);
        Normalize(0, UCHAR_MAX - 1, iAoiMinY, iAoiMaxY);
        PlotValues(pImage);
        
        return NOERROR;
    }

    HRESULT ImageAnalysisYUY2::ComputeHistogramLocal(BYTE* pImage)
    {
        YUY2PIXEL* pYUV = NULL;
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

        CheckAllocatedMemoryHistogram();
        CheckPointer(pImage, E_POINTER);

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
            int xEnd = (int)((float)m_iImageWidth / m_opts.aoiPartitions * (i + 1));

            // make multiple to 2
            xStart = (xStart >> 1) << 1;
            xEnd = (xEnd >> 1) << 1;

            memset(m_piHistogram, 0, UCHAR_MAX * sizeof(INTYUVPIXEL));

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
            {
                pYUV = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, y);

                for (int x = xStart; x < xEnd; x += 2)
                {
                    m_piHistogram[pYUV[x].luma].luma += 1;
                    m_piHistogram[pYUV[x].chroma].Cr += 1;
                }

                for (int x = xStart + 1; x < xEnd; x += 2)
                {
                    m_piHistogram[pYUV[x].luma].luma += 1;
                    m_piHistogram[pYUV[x].chroma].Cb += 1;
                }
            }

            INTYUVPIXEL min, max;
            ComputeMinMax(m_piHistogram, UCHAR_MAX, min, max);

            // normalize
            for (int j = 0; j < UCHAR_MAX; j++)
            {
                m_piHistogram[j].luma = (int)NormalizeValue(m_piHistogram[j].luma, max.luma - min.luma, min.luma, iAoiMinY - iAoiMaxY, iAoiMaxY);
                m_piHistogram[j].Cr = (int)NormalizeValue(m_piHistogram[j].Cr, max.Cr - min.Cr, min.Cr, iAoiMinY - iAoiMaxY, iAoiMaxY);
                m_piHistogram[j].Cb = (int)NormalizeValue(m_piHistogram[j].Cb, max.Cb - min.Cb, min.Cb, iAoiMinY - iAoiMaxY, iAoiMaxY);
            }

            ScaleGraph(m_piHistogram, UCHAR_MAX, m_ppHistogram[i], m_piNumHistogramResults[i]);
        }

        GrayScale(pImage);
        DrawAOI(pImage);
        PlotValuesYUV(pImage);

        return NOERROR;
    }

    void ImageAnalysisYUY2::ScaleGraph(const INTYUVPIXEL* input, int inSize, INTYUVPIXEL* output, int outSize)
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
            output[i].luma = (int)((1 - fFraction) * input[idx].luma + fFraction * input[next_idx].luma);
            output[i].Cr = (int)((1 - fFraction) * input[idx].Cr + fFraction * input[next_idx].Cr);
            output[i].Cb = (int)((1 - fFraction) * input[idx].Cb + fFraction * input[next_idx].Cb);
        }
    }

    void ImageAnalysisYUY2::ComputeMinMax(INTYUVPIXEL* pValues, int iNumValues, INTYUVPIXEL& min, INTYUVPIXEL& max)
    {
        min.luma = min.Cr = min.Cb = INT_MAX;
        max.luma = max.Cr = max.Cb = 0;

        for (int i = 0; i < iNumValues; i++)
            AdjustMinMax(pValues[i], pValues[i], min, max);
    }

    void ImageAnalysisYUY2::Normalize(int iOrigMin, int iOrigMax, int iRangeMin, int iRangeMax)
    {
        int iNewRange = iRangeMin - iRangeMax;

        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            for (int j = 0; j < m_piNumResults[i]; j++)
                NormalizeYUY2(m_ppResults[i][j], iOrigMax - iOrigMin, iOrigMin, iNewRange, iRangeMax);
        }
    }

    void ImageAnalysisYUY2::Blackout(BYTE* pImage)
    {
        if (m_opts.blackoutType == IDC_BLACK_WHOLE)
        {
            int iNumPixels = m_iImageWidth * m_iImageHeight;

            for (int i = 0; i < iNumPixels; i++)
                ((YUY2PIXEL*)pImage)[i] = YUY2_BLACK;
        }
        else if (m_opts.blackoutType == IDC_BLACK_AOI)
        {
            int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
            int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
            {
                YUY2PIXEL* pYUV = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, y);

                for (int x = 0; x < m_iImageWidth; x++)
                    pYUV[x] = YUY2_BLACK;
            }
        }
    }

    void ImageAnalysisYUY2::GrayScale(BYTE* pImage)
    {
        if (m_opts.grayscaleType == IDC_GRAY_WHOLE)
        {
            int iNumPixels = m_iImageWidth * m_iImageHeight;

            for (int i = 0; i < iNumPixels; i++)
                ((YUY2PIXEL*)pImage)[i].chroma = 128;
        }
        else if (m_opts.grayscaleType == IDC_GRAY_AOI)
        {
            int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
            int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
            {
                YUY2PIXEL* pYUV = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, y);

                for (int x = 0; x < m_iImageWidth; x++)
                    pYUV[x].chroma = 128;
            }
        }
    }

    void ImageAnalysisYUY2::DrawAOI(BYTE* pImage)
    {
        int iAoiMinY = (m_iImageHeight - m_opts.aoiHeight) / 2;
        int iAoiMaxY = iAoiMinY + m_opts.aoiHeight;
        YUY2PIXEL* pYuv;
        YUY2PIXEL aoiColor;

        if (m_opts.blackoutType == IDC_BLACK_NONE)
        {
            aoiColor = YUY2_BLACK;
        }
        else
        {
            Blackout(pImage);
            aoiColor = YUY2_WHITE;
        }

        // Draw the bottom line of our area of interest
        pYuv = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, iAoiMinY);

        for (int x = 0; x < m_iImageWidth; x++)
            pYuv[x] = aoiColor;

        // Draw the top line of our area of interest
        pYuv = (YUY2PIXEL*)ROW(pImage, m_iImageWidth, iAoiMaxY);

        for (int x = 0; x < m_iImageWidth; x++)
            pYuv[x] = aoiColor;

        // draw the partition lines
        for (int i = 1; i < m_opts.aoiPartitions; i++)
        {
            int x = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
            x = (x >> 1) << 1;

            for (int y = iAoiMinY; y < iAoiMaxY; y++)
                ((YUY2PIXEL*)ROW(pImage, m_iImageWidth, y))[x] = aoiColor;
        }
    }

    void ImageAnalysisYUY2::DrawLine(BYTE* pImage, int x0, int y0, int x1, int y1, YUY2PIXEL color)
    {
        int dx = abs(x1 - x0);
        int dy = abs(y1 - y0);
        int sx = (x0 < x1) ? 2 : -2;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            // Set the current pixel to red
            YUY2PIXEL* pixel = (YUY2PIXEL*)(ROWCOL(pImage, m_iImageWidth, x0, y0));
            *pixel = color;

            // Check if we've reached the end point
            if (x0 >= x1 && y0 == y1) break;

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

    void ImageAnalysisYUY2::PlotValues(BYTE* pImage)
    {
        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
            xStart = (xStart >> 1) << 1;

            for (int j = 0; j < m_piNumResults[i]; j++)
            {
                ((YUY2PIXEL*)ROW((pImage), m_iImageWidth, m_ppResults[i][j].luma))[j + xStart] = YUY2_WHITE;

                if (m_opts.grayscaleType == IDC_GRAY_NONE)
                    ((YUY2PIXEL*)ROW((pImage), m_iImageWidth, m_ppResults[i][j].chroma))[j + xStart] = YUY2_RED_BLUE;

                if (m_opts.connectValues)
                {
                    if (j > 0)
                        DrawLine(pImage + xStart * sizeof(YUY2PIXEL), j - 1, m_ppResults[i][j - 1].luma, j, m_ppResults[i][j].luma, YUY2_WHITE);
                    
                    if (j > 1 && m_opts.grayscaleType == IDC_GRAY_NONE)
                        DrawLine(pImage + xStart * sizeof(YUY2PIXEL), j - 2, m_ppResults[i][j - 2].chroma, j, m_ppResults[i][j].chroma, YUY2_RED_BLUE);
                }
            }
        }
    }

    void ImageAnalysisYUY2::PlotValuesYUV(BYTE* pImage)
    {
        for (int i = 0; i < m_opts.aoiPartitions; i++)
        {
            int xStart = (int)((float)m_iImageWidth / m_opts.aoiPartitions * i);
            xStart = (xStart >> 1) << 1;

            for (int j = 0; j < m_piNumHistogramResults[i]; j++)
            {
                ((YUY2PIXEL*)ROW((pImage), m_iImageWidth, m_ppHistogram[i][j].luma))[j + xStart] = YUY2_WHITE;

                if (m_opts.grayscaleType == IDC_GRAY_NONE)
                {
                    ((YUY2PIXEL*)ROW((pImage), m_iImageWidth, m_ppHistogram[i][j].Cr))[j + xStart] = YUY2_RED_BLUE;
                    ((YUY2PIXEL*)ROW((pImage), m_iImageWidth, m_ppHistogram[i][j].Cb))[j + xStart] = YUY2_RED_BLUE;
                }
                
                if (m_opts.connectValues && j > 0)
                {
                    DrawLine(pImage + xStart * sizeof(YUY2PIXEL), j - 1, m_ppHistogram[i][j - 1].luma, j, m_ppHistogram[i][j].luma, YUY2_WHITE);

                    if (m_opts.grayscaleType == IDC_GRAY_NONE)
                    {
                        DrawLine(pImage + xStart * sizeof(YUY2PIXEL), j - 1, m_ppHistogram[i][j - 1].Cr, j, m_ppHistogram[i][j].Cr, YUY2_RED_BLUE);
                        DrawLine(pImage + xStart * sizeof(YUY2PIXEL), j - 1, m_ppHistogram[i][j - 1].Cb, j, m_ppHistogram[i][j].Cb, YUY2_RED_BLUE);
                    }
                }
            }
        }
    }
}