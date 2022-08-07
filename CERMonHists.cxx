//
// Created by Maurik Holtrop on 3/29/22.
//
#include "CerMonHists.h"

CERMonHists::~CERMonHists(){
// Cleanup.
}

TGTab * CERMonHists::AddTabArea(TGWindow *frame, int w, int h) {
   // Add a Tab Area to the main window frame.
   fTabAreaTabs = new TGTab(frame, 1, 1);

   for( auto &tab_info: fTabs) {
      TGCompositeFrame *tab = fTabAreaTabs->AddTab(tab_info.name.c_str());
      // Note: Cannot use "emplace_back", because the copy constructor for TEmbeddedCanvas has been explicitly deleted.
      // This forces us to create the canvas with new.
      auto embedded_canvas = new TRootEmbeddedCanvas(tab_info.name.c_str(), tab, w, h);
      tab->AddFrame(embedded_canvas, new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
                                                       kLHintsExpandY, 5, 5, 5, 1));

      tab_info.canvas = embedded_canvas;
      SetupCanvas(tab_info);
   }
   DoDraw();
   return fTabAreaTabs;
}

void CERMonHists::SetupCanvas(TabSpace_t &tab, TCanvas *canvas){
   // Process the canvasses.
   if( tab.canvas == nullptr && canvas == nullptr){
      cout << "ERROR - Tab has no canvas. ";
      return;
   }

   TCanvas *canv;
   if( canvas == nullptr) canv = dynamic_cast<TCanvas *>(tab.canvas->GetCanvas());
   else canv = canvas;

   if( tab.nx>1 || tab.ny >1) canv->Divide( (Int_t)tab.nx, (Int_t)tab.ny, tab.x_margin, tab.y_margin);
   if( tab.pad_link.size()>0 ) {  // Setup signals for pad resizing.
      for (int i=1; i <= std::min(tab.pad_link.size(), (size_t) 6); i++) {  // Up to 6 pads can be linked.
         auto pad = dynamic_cast<TPad *>(canv->cd(i));
         char slot_name[20];
         sprintf(slot_name, "SubPad%dResized()", i);
         if(fDebug>2) std::cout << "Connecing pad " << i << " RangeChanged to slot " << slot_name << std::endl;
         pad->Connect("RangeChanged()", "RasterHists", this, slot_name);
         pad->Connect("UnZoomed()", "RasterHists", this,  slot_name);
      }
   }
   for(int i=0; i< tab.logy.size(); ++i){    // Set the logy for pads that want this. Can be set/unset by clicking on pad.
      auto pad = canv->cd(i+1);
      pad->SetLogy(tab.logy[i]);
   }

   for(int i=0; i< tab.grid.size(); ++i){    // Set the grid for pads that want this.
      auto pad = canv->cd(i+1);
      pad->SetGrid(tab.grid[i], tab.grid[i]);
   }

   for(int i=0; i< tab.calc.size(); ++i){
      if( tab.calc[i] == 1){
         auto pad = canv->cd(i+1);
         pad->SetBottomMargin(0.04);
      }
   }

   // TODO: Setup the legends for each Pad that wants one.

   //   auto legend = new TLegend(0.9,0.85,1.0,1.0);
   //   legend->AddEntry(fGRaw_x.get(), "I_x");
   //   legend->AddEntry(fGRaw_y.get(),"I_y");
   //   legend->AddEntry(fGRaw2_x.get(),"G(x)");
   //   legend->AddEntry(fGRaw2_y.get(), "G(y)");
   //   legend->Draw();
}

void CERMonHists::SetupData() {
   // For each histogram or graph, fill in the data fetch information so HistFillWorker's can do their job.
   // Link the EVIO data for each histogram to the data store.
   // This also tells (RasterEvioTool *)fEvio which channels to store.
   // We can only run SetupData() *after* the histograms and graphs have been setup, either from a config or with
   // the default function.

   for(auto &h_t: fHists){
      if( h_t.bank_tag == 0) continue;  // Not a real request for data, so a special fill.
      int his_index = fEvio->AddChannel(h_t.bank_tag, h_t.slot, h_t.adc_chan);
      if(his_index>=0) h_t.data_index = his_index;
      if(h_t.Is2D()){
         his_index = fEvio->AddChannel(h_t.bank_tag2, h_t.slot2, h_t.adc_chan2);
         if(his_index>=0) h_t.data_index2 = his_index;
      }
   }

   // Same for the graphs.
   for(auto &g_t: fGraphs){
      if( g_t.bank_tag == 0) continue;  // Not a real request for data, so a special fill.
      int his_index = fEvio->AddChannel(g_t.bank_tag, g_t.slot, g_t.adc_chan);
      if(his_index>=0) g_t.data_index = his_index;
   }
}

