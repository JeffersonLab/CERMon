//
// Created by Maurik Holtrop on 6/14/22.
//
#include "RasterMonConfigPanel.h"

RasterMonConfigPanel::RasterMonConfigPanel(const RasterMonGui *parent, RasterMonConfigInfo *info):
               fRasGui(parent), fInfo(info), TGTransientFrame(gClient->GetRoot(), (TGWindow *) parent, 400, 200){
   // Handle the user input for configurable settings of the program.
   // Run the main bit of gui code.
   // update_rate and clear_rate are passed, because the full RaterMonGui class is not known.
   // and we have RasterMonGui include RasterMonConfig.
   //
   int frame_width = 400;
   int frame_height = 400;
   int label_width = 190;
   int field_width = 200;

   fInfo->GetValues();

   SetWindowName("RasterMon Configure Dialog");
   Connect("CloseWindow()", "RasterMonConfigPanel", this, "CloseWindow()");
   DontCallClose(); // to avoid double deletions.
   // use hierarchical cleaning
   SetCleanup(kDeepCleanup);
   // Bottom buttons:  Load, Save, Close
   auto Frame1 = new TGHorizontalFrame(this, frame_width, 20, kFixedWidth);

   // ------------- Bottom button row -------------

   auto CloseButton = new TGTextButton(Frame1, "&Close", 3);
   CloseButton->Connect("Clicked()", "RasterMonGui", (TGWindow *) fRasGui, "CloseConfigure()");
   Frame1->AddFrame(CloseButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 10, 2, 4));

   auto SaveButton = new TGTextButton(Frame1, "&Save", 2);
   SaveButton->Connect("Clicked()", "RasterMonConfigInfo", fInfo, "SaveToJSON()");
   Frame1->AddFrame(SaveButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 10, 2, 4));

   auto LoadButton = new TGTextButton(Frame1, "&Load", 1);
   LoadButton->Connect("Clicked()", "RasterMonConfigInfo", fInfo, "LoadFromJSON()");
   LoadButton->Connect("Clicked()", "RasterMonConfigPanel", this, "UpdateDisplay()");
   Frame1->AddFrame(LoadButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 10, 2, 4));

   //  Frame1->AddFrame(CancelButton, L1);
   Frame1->Resize(400, CloseButton->GetDefaultHeight());

   AddFrame(Frame1, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));

   // -------------- Main Dialog Frame --------------

   // Vertically stacked items, i.e. followed by TGHorizontalFrames for filling the rows.
   auto Frame2 = new TGVerticalFrame(this, frame_width, frame_height, kFixedWidth | kFixedHeight);

   auto *h_frame0 = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame0, new TGLayoutHints(kLHintsExpandX, 10, 2, 10, 2));
   auto label0 = new TGLabel(h_frame0, "For numeric entry, use shift key for x10, crtl key for x100");
   h_frame0->AddFrame(label0, new TGLayoutHints(kLHintsExpandX, 10, 2, 10, 2));

   // ********** Debug Level Set  RasterMon ***********
   auto *h_frame_debug_level = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame_debug_level, new TGLayoutHints(kLHintsExpandX, 10, 2, 10, 2));
   auto label_debug_level = new TGLabel(h_frame_debug_level, "RasterMon Gui Debug Level:");
   h_frame_debug_level->AddFrame(label_debug_level, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 10, 5, 5));

   fDebugLevelChooser = new TGListBox(h_frame_debug_level);
   fDebugLevelChooser->AddEntry("Quiet, no debug - 0", 0);
   fDebugLevelChooser->AddEntry("Level 1 debug - 1", 1);
   fDebugLevelChooser->AddEntry("Level 2 debug - 2", 2);
   fDebugLevelChooser->AddEntry("Level 3 debug - 3", 3);
   fDebugLevelChooser->AddEntry("Level 4 debug - 4", 4);

   fDebugLevelChooser->Resize(150, 50);
   h_frame_debug_level->AddFrame(fDebugLevelChooser,
                                 new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5,
                                                   5, 5));

   fDebugLevelChooser->Connect("Selected(Int_t)", "RasterMonConfigInfo", fInfo, "SetDebug(int)");
   fDebugLevelChooser->Connect("Selected(Int_t)", "RasterMonGui", (TGWindow *) fRasGui, "SetDebug(int)");

   // ********** Debug Level Set  EVIO ***********

   auto *h_frame_debug_level2 = new TGGroupFrame(Frame2, new TGString("Evio System Debug:"),
                                                 kHorizontalFrame | kRaisedFrame);
   //auto *h_frame_debug_level2 = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame_debug_level2, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 2));

   fEvioDebugInfo1 = new TGCheckButton(h_frame_debug_level2, "Info L1", 1);
   h_frame_debug_level2->AddFrame(fEvioDebugInfo1, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 1, 1, 5, 0));

   fEvioDebugInfo2 = new TGCheckButton(h_frame_debug_level2, "Info L2", 2);
   h_frame_debug_level2->AddFrame(fEvioDebugInfo2, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 1, 1, 5, 0));

   fEvioDebug1 = new TGCheckButton(h_frame_debug_level2, "Debug L1", 3);
   h_frame_debug_level2->AddFrame(fEvioDebug1, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 1, 1, 5, 0));

   fEvioDebug2 = new TGCheckButton(h_frame_debug_level2, "Debug L2", 4);
   h_frame_debug_level2->AddFrame(fEvioDebug2, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 1, 1, 5, 0));

   fEvioDebugInfo1->Connect("Clicked()", "RasterMonConfigPanel", this, "HandleButtons()");
   fEvioDebugInfo2->Connect("Clicked()", "RasterMonConfigPanel", this, "HandleButtons()");
   fEvioDebug1->Connect("Clicked()", "RasterMonConfigPanel", this, "HandleButtons()");
   fEvioDebug2->Connect("Clicked()", "RasterMonConfigPanel", this, "HandleButtons()");

   // ********** Refresh Rate Set Box **********

   auto *h_frame1 = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame1, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 1));

   auto *h_frame1_1 = new TGHorizontalFrame(h_frame1, label_width, 20, kFixedWidth);
   h_frame1->AddFrame(h_frame1_1);
   auto label1 = new TGLabel(h_frame1_1, "Window Refresh rate:     ");
   h_frame1_1->AddFrame(label1, new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kFixedWidth, 10, 55, 5, 5));

   fEnterUpdateRate = new TGNumberEntry(h_frame1, fInfo->fUpdateRate, 10, 1, TGNumberFormat::kNESInteger,
                                        TGNumberFormat::kNEAPositive, TGNumberFormat::kNELLimitMin, 50, 10000);
   h_frame1->AddFrame(fEnterUpdateRate, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,
                                                          1, 1, 5, 5));

   auto label1tail = new TGLabel(h_frame1, " ms/frame");
   h_frame1->AddFrame(label1tail, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 1, 5, 5));

   fEnterUpdateRate->Connect("ValueSet(Long_t)", "RasterMonGui", (TGWindow *) fRasGui, "SetUpdateRate()");
   fEnterUpdateRate->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");

   // ********** Auto-Clear Set Box ************

   auto *h_frame3 = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame3, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 1));
   auto *h_frame3_1 = new TGHorizontalFrame(h_frame3, label_width, 20, kFixedWidth);
   h_frame3->AddFrame(h_frame3_1);

   auto label3 = new TGLabel(h_frame3_1, "Auto Clear Rate: ");
   h_frame3_1->AddFrame(label3, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 10, 5, 5));

   fAutoClearOnOff = new TGCheckButton(h_frame3_1, "on/off", 5);
   h_frame3_1->AddFrame(fAutoClearOnOff, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0));
   fAutoClearOnOff->Connect("Clicked()", "RasterMonConfigPanel", this, "HandleButtons()");
   fAutoClearOnOff->SetState((EButtonState)fInfo->fAutoClearOn);

   fNumberEntryAutoClear = new TGNumberEntry(h_frame3, fInfo->fAutoClearRate, 10, 2, TGNumberFormat::kNESRealOne,
                                             TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELNoLimits);
   h_frame3->AddFrame(fNumberEntryAutoClear, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,
                                                               1, 1, 5, 5));

   auto label3tail = new TGLabel(h_frame3, " s/clear");
   h_frame3->AddFrame(label3tail, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 1, 5, 5));
   fNumberEntryAutoClear->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");

   // ********** Buffer Depth Set Box **********

   auto *h_frame2 = new TGHorizontalFrame(Frame2);
   Frame2->AddFrame(h_frame2, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 1));
   auto *h_frame2_1 = new TGHorizontalFrame(h_frame2, label_width, 20, kFixedWidth);
   h_frame2->AddFrame(h_frame2_1);

   auto label2 = new TGLabel(h_frame2_1, "Scope event buffer depth:");
   h_frame2_1->AddFrame(label2, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 20, 5, 5));

   fNumberEntryScopeBufDepth = new TGNumberEntry(h_frame2, double(fInfo->fScopeBufDepth) / 1000., 10, 7,
                                                 TGNumberFormat::kNESRealOne, TGNumberFormat::kNEAPositive,
                                                 TGNumberFormat::kNELLimitMinMax, 0.2, 1000.0);

   // fNumberEntryScopeBufDepth->Resize( 60, 20);
   h_frame2->AddFrame(fNumberEntryScopeBufDepth, new TGLayoutHints(kLHintsLeft | kLHintsCenterY,
                                                                   1, 1, 5, 5));
   auto label2tail = new TGLabel(h_frame2, "* 1k evt.");
   h_frame2->AddFrame(label2tail, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 1, 5, 5));

   fNumberEntryScopeBufDepth->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");


   // ********************** Raster Conversion Numbers.

   auto *h_frame_raster_conv = new TGGroupFrame(Frame2, new TGString("Raster Conversion:"),
                                                kVerticalFrame | kRaisedFrame);
   Frame2->AddFrame(h_frame_raster_conv, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 2));

   auto *h_frame_raster_row = new TGHorizontalFrame(h_frame_raster_conv);
   h_frame_raster_conv->AddFrame(h_frame_raster_row, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 2));

   auto label_x = new TGLabel(h_frame_raster_row, "f(x) = ");
   h_frame_raster_row->AddFrame(label_x, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));
   fEnterScale_x = new TGNumberEntry(h_frame_raster_row, fInfo->fScale_x, 12, 3,
                                     TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber,
                                     TGNumberFormat::kNELNoLimits);
   fEnterScale_x->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");
   h_frame_raster_row->AddFrame(fEnterScale_x, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));
   auto label_x2 = new TGLabel(h_frame_raster_row, "*x + ");
   h_frame_raster_row->AddFrame(label_x2, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));

   fEnterOffset_x = new TGNumberEntry(h_frame_raster_row, fInfo->fOffset_x, 12, 4,
                                      TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber,
                                      TGNumberFormat::kNELNoLimits);
   fEnterOffset_x->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");
   h_frame_raster_row->AddFrame(fEnterOffset_x, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));

   h_frame_raster_row = new TGHorizontalFrame(h_frame_raster_conv);
   h_frame_raster_conv->AddFrame(h_frame_raster_row, new TGLayoutHints(kLHintsExpandX, 10, 2, 5, 2));

   auto label_y = new TGLabel(h_frame_raster_row, "f(y) = ");
   h_frame_raster_row->AddFrame(label_y, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));
   fEnterScale_y = new TGNumberEntry(h_frame_raster_row, fInfo->fScale_y, 12, 5,
                                     TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber,
                                     TGNumberFormat::kNELNoLimits);
   fEnterScale_y->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");
   h_frame_raster_row->AddFrame(fEnterScale_y, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));
   auto label_y2 = new TGLabel(h_frame_raster_row, "*y + ");
   h_frame_raster_row->AddFrame(label_y2, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));
   fEnterOffset_y = new TGNumberEntry(h_frame_raster_row, fInfo->fOffset_y, 12, 6,
                                      TGNumberFormat::kNESReal, TGNumberFormat::kNEAAnyNumber,
                                      TGNumberFormat::kNELNoLimits);
   fEnterOffset_y->Connect("ValueSet(Long_t)", "RasterMonConfigPanel", this, "ValueSet()");
   h_frame_raster_row->AddFrame(fEnterOffset_y, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 5, 2, 5, 5));

   UpdateDisplay();

   // ********************** Finish up the Dialog ******************************

   AddFrame(Frame2, new TGLayoutHints(kLHintsTop));
   MapSubwindows();
   UInt_t width = GetDefaultWidth();
   UInt_t height = GetDefaultHeight();
   Resize(width, height);
   CenterOnParent();
   // make the message box non-resizable
   SetWMSize(width, height);
   SetWMSizeHints(width, height, width, height, 0, 0);
   MapWindow();
};


