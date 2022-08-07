//
// Created by Maurik Holtrop on 3/29/22.
//
#include "RasterMon.h"
#include "RasterMonGui.h"

RasterMonGui::RasterMonGui(RasterMonConfigInfo *info, RasterHists *hist, const TGWindow *p, UInt_t w, UInt_t h)
      : fWindowWidth(w), fWindowHeight(h), fInfo(info), fRHists(hist), TGMainFrame(p,w,h) {
   Init();
}

void RasterMonGui::Init(){

   setlocale(LC_NUMERIC, "");  // This helps with fancy printing numbers, setting locale => printf("%'d",i) now works.

   static const char *gfFileTypes[] = {
         "EVIO data",   "*.evio.*",
         "EVIO File",   "*.evio",
         0,               0
   };

   static const char *gfFileSaveTypes [] = {"PDF File", "*.pdf", "PNG Image set", "*.png", "ROOT File", "*.root"};

   fFileInfo.SetIniDir("/data");
   fFileInfo.fFileTypes = gfFileTypes;
   fFileInfo.SetMultipleSelection(true);
   fSaveFileInfo.SetIniDir(std::filesystem::current_path().c_str());
   fSaveFileInfo.fFileTypes = gfFileSaveTypes;
   fSaveFileInfo.SetFilename("RasterMonHists");
   fHistUpdateTimer = new TTimer(this, fUpdateRate) ;
   fEvioStatusCheckTimer = new TTimer(this, fEvioStatusCheckRate);
   fLogBook = std::make_unique<RasterLogBookEntry>(this, fRHists);
   fLogBook->Connect("CloseWindow()", "RasterMonGui", this, "DoneLogEntry()");

   fEvio = fRHists->GetEvioPtr();
}

void RasterMonGui::AddMenuBar(){
   // Add the menu bar to the main window.

   fMenuBar = new TGMenuBar(this, 10, 10, kHorizontalFrame);

   auto fMenuFile = new TGPopupMenu(gClient->GetRoot());
   fMenuFile->AddEntry(" &Open Evio file...\tCtrl+O", M_FILE_OPEN, 0,
                       gClient->GetPicture("bld_open.png"));
   fMenuFile->AddEntry(" &ET Connect...\tCtrl+E", M_ET_CONNECT);
   fMenuFile->AddEntry(" Co&nfigure\tCtrl+N", M_CONFIGURE);

   fMenuFile->AddEntry(" &Save Histogram file...\tCtrl+O", M_HISTOGRAM_SAVE, 0,
                       gClient->GetPicture("bld_open.png"));

   fMenuFile->AddSeparator();
   fMenuFile->AddEntry(" E&xit\tCtrl+Q", M_FILE_EXIT, 0,
                       gClient->GetPicture("bld_exit.png"));
   fMenuFile->Connect("Activated(Int_t)", "RasterMonGui", this,
                      "HandleMenu(Int_t)");

   fMenuBar->AddPopup("&File", fMenuFile, new TGLayoutHints(kLHintsTop|kLHintsLeft,
                                                            0, 4, 0, 0));
   auto fMenuHelp = new TGPopupMenu(gClient->GetRoot());
   fMenuHelp->AddEntry(" &About...", M_HELP_ABOUT, 0,
                       gClient->GetPicture("about.xpm"));
   fMenuHelp->Connect("Activated(Int_t)", "RasterMonGui", this,
                      "HandleMenu(Int_t)");

   fMenuBar->AddPopup("&Help", fMenuHelp, new TGLayoutHints(kLHintsTop|kLHintsRight));

   AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 5));

}

