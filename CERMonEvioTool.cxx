//
// Created by Maurik Holtrop on 3/28/22.
//

#include "RasterEvioTool.h"
#include <thread>

RasterEvioTool::RasterEvioTool(string infile) : EvioTool(infile){
   // Basic setup of the EvioTool Class

   fAutoAdd = false;  // Do not add every bank in the file automatically.
   fChop_level = 1;   // Top level bank is chopped.
   tags = {0x80};      // Only get the physics event banks.  The 31 tag is EPICS.
   tag_masks = {0x80}; // Any event with bit 6 set  2<<6
   // EVIO Data structure unpack:
   fEvioHead = AddLeaf<unsigned int>("fEvioHead", 49152, 0, "Evio Event Header Info");
   fRasterHead =  new RasterMonEventInfo(this);  // Calls AddBank internally.
   fETWaitMode = ET_SLEEP;
   fEtReadChunkSize = 10;
}

int RasterEvioTool::AddChannel(unsigned short bank_tag, unsigned short slot, unsigned short channel){
   // Add one data item to the structure for parsing EVIO files.
   // Return the index to the fChannelAverage and fTimeBuf and fAdcAverageBuf data stores.
   //
   EvioBank_t *lbank = nullptr;
   // See if the bank is already in the parse tree.
   // TODO: Can speed this up with a proper search if the tree gets large?)
   for(auto &b: fEvioBanks){
      if( b.bank->CheckTag(bank_tag)){
         lbank = &b;
         break;
      }
   }
   bool stat = true;
   if(lbank == nullptr){ // Not found, so create it.
      fEvioBanks.emplace_back(this, bank_tag, slot, channel, fChannelAverage.size());
   }else{
      stat = lbank->AddSlot(slot, channel, fChannelAverage.size());
   }
   if(stat) {  // Data was actually added, so reserve space for it.
      fChannelAverage.push_back(0.);
      fTimeBuf.emplace_back(fAdcBufferSize);
      fAdcAverageBuf.emplace_back(fAdcBufferSize);
      return(fChannelAverage.size()-1);
   }else{  // Data was not added, so search for it in the tree.
      for(int i_t=0; i_t < fEvioBanks.size(); ++i_t) {
         if (fEvioBanks[i_t].bank->CheckTag(bank_tag)) {  // Bank tag maches.
            for (int i_s = 0; i_s < fEvioBanks[i_t].slots.size(); ++i_s) {
               if( fEvioBanks[i_t].slots[i_s].slot == slot) { // Slot matches
                  for (int i_c = 0; i_c < fEvioBanks[i_t].slots[i_s].channels.size(); ++i_c) {
                     if(fEvioBanks[i_t].slots[i_s].channels[i_c] == channel ){ // Channel matches, we have a match.
                        return fEvioBanks[i_t].slots[i_s].data_index[i_c];  // return the index.
                     }
                  }
               }
            }
         }
      }
      cout << "RasterEvioTool::AddChannel - Uh oh, I did not add the channel, but I could not find it either.\n";
      return -1;  //
   }

}

void RasterEvioTool::Clear(Option_t *opt) {
   // Clears the entire data structures.
   // Here we clear the *local* data copy, then call the parent Clear to clear the rest.
   fChannelAverage.assign(fChannelAverage.size(), 0);
   EvioTool::Clear(opt);
}

int RasterEvioTool::Next() {
   // Get the next event from EVIO and process it.
   // If the file is not opened, or the read causes an EOF, open the next file.
   Clear("");

   fFileLock.lock();
   if( !IsReadingFromEt() && !IsOpen()){
      // Open the next file from this file list.
      fiInputFile++;
      if(fiInputFile >= fInputFiles.size()){
         if( fDebug & EvioTool_Debug_Info ) std::cout << "End of EVIO file data and ET not connected. \n";
         fFileLock.unlock();
         return(EVIO_END_OF_DATA);
      }
      if( fDebug & EvioTool_Debug_Info ) std::cout << "Open EVIO file " << fInputFiles[fiInputFile] << "\n";
      Open(fInputFiles[fiInputFile].c_str());
   }
   fFileLock.unlock();

   int stat = EvioTool::Next();
   if( stat == EvioTool_Status_EOF && !IsReadingFromEt()){
      if( IsOpen() ) Close();
      return(Next());  // Recursively call next, which wil open the next file and read the next event.
   }
   if(stat == ET_ERROR_EMPTY || stat == ET_ERROR_BUSY ){
      //if(fDebug & EvioTool_Debug_L1) std::cout << "ET Error Empty or busy stat = " << stat << endl;
      return(EvioTool_Status_No_Data);
   }
   if(stat == ET_ERROR_READ ){
      std::cout << "ET Error READ. \n";
      return(EvioTool_Status_No_Data);
   }
   if(stat != ET_OK) return(stat);

   if(fDebug & EvioTool_Debug_L1) {
      if( (long(GetEventNumber()) - long(fLastEventNumber)) > 1) {
         std::cout << "Event skip: Run number: " << GetRunNumber() << " last: " << fLastEventNumber << "  this:" <<
                   GetEventNumber() << " skip: " << (long(GetEventNumber()) - long(fLastEventNumber)) << std::endl;
      }
      if( (long(GetEventNumber()) - long(fLastEventNumber)) <= 0) {
         std::cout << "OOSequence: Run number: " << GetRunNumber() << " last: " << fLastEventNumber << "  this:" <<
                   GetEventNumber() << " skip: " << (long(GetEventNumber()) - long(fLastEventNumber)) << std::endl;
      }
   }

   fLastRunNumber = GetRunNumber();      // These are only updated on actual good data.
   fLastEventNumber = GetEventNumber();

   fNEventsProcessed++;

   // We loop through the EVIO Banks in fEvioBank. If they were in the data and had an FADC bank, then we will
   // find this in b.FADC.
   // We compare the b.FADC against the slot, channel information from our fEvioBanks tree, and when matchted,
   // store the *average* FADC in the appropriate fChannelAverage slot.
   //
   // The following multi loop sort is fairly fast because the loops are small.
   // TODO: Investigate if under small loop conditions, an std::map lookup is (still) faster than this algorith.
   for(auto &b: fEvioBanks){
      for(int i_fadc=0; i_fadc< b.FADC->size(); ++i_fadc){
         for(auto &slot: b.slots){
            if( b.FADC->GetData(i_fadc).GetSlot() == slot.slot ) {
               for (int j = 0; j < slot.channels.size(); ++j) {
                  if (b.FADC->GetData(i_fadc).GetChan() == slot.channels[j]) {
                     double sum = 0.;
                     int n_samples = b.FADC->GetData(i_fadc).GetSampleSize();
                     for (int k = 0; k < n_samples; ++k) {
                        sum += (double) b.FADC->GetData(i_fadc).GetSample(k);
                     }
                     sum = sum / (double) n_samples;
                     int buf_index = slot.data_index[j];
                     fChannelAverage[buf_index] = sum;
                     double time = FADC_TIME_CONVERSION*(double)b.FADC->GetData(i_fadc).GetRefTime(); // Convert to second
                     if(time>0) { // If time == 0, then this is not data so skip it.
                        std::lock_guard<std::mutex> _lck(fBufferLock);
                        fTimeBuf[buf_index].push_back(time);
                        fAdcAverageBuf[buf_index].push_back(sum);
                     }
                  }
               }
            }
         }
      }
   }

   return(EvioTool_Status_OK);
}

ClassImp(RasterEvioTool);