void RasterMonConfigPanel::UpdateDisplay(){
   // Update the information on the display from the items in memory.
   // This is called when loading the data from a file, so also set the actual corresponding value.
   // Because the "emit" does not emit what we think, and so actual values are not updated otherwise.
   fDebugLevelChooser->Select(fInfo->fDebugLevel);
   fEvioDebugInfo1->SetState((EButtonState)((fInfo->fEvioDebugLevel & 1) > 0));
   fEvioDebugInfo2->SetState((EButtonState)((fInfo->fEvioDebugLevel & 2) > 0));
   fEvioDebug1->SetState((EButtonState)((fInfo->fEvioDebugLevel & 4) > 0));
   fEvioDebug2->SetState((EButtonState)((fInfo->fEvioDebugLevel & 8) > 0));
   fEnterUpdateRate->SetIntNumber(fInfo->fUpdateRate);
   fAutoClearOnOff->SetState((EButtonState)fInfo->fAutoClearOn);
   fNumberEntryAutoClear->SetNumber(fInfo->fAutoClearRate);
   fNumberEntryScopeBufDepth->SetNumber(double(fInfo->fScopeBufDepth) / 1000.);
   fEnterScale_x->SetNumber(fInfo->fScale_x);
   fEnterOffset_x->SetNumber(fInfo->fOffset_x);
   fEnterScale_y->SetNumber(fInfo->fScale_y);
   fEnterOffset_y->SetNumber(fInfo->fOffset_y);
   fInfo->PutValues();
}


