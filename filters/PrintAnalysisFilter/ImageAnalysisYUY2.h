#ifndef __IMAGE_ANALYSIS_YUY2_H__
#define __IMAGE_ANALYSIS_YUY2_H__

#include "ImageAnalysis.h"

namespace ImageUtils
{
	typedef struct YUY2PIXEL
	{
		BYTE luma;
		BYTE chroma;
	} YUY2PIXEL;
	
	typedef struct INTYUY2PIXEL
	{
		int luma;
		int chroma;
	} INTYUY2PIXEL;

	typedef struct INTYUVPIXEL
	{
		int luma;
		int Cr;
		int Cb;
	} INTYUVPIXEL;

	class ImageAnalysisYUY2 : public ImageAnalysis
	{
	public:
		ImageAnalysisYUY2(int iImageWidth, int iImageHeight, AnalysisOpts opts);
		virtual ~ImageAnalysisYUY2();

	private:
		void CheckAllocatedMemory();
		void CheckAllocatedMemoryHistogram();

		void DrawAOI(BYTE* pImage);
		void Blackout(BYTE* pImage);
		void GrayScale(BYTE* pImage);
		void DrawLine(BYTE* pImage, int x0, int y0, int x1, int y1, YUY2PIXEL color);
		void PlotValues(BYTE* pImage);
		void PlotValuesYUV(BYTE* pImage);
		void ComputeMinMax(INTYUVPIXEL* pValues, int iNumValues, INTYUVPIXEL& min, INTYUVPIXEL& max);
		void ScaleGraph(const INTYUVPIXEL* input, int inSize, INTYUVPIXEL* output, int outSize);

		HRESULT ComputeIntensity(BYTE* pImage, int iAoiMinY, int iAoiMaxY);
		HRESULT ComputeAverage(BYTE* pImage, int iAoiMinY, int iAoiMaxY);

		void Normalize(int iOrigMin, int iOrigMax, int iRangeMin, int iRangeMax);
		void NormalizeYUY2(INTYUY2PIXEL& iValue, int iOrigRange, int iMinOrig, int& iNewRange, int& iMinNew);

	public:
		HRESULT ComputeIntensity(BYTE* pImage);
		HRESULT ComputeAverage(BYTE* pImage);
		HRESULT ComputeHistogramLocal(BYTE* pImage);

	private:
		INTYUVPIXEL*	m_piHistogram;
		INTYUY2PIXEL**	m_ppResults;
		int*			m_piNumResults;

		int				m_iPrevHistPartitions;
		INTYUVPIXEL**	m_ppHistogram;
		int*			m_piNumHistogramResults;
	};
}

#endif // __IMAGE_ANALYSIS_YUY2_H__