#ifndef __IMAGE_ANALYSIS_H__
#define __IMAGE_ANALYSIS_H__

#include <Windows.h>
#include <streams.h>


namespace ImageAnalysis
{
	typedef struct AnalysisOpts {
		int         effect;               // Which effect are we processing
		int         aoiHeight;
		int			aoiPartitions;
		CRefTime    effectStartTime;      // When the effect will begin
		CRefTime    effectTime;
		BOOL		connectValues;
		BOOL		blackout;
	} AnalysisOpts;

	typedef struct INTRGBTRIPLE {
		int red = 0;
		int green = 0;
		int blue = 0;
	} INTRGBTRIPLE;

	HRESULT ComputeIntensity(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts);
	HRESULT ComputeAverage(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts);
	HRESULT ComputeHistogram(BYTE* pImage, int imageWidth, int yStart, int yEnd, INTRGBTRIPLE* pHistogram);
	HRESULT ComputeHistogramLocal(BYTE* pImage, int imageWidth, int imageHeight, AnalysisOpts opts);
}

#endif