void RasterMonConfigPanel::ValueSet() {
   // Handle value changed in the Raster conversion section and rate section
   TGNumberEntry *nmbr = (TGNumberEntry *) gTQSender;
   int id = nmbr->WidgetId();
   // std::cout << "Changing the value for " << id << " to " << nmbr->GetNumber() << std::endl;
   if (id == 1) fInfo->fUpdateRate = nmbr->GetNumber();

   if (id == 2){
      fInfo->fAutoClearRate = fNumberEntryAutoClear->GetNumber();
      fInfo->UpdateHistClearRate();
      fAutoClearOnOff->SetState((EButtonState)fInfo->fAutoClearOn);  // Make sure checkbox reflect state.
   }
   if (id == 3) fInfo->fScale_x = nmbr->GetNumber();
   if (id == 4) fInfo->fOffset_x = nmbr->GetNumber();
   if (id == 5) fInfo->fScale_y = nmbr->GetNumber();
   if (id == 6) fInfo->fOffset_y = nmbr->GetNumber();
   if (3 <= id && id <= 6) fInfo->SetScaleOffset();     // Updates the scales on the histograms.
   if (id == 7){
      fInfo->fScopeBufDepth = nmbr->GetNumber() * 1000;
      fInfo->UpdateADCBufDepth();
   }

}

void RasterMonConfigPanel::HandleButtons() {
   // Handle different buttons.

   TGButton *btn = (TGButton *) gTQSender;
   int id = btn->WidgetId();
   // std::cout << "Changing button " << id << " to " << btn->GetState() << std::endl;
   if (id > 0 && id < 5) {    // EVIO Debug buttons.
      int bits = fInfo->fEvioDebugLevel;
      if (btn->GetState()) {
         bits = bits | (1 << (id - 1));
      } else {
         bits = bits & ~(1 << (id - 1));
      }
//      std::bitset<8> b(bits);
//      printf("DoButton: id = %d IsSelected: %d -> ", id, btn->GetState());
//      cout << b << endl;
      fInfo->fEvioDebugLevel = bits;
   }else if(id == 5){  // Auto clear on/off
      fInfo->fAutoClearOn = btn->GetState();
   }
   fInfo->PutValues();
}