TAxis * CERMonHists::GetTopAxisFromPad(TPad *pad){
   // Collect the axes from a pad and return them in a vector.
   // Note that the returned object is
   TAxis * axes;
   auto prims1 = pad->GetListOfPrimitives();
   for(int i=0; i< prims1->GetEntries(); ++i){
      if(strncmp(prims1->At(i)->ClassName(),"TGr",3) == 0){
         auto graph = (TGraph *)prims1->At(i);
         axes = graph->GetXaxis();
         break;
      }else if(strncmp(prims1->At(i)->ClassName(),"TH1",3) == 0){
         auto histo = (TH1 *)prims1->At(i);
         axes = histo->GetXaxis();
         break;
      }
   }
   return axes;
}

void CERMonHists::SubPadCopyRange(TPad *one, TPad *two){
// Set the X axis of TPad two to be the same as that for TPad one.
// Here we assume that each histogram or TGraph has the same number of bins in one and two.
//
   if (fPadSizeIsUpdating){
      // std::cout << "Pad is already updating.\n";
      return;  // This is to make sure we don't call this twice too quickly.
   }
   fPadSizeIsUpdating = true;
   auto axis1 = GetTopAxisFromPad(one);
   auto axis2 = GetTopAxisFromPad(two);
   if(axis1 == nullptr) return;
   if(axis2 == nullptr) return;
   double ax1lo = axis1->GetBinLowEdge(axis1->GetFirst());
   double ax1hi = axis1->GetBinUpEdge(axis1->GetLast());
   double ax2lo = axis2->GetBinLowEdge(axis2->GetFirst());
   double ax2hi = axis2->GetBinUpEdge(axis2->GetLast());

   if(axis1->GetFirst() == axis2->GetFirst() && axis1->GetLast() == axis2->GetLast() &&
      abs(ax1lo - ax2lo ) < 1E5 && abs(ax1hi - ax2hi ) < 1E5 ){
      // std::cout << "Axes already the same.\n";
      fPadSizeIsUpdating = false;
   } else {
      // This zooms and un-zooms okay. BUT, if one graph has no data, the labels do not update the same.
      // Note: The mouse based zooming occurs in "TPad::ExecuteEventAxis". The final mouse up action does axis->SetRange()
      axis2->SetRange(axis1->GetFirst(), axis1->GetLast());
      fPadSizeIsUpdating = false;
      // std::cout << "Signal two modified.\n";
      two->Modified();
   }
}

void CERMonHists::SubPadResized(int i) {
   // We got a signal from Pad i that it has resized.
   // We know you can only resize a pad that is on the current tab. So find the current tab and then
   // see what pad this pad is linked to, then resize the other pad.
   int current_tab = fTabAreaTabs->GetCurrent();
   auto tab = fTabs[current_tab];
   if(i > tab.pad_link.size()){
      // cout << "Wrong tab? " << i << "\n";
      return;  // Wrong tab showing. This can happen during global canvas update.
   }
   auto canvas = tab.canvas->GetCanvas();
   auto pad1 = dynamic_cast<TPad *>(canvas->GetPad(i));
   auto pad2_number = tab.pad_link[i-1];
   if(fDebug>2) std::cout << "SubPadResize -- copying axis range from pad " << i << " to pad " << pad2_number << std::endl;
   if(pad2_number>0 && pad2_number != i) {
      auto pad2 = dynamic_cast<TPad *>(canvas->GetPad(pad2_number));
      SubPadCopyRange(pad1, pad2);
   }
}

void CERMonHists::ResizeScopeGraphs(unsigned long size){
   if(fDebug>1) std::cout << "Resizing the oscilloscope graphs to: " << size << std::endl;
//   fGRaw_x->Expand(size);
////   fGRaw_x->Set(size);
//   fGRaw_y->Expand(size);
////   fGRaw_y->Set(size);
//   fGRaw2_x->Expand(size);
////   fGRaw2_x->Set(size);
//   fGRaw2_y->Expand(size);
//   fGRaw2_y->Set(size);
}

