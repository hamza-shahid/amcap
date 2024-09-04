#ifndef __PRINT_ANALYSIS_FILTER__
#define __PRINT_ANALYSIS_FILTER__

#include <streams.h>

#include "paoptions.h"


class CPrintAnalysisFilter 
    : public CTransInPlaceFilter
    , public ISpecifyPropertyPages
    , public IAnalysisOptions
    , public CPersistStream
{

public:

    DECLARE_IUNKNOWN;
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN punk, HRESULT* phr);

    // Reveals PrintAnalysis and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

    // Overrriden from CTransInPlaceFilter base class
    HRESULT Transform(IMediaSample* pSample);
    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType* pmt);

    // ISpecifyPropertyPages interface
    STDMETHODIMP GetPages(CAUUID* pPages);
    
    // These implement the custom IAnalysisOptions interface
    STDMETHODIMP get_AnalysisOptions(ImageUtils::AnalysisOpts& opts);
    STDMETHODIMP put_AnalysisOptions(ImageUtils::AnalysisOpts opts);

    // CPersistStream stuff
    HRESULT WriteToStream(IStream* pStream);
    HRESULT ReadFromStream(IStream* pStream);
    STDMETHODIMP GetClassID(CLSID* pClsid);

private:

    // Constructor
    CPrintAnalysisFilter(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr);
    virtual ~CPrintAnalysisFilter();

    CCritSec    m_filterLock;           // Private play critical section
    CMediaType  m_mediaType;            // Media type of the connected input pin
    
    ImageUtils::ImageAnalysis*  m_pAnalysis;
    ImageUtils::AnalysisOpts    m_opts;

}; // CPrintAnalysisFilter

#endif // __PRINT_ANALYSIS_FILTER__
