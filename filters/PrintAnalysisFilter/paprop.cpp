//------------------------------------------------------------------------------
// File: EZProp.cpp
//
// Desc: DirectShow sample code - implementation of CEZRgb24Properties class.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <olectl.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include "resource.h"
#include "paprop.h"


//
// CreateInstance
//
// Used by the DirectShow base classes to create instances
//
CUnknown* CPrintAnalysisProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT* phr)
{
    ASSERT(phr);

    CUnknown* punk = new CPrintAnalysisProperties(lpunk, phr);

    if (punk == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return punk;

} // CreateInstance


//
// Constructor
//
CPrintAnalysisProperties::CPrintAnalysisProperties(LPUNKNOWN pUnk, HRESULT* phr) 
    : CBasePropertyPage(NAME("Print Analysis Property Page"), pUnk, IDD_PRINT_ANALYSIS, IDS_TITLE)
    , m_pAnalysisOptions(NULL)
    , m_bIsInitialized(FALSE)
{
    ASSERT(phr);

} // (Constructor)


//
// OnReceiveMessage
//
// Handles the messages for our property window
//
INT_PTR CPrintAnalysisProperties::OnReceiveMessage(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        if (m_bIsInitialized)
        {
            m_bDirty = TRUE;
            if (m_pPageSite)
            {
                m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
            }
        }
        return (LRESULT)1;
    }

    }

    return CBasePropertyPage::OnReceiveMessage(hwnd, uMsg, wParam, lParam);

} // OnReceiveMessage


//
// OnConnect
//
// Called when we connect to a transform filter
//
HRESULT CPrintAnalysisProperties::OnConnect(IUnknown* pUnknown)
{
    CheckPointer(pUnknown, E_POINTER);
    ASSERT(m_pAnalysisOptions == NULL);
    
    HRESULT hr = pUnknown->QueryInterface(IID_IAnalysisOptions, (void**)&m_pAnalysisOptions);
    if (FAILED(hr)) {
        return E_NOINTERFACE;
    }

    // Get the initial image FX property
    CheckPointer(m_pAnalysisOptions, E_FAIL);
    m_pAnalysisOptions->get_AnalysisOptions(m_opts);
    
    m_bIsInitialized = FALSE;
    return NOERROR;

} // OnConnect


//
// OnDisconnect
//
// Likewise called when we disconnect from a filter
//
HRESULT CPrintAnalysisProperties::OnDisconnect()
{
    // Release of Interface after setting the appropriate old effect value
    if (m_pAnalysisOptions)
    {
        m_pAnalysisOptions->Release();
        m_pAnalysisOptions = NULL;
    }
    return NOERROR;

} // OnDisconnect


void CPrintAnalysisProperties::SetControlValue(int Id, int value)
{
    TCHAR sz[60];

    (void)StringCchPrintf(sz, NUMELMS(sz), TEXT("%d\0"), value);
    Edit_SetText(GetDlgItem(m_Dlg, Id), sz);
}

//
// OnActivate
//
// We are being activated
//
HRESULT CPrintAnalysisProperties::OnActivate()
{
    /*
    TCHAR sz[60];
    
    (void)StringCchPrintf(sz, NUMELMS(sz), TEXT("%f\0"), m_opts.effectTime);
    //Edit_SetText(GetDlgItem(m_Dlg, IDC_LENGTH), sz);

    (void)StringCchPrintf(sz, NUMELMS(sz), TEXT("%f\0"), m_opts.effectStartTime);
    //Edit_SetText(GetDlgItem(m_Dlg, IDC_START), sz);
    */
    CheckRadioButton(m_Dlg, IDC_INTENSITY, IDC_NONE, m_opts.effect);
    CheckRadioButton(m_Dlg, IDC_BLACK_WHOLE, IDC_BLACK_NONE, m_opts.blackoutType);
    CheckRadioButton(m_Dlg, IDC_GRAY_WHOLE, IDC_GRAY_NONE, m_opts.grayscaleType);
    
    SetControlValue(IDC_EDIT_AOI_HEIGHT, m_opts.aoiHeight);
    SetControlValue(IDC_EDIT_AOI_PART, m_opts.aoiPartitions);
    
    Button_SetCheck(GetDlgItem(m_Dlg, IDC_CHECK_CONNECT_VALUES), m_opts.connectValues);

    m_bIsInitialized = TRUE;

    return NOERROR;

} // OnActivate


