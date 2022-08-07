//
// Created by Maurik Holtrop on 6/14/22.
//

#ifndef RASTERMON_RASTERMONCONFIGINFO_H
#define RASTERMON_RASTERMONCONFIGINFO_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "TObject.h"
#include "TBufferJSON.h"
#include "CERMonHists.h"
#include "CERMonEvioTool.h"

// Initial values are from the start of the run.
class ConfigInfo : public TObject {

public:
   int fDebugLevel = 0;
   int fEvioDebugLevel = 0;
   unsigned int fUpdateRate = 1000;
   bool fAutoClearOn = false;
   double fAutoClearRate = 30.;
   unsigned int fScopeBufDepth = 10000;
   double fScale_x = 0.0205;
   double fOffset_x = -43.8;
   double fScale_y =  -0.0369;
   double fOffset_y = 79.6;

   std::string fJSONFile{"CERMonConfig.json"};  //! Default name for the config file. Not stored.
   CERMonHists *fHists=nullptr;                            //! Pointer to CERMonHists. Not stored.
   CERMonEvioTool *fEvio=nullptr;                          //! Pointer to CERMonEvioTool. Not stored.

public:
   ConfigInfo(){};
   ConfigInfo(CERMonHists *hists, CERMonEvioTool *evio): fHists(hists), fEvio(evio){};
   void SaveToJSON();
   void LoadFromJSON();
   void CopyInfo(ConfigInfo *that);

   void GetValues();
   void PutValues();
   void GetScaleOffset();
   void SetScaleOffset();
   void UpdateADCBufDepth();
   void UpdateHistClearRate();
   void SetDebug(int i);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
   ClassDef(ConfigInfo, 1)
#pragma clang diagnostic pop
};


#endif //RASTERMON_RASTERMONCONFIGINFO_H