void RasterMonGui::AddControlBar(){
   // Bottom bar with control buttons.

   auto *hframe=new TGHorizontalFrame(this, fWindowWidth , 30, kFixedHeight);
   // Add the button frame to the window.
   AddFrame(hframe,new TGLayoutHints(kLHintsExpandX,2,4,2,2));

   fClearProgress = new TGHProgressBar(hframe,TGProgressBar::kFancy,300);
   fClearProgress->SetBarColor("lightblue");
   fClearProgress->ShowPosition(kTRUE,kFALSE,"%.1f s to clear");
   fClearProgress->SetMin(0);
   fClearProgress->SetMax(fRHists->fHistClearTimerRate);
   fClearProgress->Resize(300,10);
   hframe->AddFrame(fClearProgress, new TGLayoutHints(kLHintsLeft | kLHintsBottom, 0,0,0,0));

   auto hframe_sub = new TGHorizontalFrame(hframe, 120, 10 );
   hframe->AddFrame(hframe_sub,new TGLayoutHints(kLHintsLeft ,150,4,2,2) );
   auto *go = new TGTextButton(hframe_sub,"&Go");
   go->SetToolTipText("Start processing data, and start updating screen.");
   go->Connect("Clicked()","RasterMonGui",this,"Go()");
   hframe_sub->AddFrame(go, new TGLayoutHints(kLHintsCenterX,
                                              5,5,3,4));

   fPauseButton = new TGTextButton(hframe_sub," &Pause ");
   fPauseButton->SetToolTipText("Pause the screen updates, but keep processing incoming events.");
   fPauseButton->Connect("Clicked()","RasterMonGui",this,"Pause()");
   hframe_sub->AddFrame(fPauseButton, new TGLayoutHints(kLHintsCenterX,
                                                        5,5,3,4));

   TGButton *stop;
   auto stop_pic =  gClient->GetPicture("ed_interrupt.png");
   if(stop_pic){
      stop = new TGPictureButton(hframe_sub, stop_pic);
   }else{
      stop = new TGTextButton(hframe_sub, "&Stop");
   }
   stop->SetToolTipText("Stop processing data and stop updating screen.");
   stop->Connect("Clicked()","RasterMonGui",this,"Stop()");
   hframe_sub->AddFrame(stop, new TGLayoutHints(kLHintsCenterX,
                                                5,5,3,4));

   auto *cleartab = new TGTextButton(hframe_sub,"&Clear");
   cleartab->SetToolTipText("Clear only the histograms in this tab.");
   cleartab->Connect("Clicked()","RasterMonGui",this,"ClearTab()");
   hframe_sub->AddFrame(cleartab, new TGLayoutHints(kLHintsCenterX,
                                                    5,5,3,4));

   auto *clear = new TGTextButton(hframe_sub,"&Clear All");
   clear->SetToolTipText("Clear the histograms in all the tabls.");
   clear->Connect("Clicked()","RasterMonGui",this,"ClearAll()");
   hframe_sub->AddFrame(clear, new TGLayoutHints(kLHintsCenterX,
                                                 5,5,3,4));

   auto *config = new TGTextButton( hframe, "Config");
   config->SetToolTipText("Pop up a config window. The gui remains fully active, and config changes will be instant.");
   config->Connect("Clicked()","RasterMonGui", this, "DoConfigure()");
   hframe->AddFrame(config, new TGLayoutHints(kLHintsCenterX, 50,5,3,4));

   fLogentry = new TGTextButton( hframe, "Log Entry");
   fLogentry->SetToolTipText("Create a PNG file for each tab and pop up a log entry window to make a log entry.");
   fLogentry->Connect("Clicked()","RasterMonGui", this, "MakeLogEntry()");
   hframe->AddFrame(fLogentry, new TGLayoutHints(kLHintsCenterX, 50,5,3,4));

   auto exit_pic =  gClient->GetPicture("ed_quit.png");
   auto *exit = new TGPictureButton(hframe, exit_pic);
   exit->SetToolTipText("Exit this program.");
//   TGTextButton *exit = new TGTextButton(hframe,"&Exit");
   exit->Connect("Clicked()","RasterMonGui",this,"Exit()");
   hframe->AddFrame(exit, new TGLayoutHints(kLHintsRight,
                                            100,5,3,10));
}

void RasterMonGui::AddStatusBar() {
   // status bar

   auto hframe = new TGHorizontalFrame(this, fWindowWidth, 24, kFixedHeight);
   AddFrame(hframe, new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 5));

   Int_t parts[] = {10, 25, 25, 40};
   fStatusBar = new TGStatusBar(hframe, fWindowWidth, 20, kHorizontalFrame);
   fStatusBar->SetParts(parts, 4);
   fStatusBar->Draw3DCorner(kFALSE);
   hframe->AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 0, 0, 0, 0));
}

