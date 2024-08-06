#ifndef __PRINT_ANALYSIS_PROPERTIES__
#define __PRINT_ANALYSIS_PROPERTIES__

#include <strsafe.h>

#include "paoptions.h"


class CPrintAnalysisProperties : public CBasePropertyPage
{

public:

    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr);

private:

    INT_PTR OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT OnConnect(IUnknown* pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void    GetControlValues();
    
    void    GetControlValue(int Id, int& value);
    void    SetControlValue(int Id, int value);

    CPrintAnalysisProperties(LPUNKNOWN lpunk, HRESULT* phr);

    BOOL m_bIsInitialized;                      // Used to ignore startup messages
    ImageAnalysis::AnalysisOpts m_opts;
    
    IAnalysisOptions* m_pAnalysisOptions;       // The custom interface on the filter

}; // CPrintAnalysisProperties

#endif  // __PRINT_ANALYSIS_PROPERTIES__