//
// Created by Maurik Holtrop on 5/28/22.
//

#include "RasterMonDefaultConfig.h"
// Calibrations:
double gX_Scale = 0.0104;
double gX_Offset = -0.011;
double gY_Scale = -0.011;
double gY_Offset = 23.9;

void Default_Setup_Raster_Tab(RasterHists *r){
   r->fTabs.emplace_back("Raster", 2, 2);


   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1,
                          "Raster_x", "Raster Pos x;x[mm]", 400, -10., 10.);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gX_Offset;
   TH1D *x_ref = r->fHists.back().GetHist();
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 3,
                          "Raster_y", "Raster Pos y;y[mm]", 400, -10., 10.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.back().scale_x = gY_Scale;    // Note this is the Y scale and offset.
   r->fHists.back().offset_x = gY_Offset;
   TH1D *y_ref = r->fHists.back().GetHist();
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1,  RASTER_CRATE, RASTER_SLOT, 3,
                          "Raster_xy", "Raster Pos y vs x;x[mm];y[xx]", 400, -10., 10.,  400, -10., 10.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 2.
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gY_Offset;
   r->fHists.back().scale_y = gY_Scale;
   r->fHists.back().offset_y = gY_Offset;
   r->fHists.back().draw_opt = "colz";
   r->fHists.back().hist->SetStats(false);

   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, "Raster_r", "Raster Radius;r[mm]", 400, 0., 15.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(3); // Show on pad 3.
   // Special case - 2 inputs but a 1-D histogram
   r->fHists.back().special_fill = kHist_Special_Fill_Radius;
   r->fHists.back().bank_tag2 = RASTER_CRATE;
   r->fHists.back().slot2 = RASTER_SLOT;
   r->fHists.back().adc_chan2 = 3;
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gY_Offset;
   r->fHists.back().scale_y = gY_Scale;
   r->fHists.back().offset_y = gY_Offset;
   r->fHists.back().x_ref_hist = x_ref;
   r->fHists.back().y_ref_hist = y_ref;
   r->fHists.back().hist->SetLineColor(kRed);
}

void Default_Setup_RasterBit32_Tab(RasterHists *r){
   r->fTabs.emplace_back("RasterPulser", 2, 2);

   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1,
                          "RasterPulser_x", "Raster Pos x, Pulser only;x[mm]", 400, -10., 10.);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gX_Offset;
   TH1D *x_ref = r->fHists.back().GetHist();
   r->fHists.back().trigger_bits = 1<<31;
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 3,
                          "RasterPulser_y", "Raster Pos y, Pulser only;y[mm]", 400, -10., 10.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.back().scale_x = gY_Scale;    // Note this is the Y scale and offset.
   r->fHists.back().offset_x = gY_Offset;
   TH1D *y_ref = r->fHists.back().GetHist();
   r->fHists.back().trigger_bits = 1<<31;
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1,  RASTER_CRATE, RASTER_SLOT, 3,
                          "RasterPulser_xy", "Raster Pos y vs x, Pulser only;x[mm];y[xx]", 400, -10., 10.,  400, -10., 10.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 2.
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gY_Offset;
   r->fHists.back().scale_y = gY_Scale;
   r->fHists.back().offset_y = gY_Offset;
   r->fHists.back().draw_opt = "colz";
   r->fHists.back().trigger_bits = 1<<31;
   r->fHists.back().hist->SetStats(false);

   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, "RasterPulser_r", "Raster Radius, Pulser only;r[mm]", 400, 0., 15.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(3); // Show on pad 3.
   // Special case - 2 inputs but a 1-D histogram
   r->fHists.back().special_fill = kHist_Special_Fill_Radius;
   r->fHists.back().bank_tag2 = RASTER_CRATE;
   r->fHists.back().slot2 = RASTER_SLOT;
   r->fHists.back().adc_chan2 = 3;
   r->fHists.back().scale_x = gX_Scale;
   r->fHists.back().offset_x = gY_Offset;
   r->fHists.back().scale_y = gY_Scale;
   r->fHists.back().offset_y = gY_Offset;
   r->fHists.back().hist->SetLineColor(kRed);
   r->fHists.back().x_ref_hist = x_ref;
   r->fHists.back().y_ref_hist = y_ref;
   r->fHists.back().trigger_bits = 1<<31;
}

void Default_Setup_Raw_Raster_Tab(RasterHists *r){
   r->fTabs.emplace_back("Raw1", 2, 2);
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1,
                          "RawIx", "Raw ADC 3, I_x;ADC(1) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 3,
                          "RawIy", "Raw ADC 1, I_{y};ADC(3) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, RASTER_CRATE, RASTER_SLOT, 3,
                          "RawIxy", "Raw ADC 3-2, I_{y} vs I_{x};ADC(1) channel;ADC(3) channel", 409, -0.5, 4095.5, 409, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 4.
   r->fHists.back().draw_opt = "colz";
   r->fHists.back().hist->SetStats(false);
//   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 5, RASTER_CRATE, RASTER_SLOT, 1,
//                          "RawIGx", "Raw ADC 3-2, G(x) vs I_{x};ADC(1) channel; ADC(5) channel", 409, -0.5, 4095.5, 409, -0.5, 4095.5);
//   r->fTabs.back().hists.push_back(r->fHists.size()-1);
//   r->fTabs.back().hist_pads.push_back(3); // Show on pad 3.
//   r->fHists.back().draw_opt = "colz";
//   r->fHists.back().hist->SetStats(false);
}

