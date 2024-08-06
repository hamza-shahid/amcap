#ifndef __PRINT_ANALYSIS_OPTIONS__
#define __PRINT_ANALYSIS_OPTIONS__

#include "ImageAnalysis.h"

#ifdef __cplusplus
extern "C" {
#endif

    // {AD6DA9A5-4FCA-4766-961C-6E1C9ED599B6}
    DEFINE_GUID(IID_IAnalysisOptions ,
        0xad6da9a5, 0x4fca, 0x4766, 0x96, 0x1c, 0x6e, 0x1c, 0x9e, 0xd5, 0x99, 0xb6);

    DECLARE_INTERFACE_(IAnalysisOptions, IUnknown)
    {
        STDMETHOD(get_AnalysisOptions) (THIS_
            ImageAnalysis::AnalysisOpts &opts
            ) PURE;

        STDMETHOD(put_AnalysisOptions) (THIS_
            ImageAnalysis::AnalysisOpts opts
            ) PURE;
    };

#ifdef __cplusplus
}
#endif

#endif // __PRINT_ANALYSIS_OPTIONS__

