//
// Created by Maurik Holtrop on 6/14/22.
//

#include "ConfigInfo.h"


ClassImp(ConfigInfo);

void ConfigInfo::SaveToJSON(){
   // Open the fJSONFile and stream the config class to the file.
   TString json = TBufferJSON::ToJSON(this);
   std::ofstream out(fJSONFile);
   out << json;
   out.close();
   std::cout << "\033[94m RasterMon -- Config panel information saved to: " << fJSONFile << "\033[0m \n";
};

void ConfigInfo::LoadFromJSON(){
   // Open the fJSONFile and stream the config class from the file.
   try {
      std::ifstream in(fJSONFile);
      if(!in.is_open()){
         std::cout << "\033[93m RasterMonConfigInfo - Could not open the configuration file:" << fJSONFile << "\033[0m \n";
         return;
      }
      std::stringstream in_json;
      in_json << in.rdbuf();
      std::string json{in_json.str()};
      ConfigInfo *new_info = nullptr;
      TBufferJSON::FromJSON(new_info, json.c_str());
      CopyInfo(new_info);
   } catch (const std::exception& e){
      std::cout << "\033[93m RasterMonConfigInfo - Could not read the configuration file. \n";
      std::cout << e.what() << "\033[0m \n";
   }

};

void ConfigInfo::CopyInfo(ConfigInfo *that) {
   // Coppy only the relevant setting data from one RasterMonConfigInfo to self.
   fDebugLevel = that->fDebugLevel;
   fEvioDebugLevel = that->fEvioDebugLevel;
   fUpdateRate = that->fUpdateRate;
   fAutoClearOn = that->fAutoClearOn;
   fAutoClearRate = that->fAutoClearRate;
   fScopeBufDepth = that->fScopeBufDepth;
   fScale_x = that->fScale_x;
   fOffset_x = that->fOffset_x;
   fScale_y =  that->fScale_y;
   fOffset_y = that->fOffset_y;
}

void ConfigInfo::GetValues(){
   // Load the Info structure with the values from other places in the code.
   fScopeBufDepth = fEvio->GetAdcBufferSize();
//   fUpdateRate = update_rate;
   fAutoClearRate = fHists->fHistClearTimerRate;
   fAutoClearOn = fHists->fHistClearTimerIsOn;
   fDebugLevel = fHists->fDebug;
   fEvioDebugLevel = fEvio->fDebug;
   GetScaleOffset();
}

void ConfigInfo::PutValues(){
   // Put the updated values in the correct place, which we do after a "Load".
   fEvio->fDebug = fEvioDebugLevel;
   fHists->fDebug = fDebugLevel;
   UpdateADCBufDepth();
   UpdateHistClearRate();
   SetScaleOffset();
}

void ConfigInfo::GetScaleOffset() {
   // Get the X and Y scale offsets for the raster hists.
   // Find histograms Raster_xy and get the constants from them. Raster_x, Raster_y should have same constants!
   for (auto &h_t: fHists->fHists) {
      if (strncmp(h_t.hist->GetName(), "Raster_xy", 8) == 0) {
         fScale_x = h_t.scale_x;
         fOffset_x = h_t.offset_x;
         fScale_y = h_t.scale_y;
         fOffset_y = h_t.offset_y;
      }
   }
}

void ConfigInfo::SetScaleOffset() {
   // Get the X and Y scale offsets for the raster hists.
   // Find histograms Raster_x, Raster_y, Raster_xy and get the constants on them.
   for (auto &h_t: fHists->fHists) {
      string name = h_t.hist->GetName();   // Copy name to string and make all lower case.
      std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c);});
      if (name.compare(0, 6 ,"raster") == 0 ) {         // It is a "raster" type histogram.
         if(name.compare(name.size()-2,2,"_y") == 0){   // For the _y histograms, the X axis scaling must change!
            h_t.scale_x = fScale_y;     // Looks confusing, BUT, this is for the x-axis of this histogram!
            h_t.offset_x = fOffset_y;
         } else {
            h_t.scale_x = fScale_x;
            h_t.offset_x = fOffset_x;
            h_t.scale_y = fScale_y;
            h_t.offset_y = fOffset_y;
         }
      }
   }
}

void ConfigInfo::UpdateADCBufDepth() {
   // Update the circular buffer and resize the graphs for the ADC scopes.
   auto bufsize = (unsigned long) (fScopeBufDepth);
   if(fHists->fDebug )std::cout << "UpdateADCBufDepth to " << bufsize << std::endl;
   bool prevstate = fHists->Pause();   // Pause the worker threads.
   fEvio->UpdateBufferSize(bufsize);
   fHists->ResizeScopeGraphs(bufsize);
   if (!prevstate) fHists->UnPause();
}

void ConfigInfo::UpdateHistClearRate(){
   if(fAutoClearOn) fHists->SetAutoClearRateOn();
   else fHists->SetAutoClearRateOff();
   fHists->SetAutoClearRate(fAutoClearRate);
   fAutoClearOn = fHists->fHistClearTimerIsOn;  // In case the value was too low, which turns off the auto clear.
}

void ConfigInfo::SetDebug(int i) {
   // Just set the local value. This is linked to a signal from RasterMonConfigPanel,
   // which also signals to RasterMonGui.
   fDebugLevel = i;
}