void RasterMonGui::StatusBarUpdate(){
   // Update the information in the status bar.
   static int n_updates = 0;
   static unsigned long last_event_count=0;
   static auto time0 = std::chrono::system_clock::now();
   static auto time1 = std::chrono::system_clock::now();

   n_updates++;
   char text0[10];
   sprintf(text0, "%'8d", n_updates);
   fStatusBar->SetText( text0, 0);
   char text1[100];
   sprintf(text1,"Current event: %'10ld",fEvio->fMostRecentEventNumber);
   fStatusBar->SetText( text1, 1);
   sprintf(text1,"Nevt: %'12ld",fEvio->fNEventsProcessed);
   fStatusBar->SetText( text1, 2);

   auto delta_evt = fEvio->fNEventsProcessed - last_event_count;
   last_event_count = fEvio->fNEventsProcessed;
   auto time2 = std::chrono::system_clock::now();
   auto delta_t = std::chrono::duration_cast<std::chrono::microseconds>(time2-time1);
   auto total_t = std::chrono::duration_cast<std::chrono::microseconds>(time2-time0);
   sprintf(text1,"<r> = %8.3f kHz r: %8.3f kHz",
           1000.*fEvio->fNEventsProcessed/total_t.count() ,1000.*delta_evt/delta_t.count());
   fStatusBar->SetText( text1, 3);

   time1 = time2;
   if(fDebug==1){
      printf("Events processed:   %'10ld    delta: %5ld     Update #%5d\n", fEvio->fNEventsProcessed, delta_evt, n_updates);
      printf("Time elapsed total: %'10ld   delta t: %5ld\n", (long)total_t.count(), (long)delta_t.count());
      printf("Average rate:       %8.3f kHz  Current rate: %8.3f kHz\n",
             1000.*fEvio->fNEventsProcessed/total_t.count() ,1000.*delta_evt/delta_t.count());
   }

   double pos = fRHists->fHistClearTimerRate;
   fClearProgress->SetMax(fRHists->fHistClearTimerRate);
   if(fRHists->fHistClearTimerIsOn) pos = double(long(fRHists->fHistClearTimer->GetAbsTime() - gSystem->Now())) / 1000.;
   fClearProgress->SetPosition(pos);
}

void RasterMonGui::AddTabArea() {
   //--------- create the Tab widget
   fTabAreaTabs = fRHists->AddTabArea(this, fWindowWidth, fWindowHeight);
   AddFrame(fTabAreaTabs, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                                            2, 2, 5, 1));
}

void RasterMonGui::SetupGUI(){
   // Menu bar

   AddMenuBar();
   //
   AddTabArea();
   //
   AddControlBar();
   //
   AddStatusBar();
   // Sets window name and shows the main frame
   SetWindowName("RasterMon");
   MapSubwindows();
   Resize(GetDefaultSize());
   MapWindow();
}

void RasterMonGui::HandleMenu(int choice) {
   // Handle the menu choices.
   TRootHelpDialog *hd;
   ETConnectionConfig *et_dialog;

   switch (choice) {
      case M_FILE_OPEN:
         // Set the file, or file list, to open by EvioTool.
         new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fFileInfo);
         if(fFileInfo.fFilename){
            if(fDebug) std::cout << "Adding file " << fFileInfo.fFilename << "\n";
            if(fEvio->fiInputFile >= fEvio->fInputFiles.size()) fEvio->fiInputFile--; // The Next() in fEvio will increase this again before opening next file!
            fEvio->AddFile(fFileInfo.fFilename);
         }
         if(fFileInfo.fFileNamesList){
            if(fEvio->fiInputFile >= fEvio->fInputFiles.size()) fEvio->fiInputFile--;
            for(const auto &&ff: *fFileInfo.fFileNamesList){
               if(fDebug) std::cout << "Adding file " << ff->GetName() << "\n";
               fEvio->AddFile(ff->GetName());
            }

         }
         if(fEvio->IsReadingFromEt()){
            StartEvioStatusCheckTimer();
         }else{
            StopEvioStatusCheckTimer();
         }
         break;

      case M_HISTOGRAM_SAVE:
         new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fSaveFileInfo);
         if(fSaveFileInfo.fFilename){
            if(fDebug) std::cout << "Saving histograms to file: " << fSaveFileInfo.fFilename << "\n";
            string save_file(fSaveFileInfo.fFilename);
            std::string::size_type const p(save_file.find_last_of('.')); // Find extension.
            std::string file_ending;
            if( p == save_file.npos ){
               std::cout << "No extension give. Will use .pdf \n";
               file_ending=".pdf";
            }else {
               file_ending = save_file.substr(p + 1);
            }
            string file_stub = save_file.substr(0, p);

            if( file_ending == "" ) {   // no extension, save as pdf
               file_ending = "pdf";
            }

            if( file_ending == "pdf"){
               fRHists->SavePDF(save_file);
            }else if ( file_ending == "root"){
               fRHists->SaveRoot(save_file, fSaveFileInfo.fOverwrite);
            }else {  // Try the ROOT "SaveAs" for each canvas.
               std::vector<TCanvas *> canvs;
               for(int count =0; count < fRHists->fTabs.size(); ++count) {
                  auto tab = fRHists->fTabs.at(count);
                  TCanvas *canv = (TCanvas *) tab.canvas->GetCanvas()->DrawClone();
                  canvs.push_back(canv);
               }
               fRHists->SaveCanvasesToImageFiles(file_stub, file_ending, &canvs);
               canvs.clear();
            }
         }
         break;

      case M_HELP_ABOUT:
         hd = new TRootHelpDialog(this, "About RasterMon ...", 550, 250);
         hd->SetText(""
                     "===============  RasterMonGui, Version: " RASTERMON_VERSION " ===============\n"
                     "This is a simple GUI program to monitor the Raster in the RGC run.\n"
                     "It can open an EVIO file or attach to the CLAS12 ET ring to process\n"
                     "the data coming from the experiment.\n"
                     "You control the app with the buttons at the bottom: go, pause, stop\n"
                     "clear, clearall, config and exit. (stop and exit are symbols.)\n"
                     "There is an auto-clear timer, the status of which you see on the \n"
                     "left of the buttons. The timer is controlled from the config dialog.\n"
                     "For more detailed help go to: \n"
                     "https://github.com/JeffersonLab/RasterMon/wiki"
                     );
         hd->Popup();
         break;
      case M_ET_CONNECT:
         if(fRHists->IsWorking()){
            std::cout << "Don't mess with the ET while we are processing data! Called Stop. \n";
            fRHists->Stop();
         }
         et_dialog = new ETConnectionConfig(this, fEvio);
         if(fEvio->IsReadingFromEt()){
            StartEvioStatusCheckTimer();
         }else{
            StopEvioStatusCheckTimer();
         }
         break;

      case M_CONFIGURE:
         // Stop();
         DoConfigure();
         break;

      case M_FILE_EXIT:
         // close the window and quit application
         Exit();
         break;

   }
}