void CERMonHists::DrawCanvas(int tab_no, TCanvas *canvas, vector<Histogram_t> &histograms, vector<Graph_t> &graphs,
                             bool batch) {
   auto &tab = fTabs.at(tab_no);
   TCanvas *canv;
   if( canvas == nullptr ) canv = tab.canvas->GetCanvas();
   else canv = canvas;

   unsigned short max_pads=0;
   for(auto i_h: tab.hist_pads) max_pads = std::max(max_pads, i_h); // get the highest pad number.
   std::vector<int> pad_count(max_pads+1); // for counting pad occurrence. Initialized to zero.

   for(int i=0; i < tab.hists.size(); ++i) {
      int i_h = tab.hists[i];
      auto &h_t = histograms.at(i_h);

      unsigned short pad_number = tab.hist_pads[i];
      pad_count[pad_number]++;
      auto pad = canv->cd(pad_number);
      if(h_t.special_draw == kHist_Special_Draw_Normal){
         string draw_opt = h_t.draw_opt;
         if(pad_count[pad_number]>1) draw_opt += "same";
         fDrawLock.lock();
         gROOT->SetBatch(batch);
         h_t.hist->Draw(draw_opt.c_str());
         gROOT->SetBatch(false);
         fDrawLock.unlock();
         pad->Modified();
      }else if(h_t.special_draw == kHist_Special_Draw_Stack){   // The helicity stack has a separate draw.
         fDrawLock.lock();
         gROOT->SetBatch(batch);
         fHelicity_stack->Draw("nostackb");
         fHelicity_legend->Draw();
         gROOT->SetBatch(false);
         fDrawLock.unlock();
      }
   }

   //
   // To fill the graphs, we need to copy the data from the fEvio->fTimeBuf and fEvio->fAdcAverageBuf to th graph.
   // Because we *link* the x-axis of the graph pads, we want the same number of events in each graph, otherwise
   // such linking becomes a second order ROOT hell.
   // If an fEvio->fTimeBuf is empty, then just fill it with the correct data from another channel and set the
   // fEvio->fAdcAverageBuf to zero.
   // TODO: A problem occurs here when the data on linked pads is of different size. This is probably rare or not
   // TODO: occurring, so we can not worry about it for now.
   max_pads = 0;
   for(auto i_h: tab.graph_pads) max_pads = std::max(max_pads, i_h); // get the highest pad number.
   pad_count.resize(max_pads+1);
   pad_count.assign(max_pads+1, 0);
   for(int i=0; i < tab.graphs.size(); ++i){
      auto i_h = tab.graphs[i];
      unsigned short pad_number = tab.graph_pads[i];
      auto &g_t = graphs.at(i_h);
      pad_count[pad_number]++;
      auto pad = canv->cd(pad_number);
      auto graph = g_t.graph;
      string draw_option = g_t.draw_opt;
      if(pad_count[pad_number]>1) draw_option += "same";
      fDrawLock.lock();
      gROOT->SetBatch(batch);
      graph->Draw(draw_option.c_str());
      gROOT->SetBatch(false);
      fDrawLock.unlock();
      pad->Modified();
   }
   fDrawLock.lock();
   gROOT->SetBatch(batch);
   canv->Update();
   gROOT->SetBatch(false);
   fDrawLock.unlock();
}

void CERMonHists::FillGraphs(int tab_no, vector<Graph_t> &graphs) {
   // Fill the graphs with the information from the fEvio buffers.
   // We don't need to do this every event, only when we update the tab that displays the graphs.
   // This is a relatively expensive operation, and gets pretty slow for big buffers.
   auto &tab = fTabs.at(tab_no);
   for (int i_h: tab.graphs) {
      auto &g_t = graphs.at(i_h);
      auto graph = g_t.graph;
      if (fEvio) {
         auto data_idx = g_t.data_index;
         if (fEvio->fTimeBuf[data_idx].empty()) {
            int not_empty = 0;
            for (; not_empty < tab.graphs.size(); ++not_empty) if (!fEvio->fTimeBuf[not_empty].empty()) break;
            if (not_empty < tab.graphs.size()) { // We found the first not empty graph.
               for (int i = 0; i < fEvio->fTimeBuf[not_empty].size(); ++i) {
                  graph->SetPoint(i, fEvio->fTimeBuf[not_empty].at(i), 0.);
               }
            } else { // All the graphs are empty!
               graph->SetPoint(0, 0., 0.);
               graph->SetPoint(1, 1., 0.);
            }
         } else {
            for (int i = 0; i < fEvio->fTimeBuf[data_idx].size() && i < fEvio->fAdcAverageBuf[data_idx].size(); ++i) {
               graph->SetPoint(i, fEvio->fTimeBuf[data_idx].at(i), fEvio->fAdcAverageBuf[data_idx].at(i));
            }
         }
      }
   }
}

