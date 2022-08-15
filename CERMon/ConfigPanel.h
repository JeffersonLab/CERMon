//
// Created by Maurik Holtrop on 5/2/22.
//

#ifndef RASTERMON_RASTERMONCONFIG_H
#define RASTERMON_RASTERMONCONFIG_H

#include "TObject.h"
#include "TGCanvas.h"
#include "TGFrame.h"
#include "TGLabel.h"
#include "TGTextEntry.h"
#include "TGNumberEntry.h"
#include "TGComboBox.h"
#include <RQ_OBJECT.h>

#include <iostream>
#include <string>
#include <map>

//#include "RasterEvioTool.h"
//#include "RasterHists.h"
#include "ConfigInfo.h"

class CERMonGui;

class ConfigPanel : public TGTransientFrame {

public:
   // const TGWindow *fParentWindow;
//   RasterEvioTool *fEvio = nullptr;
//   RasterHists *fHists = nullptr;
   const CERMonGui *fRasGui = nullptr;

   TGListBox *fDebugLevelChooser;
   TGNumberEntry *fEnterUpdateRate;
   TGNumberEntry *fNumberEntryAutoClear;
   TGNumberEntry *fNumberEntryScopeBufDepth;
   TGCheckButton *fAutoClearOnOff;
   TGCheckButton *fEvioDebugInfo1;
   TGCheckButton *fEvioDebugInfo2;
   TGCheckButton *fEvioDebug1;
   TGCheckButton *fEvioDebug2;
   TGNumberEntry *fEnterScale_x;
   TGNumberEntry *fEnterOffset_x;
   TGNumberEntry *fEnterScale_y;
   TGNumberEntry *fEnterOffset_y;

   ConfigInfo *fInfo;

public:
   ConfigPanel(const CERMonGui *parent, ConfigInfo *info);

   void UpdateDisplay();
   void OK() {
      CloseWindow();
   };

   void ValueSet();
   void HandleButtons();

};

#endif //RASTERMON_RASTERMONCONFIG_H