void Default_Setup_Raw_Raster_Tab2(RasterHists *r){
   r->fTabs.emplace_back("Raw2", 2, 2);
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 5,
                          "RawGx", "Raw ADC 5, G(x); ADC(5) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 7,
                          "RawGy", "Raw ADC 7, G(y); ADC(7) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 5, RASTER_CRATE, RASTER_SLOT, 7,
                          "RawGxy", "Raw ADC 7-5, G(y) vs G(x)", 409, -0.5, 4095.5, 409, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 2.
   r->fHists.back().draw_opt = "colz";
   r->fHists.back().hist->SetStats(false);
//   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 3, RASTER_CRATE, RASTER_SLOT, 7,
//                          "RawIGy", "Raw ADC 7-5, G(y) vs I_{y}", 409, -0.5, 4095.5, 409, -0.5, 4095.5);
//   r->fTabs.back().hists.push_back( r->fHists.size()-1);
//   r->fTabs.back().hist_pads.push_back(3); // Show on pad 3.
//   r->fHists.back().draw_opt = "colz";
//   r->fHists.back().hist->SetStats(false);

}
void Default_Setup_Raw_Raster_Tab3(RasterHists *r){
   r->fTabs.emplace_back("Raw3", 2, 2);
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 9,
                          "RawVx", "Raw ADC 9, G(x); ADC(5) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 11,
                          "RawVy", "Raw ADC 11, G(y); ADC(7) channel", 4096, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.emplace_back(RASTER_CRATE, RASTER_SLOT, 9, RASTER_CRATE, RASTER_SLOT, 11,
                          "RawVxy", "Raw ADC 11-9, G(y) vs G(x)", 409, -0.5, 4095.5, 409, -0.5, 4095.5);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 2.
   r->fHists.back().draw_opt = "colz";
   r->fHists.back().hist->SetStats(false);
}

void Default_Setup_1_Channel_Scope(RasterHists *r){
   r->fTabs.emplace_back("Scope", 1, 1, 0, 0);
   r->fTabs.back().grid = {1};
   r->fTabs.back().calc = {0}; // Calc 1 - modify the bottom margin.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, "IX0", ";t (s)", "I_{x}", kRed+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(0); // Show on pad 0.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 3,"IY0", ";t(s)", "I_{y}", kGreen+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(0); // Show on pad 0.
   r->fTabs.back().pad_link = {1};
}


void Default_Setup_2_Channel_Scope(RasterHists *r){
   r->fTabs.emplace_back("Scope", 1, 2, 0, 0);
   r->fTabs.back().grid = {1, 1};
   r->fTabs.back().calc = {1, 0}; // Calc 1 - modify the bottom margin.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, "IX0", ";t (s)", "I_{x}", kRed+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(1); // Show on pad 1.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 3, "IY0", ";t(s)", "I_{y}", kGreen+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(1); // Show on pad 1.

   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 5,
                           "GX1", ";t(s)", "G(x)", kBlue+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(2); // Show on pad 2.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 7,
                           "GY1", ";t(s)", "G(y)", kMagenta+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(2); // Show on pad 2.
   r->fTabs.back().pad_link = {2, 1};
}

void Default_Setup_3_Channel_Scope(RasterHists *r){
   r->fTabs.emplace_back("Scope", 1, 3, 0, 0);
   r->fTabs.back().grid = {1, 1, 1};
   r->fTabs.back().calc = {1, 1, 0}; // Calc 1 - modify the bottom margin.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 1, ""
                                      "IX0", ";t (s)", "I_{x}", kRed+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(1); // Show on pad 1.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 3,
                           "IY0", ";t(s)", "I_{y}", kGreen+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(1); // Show on pad 1.

   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 5,
                           "GX1", ";t(s)", "G(x)", kBlue+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(2); // Show on pad 2.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 7,
                           "GY1", ";t(s)", "G(y)", kMagenta+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(2); // Show on pad 2.

   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 9,
                           "VX1", ";t(s)", "V_{x}", kAzure+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(3); // Show on pad 3.
   r->fGraphs.emplace_back(RASTER_CRATE, RASTER_SLOT, 11,
                           "VY1", ";t(s)", "V_{y}", kViolet+1, 2, true);
   r->fTabs.back().graphs.push_back(r->fGraphs.size() - 1);
   r->fTabs.back().graph_pads.push_back(3); // Show on pad 3.
   r->fTabs.back().pad_link = {2, 3, 1};
}

