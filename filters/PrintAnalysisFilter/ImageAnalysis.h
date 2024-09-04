#ifndef __IMAGE_ANALYSIS_H__
#define __IMAGE_ANALYSIS_H__

#include <Windows.h>
#include <streams.h>


namespace ImageUtils
{
	typedef struct AnalysisOpts
	{
		int         effect;               // Which effect are we processing
		int         aoiHeight;
		int			aoiPartitions;
		CRefTime    effectStartTime;      // When the effect will begin
		CRefTime    effectTime;
		BOOL		connectValues;
		GUID		imageType;
		int			blackoutType;
		int			grayscaleType;
	} AnalysisOpts;

	class ImageAnalysis
	{
	public:
		ImageAnalysis(int iImageWidth, int iImageHeight, AnalysisOpts opts);
		~ImageAnalysis();

		virtual HRESULT ComputeIntensity(BYTE* pImage) PURE;
		virtual HRESULT ComputeAverage(BYTE* pImage) PURE;
		virtual HRESULT ComputeHistogramLocal(BYTE* pImage) PURE;

		void SetAnalysisOpts(AnalysisOpts& opts);

	protected:
		virtual void CheckAllocatedMemory() PURE;

		static double NormalizeValue(double fValue, double fOrigRange, double fMinOrig, double fNewRange, double fMinNew);

	protected:
		AnalysisOpts	m_opts;
		int				m_iPrevPartitions;
		int				m_iImageWidth;
		int				m_iImageHeight;
	};
}

#endif