void CERMonHists::Stop(){
   fKeepWorking = false;
   fHistClearTimer->TurnOff();
   for(auto &&worker : fWorkers) {
      if(fIsTryingToRead && fEvio->fReadFromEt){
         // The worker is actively reading. If it is hung (no event from ET) then worker.join() will hang until
         // an event appears on the ET, which may take forever.
         std::this_thread::sleep_for(std::chrono::seconds(1)); // Sleep for a whole 1 second.
         for(int i=10; i>=0 && fIsTryingToRead && fEvio->fReadFromEt; --i ){
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for another 1/2 second.
            std::cout << "The worker thread is still waiting for an event from ET. \n";
            std::cout << "We cannot stop the thread until an event is received. Waiting for " << i << " more seconds.\n";
         }
      }
      if(fIsTryingToRead && fEvio->fReadFromEt) {
         std::cout
               << "Reader thread has hung. We are aborting stop(). This condition will clear with the next event on the ET.\n";
         return;  // <-- We abandon the thread? :-(.
      }
      if(fDebug>1) std::cout << "Joining the worker.\n";
      worker.join();
   }
   fWorkers.clear();
}

void CERMonHists::Go(){
   fPause = false;
   if(fKeepWorking) return;  // Do not allow multiple go() clicks.
   fKeepWorking = true;
   // Start up the thread workers to fill the histogram.
   if(fDebug) std::cout << "Start " <<  fNWorkers << " workers.\n";
   for(int i=0; i< fNWorkers; ++i) {
      fWorkers.emplace_back(&CERMonHists::HistFillWorker, this, i);
   }
   if(fHistClearTimerIsOn) fHistClearTimer->TurnOn();
}

void CERMonHists::DoDraw(int active_tab){
   if(active_tab == -1) {
      for(int i_tab=0; i_tab< fTabs.size(); ++i_tab) {
         FillGraphs(i_tab, fGraphs);
         DrawCanvas(i_tab, nullptr, fHists, fGraphs);
      }
   }else{
      FillGraphs(active_tab, fGraphs);
      DrawCanvas(active_tab, nullptr, fHists, fGraphs);
   }
}

void CERMonHists::Clear(int active_tab){
   // Clear the histograms
   if(active_tab== -1){ // Clears EVERYTHING
      if(fDebug) std::cout << "RasterHists::clear() everything \n";
      for(auto &h_t: fHists){
         auto &h = h_t.hist;
         h->Reset();
      }
      for(auto &g_t: fGraphs){
         auto &g = g_t.graph;
         g->Set(1);  // Only one point effectively empties the graph.
         for(auto &buf: fEvio->fTimeBuf) buf.clear();  // Empty the buffers too.
         for(auto &buf: fEvio->fAdcAverageBuf) buf.clear();
      }
   }else if(active_tab== -2) { // Clears HISTOGRAMS only
      if (fDebug) std::cout << "RasterHists::clear() histograms. \n";
      for (auto &h_t: fHists) {
         auto &h = h_t.hist;
         h->Reset();
      }
   }else if(active_tab>=0 && active_tab < fTabs.size() ){
      if(fDebug) std::cout << "RasterHists::clear() tab " << active_tab << "\n";
      auto tab = fTabs.at(active_tab);
      for(int i_h: tab.hists){
         fHists.at(i_h).hist->Reset();
      }
      for(int i_g: tab.graphs){
         fGraphs.at(i_g).graph->Set(1);
         fEvio->fTimeBuf.at(i_g).clear();
         fEvio->fAdcAverageBuf.at(i_g).clear();
      }
   }
   DoDraw();    // Draw so that even during a Pause the display is updated.
}

