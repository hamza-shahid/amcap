#include "ImageAnalysis.h"
#include "resource.h"

#include <wxdebug.h>


namespace ImageUtils
{
    ImageAnalysis::ImageAnalysis(int iImageWidth, int iImageHeight, AnalysisOpts opts)
        : m_iPrevPartitions(0)
    {
        m_iImageWidth = iImageWidth;
        m_iImageHeight = iImageHeight;
        m_opts = opts;
    }

    ImageAnalysis::~ImageAnalysis()
    {
        if (m_ppResults)
        {
            for (int i = 0; i < m_iPrevPartitions; i++)
                delete[] m_ppResults[i];

            delete[] m_ppResults;
            delete[] m_piNumResults;
        }
    }

    void ImageAnalysis::SetAnalysisOpts(AnalysisOpts& opts)
    {
        m_opts = opts;
    }

    /*
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
    */
}