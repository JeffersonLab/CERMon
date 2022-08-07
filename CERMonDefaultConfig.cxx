//
// Created by Maurik Holtrop on 5/28/22.
//

#include "CERMonDefaultConfig.h"

void Default_Initialize_Histograms(CERMonHists *r){
   // This function is called at the start to initialize the histograms.
   // This will setup the structures for the tabs and the channels for the scope and histograms with a default layout.
   //
   // Init with:  (bank_tag, slot, adc_chan, tab_number, pad_number, name, title, legend, color, width, show)
   // Or with:    (adc_chan, tab_number, pad_number, name, title, legend, color, width)

   // Some general styling of the app.
   gStyle->SetPaperSize(TStyle::kUSLetter);   // Output for PDF is US-letter format.

   if(r->fEvio != nullptr) r->fEvio->UpdateBufferSize(10000);   // Set default Scope buffer.
   r->SetAutoClearRate(30.);
   r->SetAutoClearRateOff();

}