//
// OnDeactivate
//
// We are being deactivated
//
HRESULT CPrintAnalysisProperties::OnDeactivate(void)
{
    ASSERT(m_pAnalysisOptions);

    m_bIsInitialized = FALSE;
    GetControlValues();

    return NOERROR;

} // OnDeactivate


//
// OnApplyChanges
//
// Apply any changes so far made
//
HRESULT CPrintAnalysisProperties::OnApplyChanges()
{
    GetControlValues();

    CheckPointer(m_pAnalysisOptions, E_POINTER);
    m_pAnalysisOptions->put_AnalysisOptions(m_opts);

    return NOERROR;

} // OnApplyChanges


void CPrintAnalysisProperties::GetControlValue(int Id, int& value)
{
    TCHAR sz[STR_MAX_LENGTH];

    Edit_GetText(GetDlgItem(m_Dlg, Id), sz, STR_MAX_LENGTH);

#ifdef UNICODE
    char szANSI[STR_MAX_LENGTH];

    // Convert Multibyte string to ANSI
    WideCharToMultiByte(CP_ACP, 0, sz, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
    value = atoi(szANSI);
#else
    value = atoi(sz);
#endif
}

void CPrintAnalysisProperties::GetControlValues()
{
/*    TCHAR sz[STR_MAX_LENGTH];
    REFTIME tmp1, tmp2;
    
    // Get the start and effect times
    //Edit_GetText(GetDlgItem(m_Dlg, IDC_LENGTH), sz, STR_MAX_LENGTH);

#ifdef UNICODE
    // Convert Multibyte string to ANSI
    char szANSI[STR_MAX_LENGTH];

    int rc = WideCharToMultiByte(CP_ACP, 0, sz, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
    tmp2 = COARefTime(atof(szANSI));
#else
    tmp2 = COARefTime(atof(sz));
#endif

    //Edit_GetText(GetDlgItem(m_Dlg, IDC_START), sz, STR_MAX_LENGTH);

#ifdef UNICODE
    // Convert Multibyte string to ANSI
    rc = WideCharToMultiByte(CP_ACP, 0, sz, -1, szANSI, STR_MAX_LENGTH, NULL, NULL);
    tmp1 = COARefTime(atof(szANSI));
#else
    tmp1 = COARefTime(atof(sz));
#endif

    // Quick validation of the fields
    if (tmp1 >= 0 && tmp2 >= 0) {
        m_opts.effectStartTime = COARefTime(tmp1);
        m_opts.effectTime = COARefTime(tmp2);
    }
 */   
    GetControlValue(IDC_EDIT_AOI_HEIGHT, m_opts.aoiHeight);
    GetControlValue(IDC_EDIT_AOI_PART, m_opts.aoiPartitions);

    m_opts.connectValues    = Button_GetCheck(GetDlgItem(m_Dlg, IDC_CHECK_CONNECT_VALUES));

    // Find which special effect we have selected
    for (int i = IDC_INTENSITY; i <= IDC_NONE; i++)
    {
        if (IsDlgButtonChecked(m_Dlg, i))
        {
            m_opts.effect = i;
            break;
        }
    }

    // Find which blackout type we have selected
    for (int i = IDC_BLACK_WHOLE; i <= IDC_BLACK_NONE; i++)
    {
        if (IsDlgButtonChecked(m_Dlg, i))
        {
            m_opts.blackoutType = i;
            break;
        }
    }

    // Find which grayscale type we have selected
    for (int i = IDC_GRAY_WHOLE; i <= IDC_GRAY_NONE; i++)
    {
        if (IsDlgButtonChecked(m_Dlg, i))
        {
            m_opts.grayscaleType = i;
            break;
        }
    }
}
