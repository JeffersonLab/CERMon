//
//  RasterMonEventInfo.h
//  HPSEvioReader
//
//  Created by Maurik Holtrop on 4/11/22.
//  Copyright © 2022 UNH. All rights reserved.
//
// For HPS data the RasterMonEventInfo is a Leaf from the tag=46 bank.
//
// RasterMonEventInfo   tag = 0xe10f (57615)   unsigned int
//  0(31:0)  "version number"
//  1(31:0)  "run number"
//  2(31:0)  "event number"
//  3(31:0)  0
//  4(31:0)  0
//  5(31:0)  0
//
//
#ifndef Headbank_h
#define Headbank_h

#include "EvioTool.h"
#include "Leaf.h"

class CERMonEventInfo: public Bank{

public:
    Leaf<uint32_t> *RHead1 = nullptr;
    Leaf<uint32_t> *RHead2 = nullptr;

public:
  CERMonEventInfo(){};
  CERMonEventInfo(Bank *b, unsigned short itag=37, unsigned short inum=0): Bank("RasterMonEventInfo", itag, inum, "RasterMonEventInfo data"){
      b->AddBank((Bank *)this);
      // Bank 57610, 7 uint32, ??, event number, timestamp, ??, ??, ??, ??
      RHead1 = this->AddLeaf<uint32_t>("EventInfo", 57610, 0, "Event Info");
      // Bank 57615, 5 uint32, 0, run number, event number, 0, 0, 0
      RHead2 = this->AddLeaf<uint32_t>("EventInfo2", 57615, 0, "Event RHead2");
      vector<unsigned short> tags = this->GetTags();
  };

  Leaf<uint32_t> *GetRHead1(){ return(RHead1); }
  Leaf<uint32_t> *GetRHead2(){ return(RHead2); }

  unsigned int GetVersionNumber() const {
      if(RHead1 && RHead1->size() > 1) return(RHead1->GetData(0));
      else return 0;
  };

  unsigned long GetTimeStamp() const {
     if(RHead1 && RHead1->size() > 5) {
        unsigned long time = RHead1->GetData(2) + ((unsigned long) (RHead1->GetData(3) & 0x0000ffff) << 32);
        return time;
     } else return 0;
  };

  unsigned int GetTrigger1() const {
     // Return the main 32 bits of the trigger.
     if(RHead1 && RHead1->size() >= 7) {
        return RHead1->GetData(4);
     }else return 0;
  }

   unsigned int GetTrigger2() const {
      // Return the upper 32 bits of the trigger.
      if(RHead1 && RHead1->size() >= 7) {
         return RHead1->GetData(5);
      }else return 0;
   }



   unsigned int GetRunNumber() const {
     // Run number. This is only available in the 57615 bank.
      if(RHead2 && RHead2->size() > 4) return RHead2->GetData(1);
      else return 0;
  };
   unsigned int GetEventNumber() const {
      // Event number from the 57610 bank.
      if(RHead1 && RHead1->size() > 4) return(RHead1->GetData(1));
      else return 0;
   };

   unsigned int GetEventNumber2() const {
      // Event number from the 57615 bank.
      // This, it seems, is just a copy of the information in the 57610 bank.
      if(RHead2 && RHead2->size() > 4) return RHead2->GetData(2);
      else return 0;
  };

  ClassDef(CERMonEventInfo, 1);
};

#endif /* Headbank_h */
