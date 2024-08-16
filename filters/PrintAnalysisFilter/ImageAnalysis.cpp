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
    }

    void ImageAnalysis::SetAnalysisOpts(AnalysisOpts& opts)
    {
        m_opts = opts;
    }

    double ImageAnalysis::NormalizeValue(double fValue, double fOrigRange, double fMinOrig, double fNewRange, double fMinNew)
    {
        return fOrigRange ? ((fValue - fMinOrig) / fOrigRange) * fNewRange + fMinNew : fMinNew;
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