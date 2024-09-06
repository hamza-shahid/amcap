#include "ImageAnalysis.h"
#include "resource.h"


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
}