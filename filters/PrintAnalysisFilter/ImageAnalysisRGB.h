#ifndef __IMAGE_ANALYSIS_RGB_H__
#define __IMAGE_ANALYSIS_RGB_H__

#include "ImageAnalysis.h"

namespace ImageUtils
{
	class ImageAnalysisRGB : public ImageAnalysis
	{
	public:
		ImageAnalysisRGB(int iImageWidth, int iImageHeight, AnalysisOpts opts);
		virtual ~ImageAnalysisRGB();

	private:
		void CheckAllocatedMemory();

		HRESULT ComputeIntensity(BYTE* pImage, int aoiMinY, int aoiMaxY);
		HRESULT ComputeAverage(BYTE* pImage, int iAoiMinY, int iAoiMaxY);
		
		void DrawAOI(BYTE* pImage);
		void Blackout(BYTE* pImage);
		void PlotValues(BYTE* pImage);
		void DrawLine(BYTE* pImage, int x0, int y0, int x1, int y1, RGBTRIPLE color);
		void ScaleGraph(const INTRGBTRIPLE* input, int inSize, INTRGBTRIPLE* output, int outSize);
		void NormalizeLocal(int iRangeMin, int iRangeMax);
		void Normalize(int iRangeMin, int iRangeMax);

		static void ComputeMinMax(INTRGBTRIPLE* pValues, int iNumValues, INTRGBTRIPLE& min, INTRGBTRIPLE& max);

	public:
		HRESULT ComputeIntensity(BYTE* pImage);
		HRESULT ComputeAverage(BYTE* pImage);
		HRESULT ComputeHistogramLocal(BYTE* pImage);

	private:
		INTRGBTRIPLE*	m_piHistogram;
	};
}

#endif // __IMAGE_ANALYSIS_RGB_H__