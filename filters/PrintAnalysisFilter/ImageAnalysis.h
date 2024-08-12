#ifndef __IMAGE_ANALYSIS_H__
#define __IMAGE_ANALYSIS_H__

#include <Windows.h>
#include <streams.h>


namespace ImageUtils
{
	typedef struct AnalysisOpts {
		int         effect;               // Which effect are we processing
		int         aoiHeight;
		int			aoiPartitions;
		CRefTime    effectStartTime;      // When the effect will begin
		CRefTime    effectTime;
		BOOL		connectValues;
		BOOL		blackout;
		GUID		imageType;
	} AnalysisOpts;

	typedef struct INTRGBTRIPLE {
		int red = 0;
		int green = 0;
		int blue = 0;
	} INTRGBTRIPLE;

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

	protected:
		AnalysisOpts	m_opts;
		int				m_iPrevPartitions;
		int				m_iImageWidth;
		int				m_iImageHeight;
		//INTRGBTRIPLE*	m_pResults;
		//int				m_iNumResults;

		INTRGBTRIPLE**	m_ppResults;
		int*			m_piNumResults;

	};
}

#endif