void CERMonHists::HistFillWorker(int thread_num){
   // This is the main worker component of the program. It is currently setup to run in a single thread.
   // If multiple threads are needed, more care needs to be taken to fill histograms efficiently since a simple
   // histogram fill mutex lock will not scale well.
   //
   // This code mostly directly fills histograms from the EVIO data. Some histograms have special calculations or
   // conditions as directed by Histogram_t.special_fill.
   TRandom3 rndm(thread_num);

   std::vector<double> local_data;
   local_data.reserve(fHists.size() + fGraphs.size());   // Probably more than needed, but okay.

   static double x_sum=0;   // To compute an average
   static double y_sum=0;
   static unsigned long sum_count;
   static double x_max;
   static double x_min;
   static double y_max;
   static double y_min;

   if(fDebug>0) std::cout << "RasterHists::HistFillWorker - Start thread "<< thread_num << "\n";

   while(fKeepWorking){
      if(!fPause) {
         fEvioReadLock.lock();
         fIsTryingToRead = true;

         if(fEvio->IsReadingFromEt() && !fEvio->IsETAlive()){
            // Dead ET system. Don't read, stop
            // The RasterMonGui will detect the dead ET, destroy it and try to reconnect.
            std::cout << "ET system dead?  -- will try to reconnect.\n";
            fKeepWorking = false;
            fEvioReadLock.unlock();
            break;
         }

         int stat = fEvio->Next();

         fIsTryingToRead = false;
         if(stat == EvioTool::EvioTool_Status_No_Data){
            fEvioReadLock.unlock();
            continue;
         }

         if(stat != EvioTool::EvioTool_Status_OK){
            fKeepWorking = false;
            fEvioReadLock.unlock();
            if(stat != EvioTool::EvioTool_Status_EOF) std::cout << "Evio Error -- stat = " << stat << std::endl;
            else if(fDebug) std::cout << "Evio - End of file. Stop.\n";
            break;
         }
         if(fEvio->GetEventNumber() == 0){  // Event number = 0 does not have useful data for us.
            fEvioReadLock.unlock();
            if(fDebug>2) std::cout << "Event without event number. Skip. \n";
            continue;
         }

         unsigned int trigger_bits = fEvio->GetTrigger();
         fEvio->fMostRecentEventNumber = fEvio->GetEventNumber(); // For GUI to always show a useful number.

         // *Copy* the data. Then is is safe to release the lock.
         // If we don't copy, then the lock cannot be opened, and so there is no point in having multiple workers.
         // This copy is pretty fast, so okay even for a single thread.
         //

         local_data.clear();
         // memcpy is perhaps twice as fast ~ 1,879 ns per copy if size = 10;
         // memcpy(&local_data[0], &fEvio->fChannelAverage[0], fEvio->fChannelAverage.size() * sizeof(double));
         // Safer way to do this is assign or insert. ~ 3.7953 ns per copy is size = 10;
         local_data.assign(fEvio->fChannelAverage.begin(), fEvio->fChannelAverage.end());

         fEvioReadLock.unlock();

         ///////////////////////////////////////////////////////////////////////////////////////////////////
         // Sort and fill the histograms here...
         ///////////////////////////////////////////////////////////////////////////////////////////////////
         // The part below would benefit from multiple threads *if* the Fill() ends up being too slow.
         //
         for(auto &h: fHists) {
            if( trigger_bits && !(h.trigger_bits & trigger_bits) ) continue;    // Skip if bits do not agree with trigger bits set.
            if (h.special_fill == kHist_Special_Fill_Normal) {
               int indx = h.data_index;
               double x = fEvio->GetData(h.data_index)*h.scale_x + h.offset_x;
               if (h.Is2D()) {
                  double y = fEvio->GetData(h.data_index2)*h.scale_y + h.offset_y;
                  h.GetHist2D()->Fill(x, y);
               }
               else {
                  h.GetHist()->Fill(x);
               }
            }else if(h.special_fill == kHist_Special_Fill_Helicity){
               // The helicity histgrams are -1 or 1
               // TODO: determine hi and low level?
               int indx = h.data_index;
               if(fEvio->GetData(indx) > 1500 )
                  h.GetHist()->Fill(1);
               else
                  h.GetHist()->Fill(-1);
            }else if(h.special_fill == kHist_Special_Fill_Radius){
               // Compute the radius from x and y, relative to the pattern center.
               // Pattern center can come from <x> and <y> or from (x_max - x_min)/2 and (y_max - y_min)/2

               double x = fEvio->GetData(h.data_index )*h.scale_x + h.offset_x;
               double y = fEvio->GetData(h.data_index2)*h.scale_y + h.offset_y;
               double x_mean = 0;
               double y_mean = 0;
               if( h.x_ref_hist != nullptr ){
                  x_mean = h.x_ref_hist->GetMean();
               }
               if( h.y_ref_hist != nullptr ){
                  y_mean = h.y_ref_hist->GetMean();
               }
               // if( h.x_ref_hist != nullptr ) printf("<x> = %8.5f  <y> = %8.5f \n",x_mean, y_mean);
               double r = sqrt((x-x_mean)*(x-x_mean) + (y-y_mean)*(y-y_mean));
               h.GetHist()->Fill(r);
            }else if(h.special_fill == kHist_Special_Fill_Trigger){
               unsigned int trig_bits = fEvio->GetTrigger();
               for(int i=0; i<32; ++i){
                  if( trig_bits & (1<<i)) h.GetHist()->Fill(i);
               }
            }
         }
         // std::this_thread::sleep_for(std::chrono::milliseconds(10));
         // Nothing to do for Scope, the buffer is filled in
         // fEvio->Next(); We just need to draw it.

      }else{
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
   }
   if(fDebug>0) std::cout << "RasterHists::HistFillWorker - Exit thread "<< thread_num << "\n";
}

void CERMonHists::SavePDF(const string &file, bool overwrite){
   // Save the canvasses as PDF file
   DoDraw();  // Make sure they are all updated.

   for(int count =0; count < fTabs.size(); ++count){
      auto tab = fTabs.at(count);
      TCanvas *canv = tab.canvas->GetCanvas();
      string out;
      if(count == 0 && fTabs.size() > 1) out = file + "(";  // First one of set.
      else if(count == fTabs.size() -1) out = file + ")";   // Last one of set.
      else out = file;                                      // Middle page.
      canv->Print(out.c_str());
   }
};

void CERMonHists::SaveCanvasesToPDF(const string &filename, std::vector<TCanvas *> *canvasses) {
   // Save the canvasses to PDF file.
   for (int i = 0; i < canvasses->size(); ++i) {
      auto canv = canvasses->at(i);
      string out = filename + to_string(i) + ".pdf";
      fDrawLock.lock();
      gROOT->SetBatch(true);
      canv->Draw();
      canv->Print(out.c_str());
      gROOT->SetBatch(false);
      fDrawLock.unlock();
      canv->Destructor();
      delete canv;
   }
   canvasses->clear();
}

std::vector<std::string> CERMonHists::SaveCanvasesToImageFiles(const string &filename, const string &ending, std::vector<TCanvas *> *canvasses){
   // Save the canvasses  as set of PNG, GIF or JPG files, depending on the "ending" provided.
   // The tabs will be stored as filename_#.ending  where # is the tab number.
   // Make sure they are all updated.
   std::vector<std::string> out_filenames;
   for(int i =0; i < canvasses->size(); ++i) {
      gSystem->ProcessEvents();
      auto canv = canvasses->at(i);
      string out = filename + to_string(i) + "." + ending;
      fDrawLock.lock();
      gROOT->SetBatch(true);
      canv->Draw();
      canv->Print(out.c_str());
      gSystem->Sleep(250);
      gROOT->SetBatch(false);
      fDrawLock.unlock();
      out_filenames.push_back(out);
      canv->Close();
      canv->Destructor();
      delete canv;
   }
   canvasses->clear();

   return out_filenames;
};


void CERMonHists::SaveRoot(const string &file, bool overwrite){
   // Save the histograms to a ROOT file.
   std::unique_ptr<TFile> out_file;
   try {
      if (overwrite) {
         out_file = std::make_unique<TFile>(file.c_str(), "RECREATE");
      } else {
         out_file = std::make_unique<TFile>(file.c_str(), "UPDATE");
      }

      for(auto &hist: fHists) hist.GetTH1()->Write();
      for(auto &graph: fGraphs) graph.graph->Write();
      for(auto &tab: fTabs){
         TCanvas *canv = tab.canvas->GetCanvas();
         canv->Write();
      }
      out_file->Write();
      out_file->Close();

   } catch(exception e){
      std::cout << "ERROR Opening ROOT file to save histograms \n";
      std::cout << e.what() << std::endl;
   }

};