void Default_Setup_Helicity(RasterHists *r){
   r->fTabs.emplace_back("Helicity", 2, 2);
   r->fTabs.back().logy = {true, true, true, true};

   r->fHelicity_stack = new THStack("Helicity_stack","Helicity Histograms");
   r->fHelicity_legend = new TLegend(0.9,0.8,1.0 ,1.0);

   r->fHists.emplace_back(19, 19, 0,
                          "HelicityRaw", "Helicity raw", 256, -2, 4097.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.

   r->fHists.emplace_back(19, 19, 2,
                          "SyncRaw", "Sync raw", 256, -2, 4097.);
   r->fTabs.back().hists.push_back( r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(2); // Show on pad 2.

   r->fHists.emplace_back(19, 19, 4,
                          "QuartedRaw", "Quarted raw", 256, -2, 4097.);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(3); // Show on pad 3.

   r->fHists.emplace_back(19, 19, 0,              // Special - do not fill with raw info.
                          "Helicity", "Helicity", 3, -1.5, 1.5);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.back().hist->SetFillColor(kRed);
   r->fHists.back().special_fill = kHist_Special_Fill_Helicity;
   r->fHists.back().special_draw = kHist_Special_Draw_NoDraw;
   r->fHelicity_stack->Add(r->fHists.back().GetHist());
   r->fHelicity_legend->AddEntry(r->fHists.back().GetHist(), "Helicity");

   r->fHists.emplace_back(19, 19, 2,              // Special - do not fill with raw info.
                          "Sync", "Sync", 3, -1.5, 1.5);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.back().hist->SetFillColor(kGreen);
   r->fHists.back().special_fill = kHist_Special_Fill_Helicity;
   r->fHists.back().special_draw = kHist_Special_Draw_NoDraw;
   r->fHelicity_stack->Add(r->fHists.back().GetHist());
   r->fHelicity_legend->AddEntry(r->fHists.back().GetHist(), "Sync");

   r->fHists.emplace_back(19, 19, 4,              // Special - do not fill with raw info.
                          "Quartet", "Quartet", 3, -1.5, 1.5);
   r->fTabs.back().hists.push_back(r->fHists.size()-1);
   r->fTabs.back().hist_pads.push_back(4); // Show on pad 4.
   r->fHists.back().hist->SetFillColor(kBlue);
   r->fHists.back().special_fill = kHist_Special_Fill_Helicity;
   r->fHists.back().special_draw = kHist_Special_Draw_Stack;
   r->fHelicity_stack->Add(r->fHists.back().GetHist());
   r->fHelicity_legend->AddEntry(r->fHists.back().GetHist(), "Quartet");
}

void Default_Setup_Trigger(RasterHists *r){
   /////////////// Trigger Tab
   r->fTabs.emplace_back("Trigger", 1, 2);
   r->fTabs.back().logy = {true, true};
   r->fTabs.back().calc = {0, 0}; // Calc 1 - modify the bottom margin.

   r->fHists.emplace_back(0, 0, 1, "TriggerBits", "Trigger Bits", 32, -0.5, 31.5);
   int hist1 =  r->fHists.size()-1;
   r->fTabs.back().hists.push_back(hist1);
   r->fTabs.back().hist_pads.push_back(1); // Show on pad 1.
   r->fHists.back().hist->SetFillColor(kBlue);
   r->fHists.back().special_fill = kHist_Special_Fill_Trigger;

//   r->fHists.emplace_back(0, 0, 3, "TriggerBits25", "Trigger Bit 25", 32, -0.5, 31.5);
//   int hist2 =  r->fHists.size()-1;
//   r->fTabs.back().hists.push_back(hist2);
//   r->fTabs.back().hist_pads.push_back(2);
//   r->fHists.back().hist->SetFillColor(kGreen);
//   r->fHists.back().special_fill = kHist_Special_Fill_Trigger;
//   r->fHists.back().trigger_bits = 1<<25;

   r->fHists.emplace_back(0, 0, 2, "TriggerBits31", "Trigger Bit 31", 32, -0.5, 31.5);
   int hist2 = r->fHists.size()-1;
   r->fTabs.back().hists.push_back(hist2);
   r->fTabs.back().hist_pads.push_back(1);
   r->fHists.back().hist->SetFillColor(kRed);
   r->fHists.back().special_fill = kHist_Special_Fill_Trigger;
   r->fHists.back().trigger_bits = 1<<31;
   r->fHists.back().hist->SetStats(false);

   r->fTabs.back().hists.push_back(hist2);
   r->fTabs.back().hist_pads.push_back(2);
}

void Default_Initialize_Histograms(RasterHists *r){
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

   // Note: PAD numbering starts at 1, with 0 being the Canvas (i.e. only one object on the screen)
   Default_Setup_Raster_Tab(r);
   Default_Setup_RasterBit32_Tab(r);
   Default_Setup_Raw_Raster_Tab(r);
   Default_Setup_1_Channel_Scope(r);
   Default_Setup_Helicity(r);
   Default_Setup_Trigger(r);
}
