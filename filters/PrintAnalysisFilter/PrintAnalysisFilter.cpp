#include <Windows.h>

#include "PrintAnalysisFilter.h"
#include "ImageAnalysisRGB.h"
#include "pauuids.h"
#include "resource.h"


using namespace ImageUtils;

#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;

//
// Constructor
//
CPrintAnalysisFilter::CPrintAnalysisFilter(TCHAR* tszName, LPUNKNOWN punk, HRESULT* phr) 
    : CTransInPlaceFilter(tszName, punk, CLSID_PrintAnalysis, phr)
    , CPersistStream(punk, phr)
    , m_pAnalysis(NULL)
{
    m_opts.effect           = IDC_INTENSITY;
    m_opts.aoiHeight        = 100;
    m_opts.aoiPartitions    = 1;
    m_opts.connectValues    = FALSE;
    m_opts.blackout         = FALSE;

} // (Constructor)


//
// Destructor
//
CPrintAnalysisFilter::~CPrintAnalysisFilter()
{
    if (m_pAnalysis)
        delete m_pAnalysis;
} // (Destructor)


//
// CreateInstance
//
// Provide the way for COM to create a CPrintAnalysisFilter object
//
CUnknown* CPrintAnalysisFilter::CreateInstance(LPUNKNOWN punk, HRESULT* phr)
{
    ASSERT(phr);

    CPrintAnalysisFilter* pNewObject = new CPrintAnalysisFilter(NAME("Print Analysis"), punk, phr);

    if (pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance

//
// NonDelegatingQueryInterface
//
// Reveals IIPEffect and ISpecifyPropertyPages
//
STDMETHODIMP CPrintAnalysisFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if (riid == IID_IAnalysisOptions) {
        return GetInterface((IAnalysisOptions*)this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages*)this, ppv);
    }
    else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface

  //
// Transform
//
// This where the actual analysis will be performed
//
HRESULT CPrintAnalysisFilter::Transform(IMediaSample *pSample)
{
    CheckPointer(pSample, E_POINTER);

    CRefTime tStart, tStop;
    HRESULT hr = pSample->GetTime((REFERENCE_TIME*)&tStart, (REFERENCE_TIME*)&tStop);

    if (m_mediaType.subtype == MEDIASUBTYPE_RGB24)
    {
        return TransformRGB(pSample);
    }

    return NOERROR;
} // Transform


//
// TransformRGB
//
// This where the actual analysis will be performed on rgb input
//
HRESULT CPrintAnalysisFilter::TransformRGB(IMediaSample* pSample)
{
    BYTE* pData;             // Pointer to the actual image buffer

    CheckPointer(pSample, E_POINTER);
    pSample->GetPointer(&pData);

    if (m_pAnalysis)
    {
        switch (m_opts.effect)
        {
        case IDC_INTENSITY:
            m_pAnalysis->ComputeIntensity(pData);
            break;

        case IDC_MEAN:
        case IDC_MEAN_LOCAL:
            m_pAnalysis->ComputeAverage(pData);
            break;

        case IDC_HISTOGRAM:
            m_pAnalysis->ComputeHistogramLocal(pData);
            break;
        }
    }

    return NOERROR;
} // TransformRGB

//
// CheckInputType
//
// Check whether we support the media type
//
HRESULT CPrintAnalysisFilter::CheckInputType(const CMediaType* mtIn)
{
    CheckPointer(mtIn, E_POINTER);

    // check this is a VIDEOINFOHEADER type
    if (*mtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    // Can we transform this type
    if (IsEqualGUID(*mtIn->Type(), MEDIATYPE_Video))
    {
        if (IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_RGB24))
        {
            VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)mtIn->Format();
            
            if (pvi->bmiHeader.biBitCount == 24)
                return NOERROR;
        }
        else if (IsEqualGUID(*mtIn->Subtype(), MEDIASUBTYPE_YUY2))
        {
            VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)mtIn->Format();
            return NOERROR;
        }

    }
    return E_FAIL;
} // CheckInputType

//
// SetMediaType
//
// Called when the media type is set on one of the filter's pins
// We want to save this mediatype to use in the transform function
//
HRESULT CPrintAnalysisFilter::SetMediaType(PIN_DIRECTION direction, const CMediaType* pmt)
{
    if (direction == PINDIR_INPUT)
    {
        // save the media type
        m_mediaType = *pmt;

        if (pmt->formattype == FORMAT_VideoInfo)
        {
            // resize our window to the default capture size
            m_opts.aoiHeight = min(HEADER(pmt->pbFormat)->biHeight, m_opts.aoiHeight);
        }

        if (IsEqualGUID(*m_mediaType.Subtype(), MEDIASUBTYPE_RGB24))
        {
            VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)m_mediaType.Format();

            if (pvi->bmiHeader.biBitCount == 24)
            {
                if (m_pAnalysis)
                    delete m_pAnalysis;

                m_pAnalysis = new ImageAnalysisRGB(pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight, m_opts);

                return NOERROR;
            }
        }
        else if (IsEqualGUID(*m_mediaType.Subtype(), MEDIASUBTYPE_YUY2))
        {
            VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)m_mediaType.Format();
            return NOERROR;
        }
    }
    return NOERROR;
} // SetMediaType

//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CPrintAnalysisFilter::GetPages(CAUUID* pPages)
{
    CheckPointer(pPages, E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_PrintAnalysisPropertyPage;
    return NOERROR;

} // GetPages

//
// get_AnalysisOptions
//
// Return the current effect selected
//
STDMETHODIMP CPrintAnalysisFilter::get_AnalysisOptions(AnalysisOpts& opts)
{
    CAutoLock cAutolock(&m_filterLock);

    opts = m_opts;

    return NOERROR;

} // get_IPEffect


//
// put_AnalysisOptions
//
// Set the required video effect
//
STDMETHODIMP CPrintAnalysisFilter::put_AnalysisOptions(AnalysisOpts opts)
{
    CAutoLock cAutolock(&m_filterLock);

    m_opts = opts;
    
    if (m_pAnalysis)
        m_pAnalysis->SetAnalysisOpts(opts);

    if (m_mediaType.formattype == FORMAT_VideoInfo)
    {
        // resize our window to the default capture size
        m_opts.aoiHeight = min(HEADER(m_mediaType.pbFormat)->biHeight, m_opts.aoiHeight);
    }

    SetDirty(TRUE);
    return NOERROR;
} // put_IPEffect

//
// WriteToStream
//
// Overriden to write our state into a stream
//
HRESULT CPrintAnalysisFilter::WriteToStream(IStream* pStream)
{
    HRESULT hr;

    WRITEOUT(m_opts);
    
    return NOERROR;

} // WriteToStream


//
// ReadFromStream
//
// Likewise overriden to restore our state from a stream
//
HRESULT CPrintAnalysisFilter::ReadFromStream(IStream* pStream)
{
    HRESULT hr;

    READIN(m_opts);

    if (m_pAnalysis)
        m_pAnalysis->SetAnalysisOpts(m_opts);
    
    return NOERROR;

} // ReadFromStream

//
// GetClassID
//
// This is the only method of IPersist
//
STDMETHODIMP CPrintAnalysisFilter::GetClassID(CLSID* pClsid)
{
    return CBaseFilter::GetClassID(pClsid);

} // GetClassID
