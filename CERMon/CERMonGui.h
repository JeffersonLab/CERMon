//
// Created by Maurik Holtrop on 3/29/22.
//

#ifndef RASTERMON_RASTERMONGUI_H
#define RASTERMON_RASTERMONGUI_H

#include <TROOT.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TTimer.h>
#include <TObjString.h>
#include <TGClient.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGMenu.h>
#include <TGTab.h>
#include <TGFileDialog.h>
#include <TGStatusBar.h>
#include <TGProgressBar.h>
#include <TRootHelpDialog.h>

#include <locale>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem>

// #include "CERMon.h"
#include "CERMonHists.h"
#include "CERMonEvioTool.h"
#include "ETConnectionConfig.h"
#include "ConfigPanel.h"
#include "LogbookEntry.h"

class CERMonGui : public TGMainFrame {

public:

   enum EMyMessageTypes {
      M_FILE_OPEN,
      M_HISTOGRAM_SAVE,
      M_ET_CONNECT,
      M_CONFIGURE,
      M_FILE_EXIT,
      M_HELP_ABOUT
   };

   unsigned int fWindowWidth;
   unsigned int fWindowHeight;

   TGTab *fTabAreaTabs = nullptr;
   TGMenuBar *fMenuBar = nullptr;
   TGStatusBar *fStatusBar = nullptr;
   TGTextButton *fPauseButton = nullptr;
   bool fPause = false;
   TGTextButton *fLogentry;
   TTimer  *fHistUpdateTimer = nullptr;
   TTimer  *fEvioStatusCheckTimer = nullptr;
   unsigned int fEvioStatusCheckRate=1000;  // How often to check if ET is okay.
   TGHProgressBar *fClearProgress;

   CERMonHists* fRHists = nullptr;
   CERMonEvioTool *fEvio = nullptr;  // Not an object we own, just a handy pointer.
   TGFileInfo fFileInfo;             // Contains file(s) chosen by Open dialog or populated from command line.
   TGFileInfo fSaveFileInfo;         // Contains info for Histogram Save dialog.
   unsigned int fUpdateRate=1000;     // Update rate in ms.

   ConfigPanel *fConfig = nullptr;
   ConfigInfo *fInfo = nullptr;
   bool fUpdateSelectedTabOnly = true;
   std::unique_ptr<LogBookEntry> fLogBook = nullptr;

public:
   int fDebug = 0;

public:
   CERMonGui(ConfigInfo *info, CERMonHists *hist, const TGWindow *p, UInt_t w, UInt_t h);

   virtual ~CERMonGui() {
      // Clean up used widgets: frames, buttons, layout hints
      Stop();
      // Cleanup specifically allocated memory.
   }

   void Init();
   void DoDraw();
   void SetupGUI();
   void AddMenuBar();
   void AddTabArea();
   void AddControlBar();
   void AddStatusBar();
   void StatusBarUpdate();
   void HandleMenu(int choice);
   void DoConfigure();

   void CloseConfigure(){
      fConfig->OK();
      fConfig = nullptr;
   }

   void CancelConfigure(){
      fConfig->OK();
      fConfig = nullptr;
   }

   void CloseETConfigure(){

   }

   void CancelETConfigure(){
      StopEvioStatusCheckTimer();
      fEvio->Close();
   }

   void SetUpdateRate(unsigned long rate=0){
      if(fConfig){
         rate = fConfig->fEnterUpdateRate->GetIntNumber();
         if(fDebug) std::cout << "Set update rate to: " << rate << std::endl;
         fUpdateRate = rate;
         fHistUpdateTimer->SetTime(rate);
      }else{
         if(fDebug) std::cout << "Rate not updates.\n";
      }
   }

   unsigned int GetUpdateRate() const {return fUpdateRate;}

   void SetDebug(int level){
      // Set the debug level for RasterMonGui and RasterHists
      cout << "CERMonGui::SetDebug to level " << level << endl;
      fDebug = level;
      fRHists->SetDebug(level);
   }
   void Go(){
      if(fDebug>1) std::cout << "Go \n";
      Pause(-1);
      fRHists->Go();
   };

   void Pause(int set_state=0);  // set_state: -1 = go, 0=toggle, 1 = Pause;

   void Stop(){
      if(fDebug>1) std::cout << "Stop \n";
      Pause(1);
      fRHists->Stop();
   };

   void ClearAll(){
      fRHists->Clear(-1);
      fRHists->fHistClearTimer->Reset();
   }

   void ClearTab(){
      fRHists->Clear(fTabAreaTabs->GetCurrent());
   }

   void Clear(int what= -1){
      fRHists->Clear(what);
   }

   void Exit(){
      Stop();
      cout << "Exiting CERMon. Bye now. \n";
      CloseWindow();
      gApplication->Terminate();
   }

   void MakeLogEntry(){
      // Receives the signal from the Log Book button.
      fLogentry->SetEnabled(false);
      fLogBook->MakeEntry();
   }

   void DoneLogEntry(){
      fLogentry->SetEnabled(true);
   }

   void StartEvioStatusCheckTimer(){
      fEvioStatusCheckTimer->TurnOn();
   }

   void StopEvioStatusCheckTimer(){
      fEvioStatusCheckTimer->TurnOff();
   }


   Bool_t HandleTimer(TTimer *timer) override{
      if(timer == fHistUpdateTimer){
         StatusBarUpdate();
         DoDraw();
      }else if(timer == fEvioStatusCheckTimer){
         if(fDebug>2) std::cout << "CERMon: ET status check: Is alive: " << fEvio->IsETAlive() << "\n";
         if(!fEvio->IsETAlive()){
            // Reset the ET system
            std::cout << "CERMon: ET system dead. Trying to reconnect.\n";
            fEvio->Close();
            fEvio->ReOpenEt();
         }
      }
      return kTRUE;
   }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
ClassDef(CERMonGui, 0)
#pragma clang diagnostic pop

};


#endif