void RasterMonGui::DoConfigure(){
   if(fConfig == nullptr){
      if(fDebug>1) std::cout << "Start Configure new configure dialog.\n";
      fConfig = new RasterMonConfigPanel(this, fInfo);
   }else{
      auto x = this->GetX();
      auto y = this->GetY();
      int x2, y2;
      this->GetWMPosition(x2, y2);
      if(fDebug>1){
         std::cout << "Configure already open. Raise and recenter. \n";
         printf("POS: (%d, %d) WM (%d, %d) \n",x,y,x2,y2);
      }
      fConfig->RaiseWindow();
   }
}

void RasterMonGui::DoDraw() {
   // This is a Draw for the tab selected canvas.
   if(fUpdateSelectedTabOnly){
      int current_tab = fTabAreaTabs->GetCurrent();
      fRHists->DoDraw(current_tab);
   }else {
      fRHists->DoDraw(-1);
   }
   if (!fRHists->IsWorking() && !fPause) Stop();  // Stop if you detect the worker threads ended.
   // But not if Paused, because Pause calls DoDraw to make sure canvasses are updated, and we don't want to loop.
}

void RasterMonGui::Pause(int set_state){
   // Pause or UnPause, there is no try.
   // If set_state == 1  then go to Pause state -- for stop()
   // If set_state == -1 then go to UnPause state -- for go()
   if(set_state == 0){
      if(fRHists->IsWorking()) {
         if (fPause) set_state = -1;
      }
      else{
         if(fDebug) std::cout << "Cannot Pause when I am not working. \n";
         return;
      }
   }

   if(set_state == -1){
      // Unpause
      fPauseButton->SetText("&Pause");
      if(fDebug>1) std::cout << "Un-Pause \n";
      fHistUpdateTimer->TurnOn();
      if(fRHists->fHistClearTimerIsOn ){
         fRHists->fHistClearTimer->Reset();
         fRHists->fHistClearTimer->TurnOn();
      }
      fPause = false;
      fRHists->UnPause();
   }else{
      // Pause
      if(set_state == 0) fPauseButton->SetText("&UnPause");
      if(fDebug>1) std::cout << "Pause \n";
      fHistUpdateTimer->TurnOff();
      fRHists->fHistClearTimer->TurnOff();
      // fRHists->Pause();
      fPause = true;
      fUpdateSelectedTabOnly = false;  // Update all the canvases, so they show something.
      DoDraw();
      fUpdateSelectedTabOnly = true;
   }
}
