//
// Created by Maurik Holtrop on 5/12/22.
//
#include <iostream>
#include <sstream>
#include <thread>
#include <ctime>
#include <cstdlib>
#include "LogbookEntry.h"

#ifdef HAS_LOGBOOK
#include "ELogBookShim.h"
#endif

LogBookEntry::LogBookEntry(const TGWindow *parent_window, CERMonHists *rhists): fRHists(rhists) {
   fParentWindow = parent_window;

   // Check if the logentry program can be found on disk.
   // Not that exists throws an exception if (part of) the *path* exists but cannot be read, so catch that.
   try {
      if (std::filesystem::exists(std::filesystem::path(CLI_LOGENTRY_PROGRAM))) {
         fLogEntryOK = true;
      }else {
         fLogEntryOK = false;
      }
   }catch(std::filesystem::filesystem_error const& ex) {
      std::cout << "\033[0mError - Exception looking for " CLI_LOGENTRY_PROGRAM ":\n"
            << "what():  " << ex.what() << '\n'
            << "path1(): " << ex.path1() << '\n'
            << "path2(): " << ex.path2() << '\n'
            << "code().value():    " << ex.code().value() << '\n'
            << "code().message():  " << ex.code().message() << '\n'
            << "code().category(): " << ex.code().category().name() << "\033[0m\n";
      
      fLogEntryOK = false;
   }

   // Check if the logentry program is actually functioning.
   try {
      std::string cmd = CLI_LOGENTRY_PROGRAM  " --help > /dev/null";
      auto sys_stat = std::system(cmd.c_str());
      if(sys_stat != 0) {
         std::cout << RED "The logentry command does not function properly. Logentry will not be automatic.\n" ENDC;
         fLogEntryOK = false;
      }
   }catch(exception const& ex) {
      std::cout << RED "Exception: " << ex.what() << "\n";
      std::cout << "The logentry command does not function properly and throws an error. Logentry will not be automatic.\n" ENDC;
      fLogEntryOK = false;
   }

   // Check the directory for storing the image files.
   bool image_path_ok = false;
   try {
      if (std::filesystem::exists(fHistogramPath)) {
         image_path_ok = true;
      }
   }catch(std::filesystem::filesystem_error const& ex) {
      std::cout << RED "Error - Exception looking for " << fHistogramPath << ":\n"
                << "what():  " << ex.what() << '\n'
                << "path1(): " << ex.path1() << '\n'
                << "path2(): " << ex.path2() << '\n'
                << "code().value():    " << ex.code().value() << '\n'
                << "code().message():  " << ex.code().message() << '\n'
                << "code().category(): " << ex.code().category().name() << '\n';
      std::cout << "The error prevents histogram images from being created. \n" ENDC;
   }

   if(!image_path_ok) {
      try {
         // The path for storing histograms does not exist.
         string home_dir{std::getenv("HOME")};
         auto home_path = std::filesystem::path(home_dir);
         fHistogramPath = home_path / "rastermon";
         if (!std::filesystem::exists(fHistogramPath)) {
            filesystem::create_directory(fHistogramPath);
            std::cout << GREEN "Created a directory: " << fHistogramPath
                      << " for image output with Log Entry button.\n" ENDC;
         }
      }catch(std::filesystem::filesystem_error const& ex) {
         std::cout << RED "Error - Exception constructing image path:\n"
                   << "what():  " << ex.what() << '\n'
                   << "path1(): " << ex.path1() << '\n'
                   << "path2(): " << ex.path2() << '\n'
                   << "code().value():    " << ex.code().value() << '\n'
                   << "code().message():  " << ex.code().message() << '\n'
                   << "code().category(): " << ex.code().category().name() << '\n';
         std::cout << "The error prevents histogram images from being created. \n" ENDC;
      }
   }

   fMain = new TGTransientFrame(gClient->GetRoot(), fParentWindow, 400, 400);
   fMain->SetWindowName("Logbook Entry Dialog");
   fMain->DontCallClose(); // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);
   // We spin off a thread to save the canvasses to file, otherwise the entire app blocks while these are being
   // written, and writing them can be slow. We use a lock mechanism (see fRHists->fDrawLock) to allow the thread to
   // write the files in batch mode.
   // Next we open the dialog window for the user to interact with.
   fMain->Connect("CloseWindow()", "RasterLogBookEntry", this, "CloseWindow()");

};

TGTextEntry* LogBookEntry::AddTextLine(string label_text, string init_text, string tooltip){
   // Add a single entry line with label to the dialog.
   int label_width = 90;
   auto Frame = new TGHorizontalFrame(fMain, 0, 50, kLHintsExpandX);
   auto LabelFrame = new TGHeaderFrame( Frame, label_width, 50, kFixedWidth);
   Frame->AddFrame(LabelFrame, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0));
   auto label = new TGLabel(LabelFrame, label_text.c_str());
   LabelFrame->AddFrame(label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
   auto TextEntry = new TGTextEntry(Frame, init_text.c_str(), 0);
   TextEntry->SetToolTipText(tooltip.c_str());
   Frame->AddFrame(TextEntry, new TGLayoutHints(kLHintsExpandX | kLHintsCenterY, 2, 2, 2, 2));
   fMain->AddFrame(Frame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 15, 5, 2));
   return TextEntry;
}



void LogBookEntry::MakeEntry() {

   if(!fAlreadyWritingImages) {
      //fEntryThread = std::thread(&RasterLogBookEntry::SaveCanvassesToFile, this);
      SaveCanvassesToFile();
   }else{
      std::cout << YELLOW "Ignore click, already entering a log message.\n" ENDC;
      return;  // Ignore this click.
   }

   fMain = new TGTransientFrame(gClient->GetRoot(), fParentWindow, 400, 400);
   fMain->SetWindowName("Logbook Entry Dialog");
   fMain->DontCallClose(); // to avoid double deletions.
   // use hierarchical cleaning
   fMain->SetCleanup(kDeepCleanup);
   // We spin off a thread to save the canvasses to file, otherwise the entire app blocks while these are being
   // written, and writing them can be slow. We use a lock mechanism (see fRHists->fDrawLock) to allow the thread to
   // write the files in batch mode.
   // Next we open the dialog window for the user to interact with.
   fMain->Connect("CloseWindow()", "RasterLogBookEntry", this, "CloseWindow()");

   auto Frame1 = new TGHorizontalFrame(fMain, 800, 50, kFixedWidth | kFitHeight);

   if(fLogEntryOK) {

      auto OkButton = new TGTextButton(Frame1, "&Ok", 991);
      OkButton->Connect("Clicked()", "RasterLogBookEntry", this, "OK()");
      Frame1->AddFrame(OkButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2));

      auto CancelButton = new TGTextButton(Frame1, "&Cancel", 990);
      CancelButton->Connect("Clicked()", "RasterLogBookEntry", this, "Cancel()");
      Frame1->AddFrame(CancelButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2));

      fTitle = "Run #" + to_string(fRHists->fEvio->GetLastRunNumber()) + ": RasterMon Histograms.";
      fTitleEntry = AddTextLine("Title:", fTitle, "Update the title for the logbook entery, or leave as is.");
//      fLogBooksEntry = AddTextLine("Logbook(s):", fLogBooks,
//                                   "A list of logbooks (separate by a comma) where this entry should be logged.");
      //////////////// Logbook dropdown chooser //////////////////////

      auto LogbookFrame = new TGHorizontalFrame(fMain, 300, 50, kFixedWidth);
      auto LLabelFrame = new TGHeaderFrame(LogbookFrame, 90, 50, kFixedWidth);
      LogbookFrame->AddFrame(LLabelFrame, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0));
      auto label = new TGLabel(LLabelFrame, "Logbook(s):");
      LLabelFrame->AddFrame(label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
      fLogBookCBox = new TGComboBox(LogbookFrame);
      fLogBookCBox->Resize(120, 20);
      for(int i=0; i < fLogBookChoices.size(); ++i){
         fLogBookCBox->AddEntry(fLogBookChoices[i].c_str(), i);
      }
      fLogBookCBox->Select(0);

      LogbookFrame->AddFrame(fLogBookCBox, new TGLayoutHints(kLHintsCenterY, 2, 2, 2, 2));
      fMain->AddFrame(LogbookFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 15, 5, 2));

      ////////////////// Tag chooser  /////////////////////////////////////////////////

      auto TagFrame = new TGHorizontalFrame(fMain, 300, 50, kFixedWidth);
      auto TLabelFrame = new TGHeaderFrame(TagFrame, 90, 50, kFixedWidth);
      TagFrame->AddFrame(TLabelFrame, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 0, 0, 0));
      auto tag_label = new TGLabel(TLabelFrame, "Tag(s):");
      TLabelFrame->AddFrame(tag_label, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
      fTagsCBox = new TGComboBox(TagFrame);
      fTagsCBox->Resize(120, 20);
      for(int i=0; i < fTagChoices.size(); ++i){
         fTagsCBox->AddEntry(fTagChoices[i].c_str(), i);
      }
      fTagsCBox->Select(0);

      TagFrame->AddFrame(fTagsCBox, new TGLayoutHints(kLHintsCenterY, 2, 2, 2, 2));
      fMain->AddFrame(TagFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 15, 5, 2));

      //////////////////////////////////////////////////////////////////////////////////
      fEntryMakersEntry = AddTextLine("Entry makers:", fEntryMakers,
                                      "A list of usernames (separated by commas) of individuals who should be associated with authorship of the entry.");
      fEmailNotifyEntry = AddTextLine("Email notify:", fEmailNotify,
                                      "A list of email addresses (separated by commas) to receive an email copy of the entry after it is posted.");

      fBodyEdit = new TGTextEdit(fMain, 0, 500);
      fMain->AddFrame(fBodyEdit, new TGLayoutHints(kLHintsExpandX, 10, 10, 10, 10));

      string body{"RasterMon auto generated logbook entry for run "};
      body += to_string(fRHists->fEvio->GetLastRunNumber()) + " around event number " + to_string(fRHists->fEvio->GetLastEventNumber());
      fBodyEdit->AddLine(body.c_str());
      fBodyEdit->AddLine("Please edit this entry with details.");
   }else{
      auto OkButton = new TGTextButton(Frame1, "&Ok", 991);
      OkButton->Connect("Clicked()", "RasterLogBookEntry", this, "CloseWindow()");
      Frame1->AddFrame(OkButton, new TGLayoutHints(kLHintsTop | kLHintsRight, 2, 2, 2, 2));

      auto Frame = new TGHorizontalFrame(fMain, 0, 200, kLHintsExpandX);
      auto TextMessage = new TGTextView( Frame, 500, 150);
      Frame->AddFrame(TextMessage, new TGLayoutHints(kLHintsExpandX| kLHintsTop, 5, 5, 5, 5));
      TextMessage->AddLine("We are sorry, but automatic log entry is not supported on this system.");
      TextMessage->AddLine("The 'logentry' program is not properly installed.");
      TextMessage->AddLine("Please make a log entry using the standard browser, linking the files:\n");
      for(auto att: fAttachments) {
        TextMessage->AddLine(att.c_str());
      }
      fMain->AddFrame(Frame, new TGLayoutHints(kLHintsExpandX | kLHintsTop));
   }

   fMain->AddFrame(Frame1, new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1));
   fMain->MapSubwindows();

   UInt_t width = fMain->GetDefaultWidth();
   UInt_t height = fMain->GetDefaultHeight();
   fMain->Resize(width, height);
   fMain->CenterOnParent();
   fMain->MapWindow();
//   gClient->WaitFor(fMain);
// #endif
}

void LogBookEntry::ScrubString(string &instr) {
   // Remove the single quote character from the string, so command line entry works.
   size_t pos=0;
   while( (pos = instr.find("'", pos)) < instr.size()){
      instr.replace(pos,1, "\"");
      pos++;
   }
}

void LogBookEntry::SubmitToLogBook() {
   std::cout << "Submit to logbook\n";

   fTitle = fTitleEntry->GetText();
   int selected_log = fLogBookCBox->GetSelected();
   fLogBooks = fLogBookChoices.at(selected_log);

   int selected_tag = fTagsCBox->GetSelected();
   fTags = fTagChoices.at(selected_tag);

   fEntryMakers = fEntryMakersEntry->GetText();
   fEmailNotify = fEmailNotifyEntry->GetText();
   auto body_tgtext = fBodyEdit->GetText();
   fBody = body_tgtext->AsString();

   while( fAlreadyWritingImages ){
      gSystem->ProcessEvents();
      gSystem->Sleep(50);
   }
   if(fEntryThread.joinable()) fEntryThread.join();
   // At this point the attachments should be ready.
   for(int i=0; i< fRHists->fTabs.size(); ++i){
      auto tab = fRHists->fTabs.at(i);
      fAttachmentCaptions.push_back("Histograms for tab " + tab.name);
   }

#ifdef HAS_LOGBOOK
   ELogBookShim lb;
   lb.fTitle = fTitle;
   lb.fLogBooks = fLogBooks;
   lb.fTags = fTitle;
   lb.fEntryMakers = fEntryMakers;
   lb.fEmailNotify = fEmailNotify;
   lb.fBody = fBody;
   lb.fAttachments = fAttachments;
   lb.fAttachmentCaptions = fAttachmentCaptions;
#else
   ScrubString(fBody);
   ScrubString(fTitle);
   ScrubString(fEntryMakers);
   ScrubString(fEmailNotify);
   string cmd = "echo '" + fBody + "' | " + CLI_LOGENTRY_PROGRAM;
   if(!fLogBooks.empty()) {
      stringstream ss(fLogBooks);
      while(ss.good()) {
         string substr;
         getline(ss, substr, ',');
         cmd += " -l " + substr + " ";
      }
   }else {
      cmd += " -l HBLOG ";
   }

   if(!fTitle.empty()){
      cmd += " --title '" + fTitle + "' ";
   }else{
      cmd += "--title 'Run #" + to_string(fRHists->fEvio->GetLastRunNumber()) + ": RasterMon Histograms.' ";
   }
   if(!fTags.empty()) {
      stringstream ss(fTags);
      while (ss.good()) {
         string substr;
         getline(ss, substr, ',');
         cmd += " --tag " + substr + " ";
      }
   }
   if(!fEntryMakers.empty()){
      stringstream ss(fEntryMakers);
      while (ss.good()) {
         string substr;
         getline(ss, substr, ',');
         cmd += "--entrymaker '" + substr + "' ";
      }
//      cmd += "--entrymaker '" + fEntryMakers +"' ";
   }
   if(!fEmailNotify.empty()) {
      stringstream ss(fEmailNotify);
      while (ss.good()) {
         string substr;
         getline(ss, substr, ',');
         cmd += " --notify '" + substr + "' ";
      }
   }
   for(int i=0; i< fAttachments.size(); ++i) {
      cmd += " --attach '" + fAttachments[i] + "' --caption '" + fAttachmentCaptions[i] + "' ";
   }

   cmd += " -b - ";

   if(fRHists->fDebug){
      std::cout << "Logbook command: \n";
      std::cout << cmd << std::endl;
   }
   std::system(cmd.c_str());

#endif
}

void LogBookEntry::SaveCanvassesToFile(){
   // This is where all the work is done for the logbook entry. Reminder: this runs multi-threaded,
   // so resources must be local to this thread, or global to all log entries open.
   //
   // First step: Snapshot of all the canvases, so that we get an accurate copy of the current state.
   // Here we do the work to save the images of the canvasses.
   fAlreadyWritingImages = true;
   std::vector<TCanvas *> canvs;
   bool prevstate = fRHists->Pause();                                        // Pause, so all histos and graphs are at the same event.
   std::vector<Histogram_t> histo_copy{fRHists->fHists};   // Copy the histograms and graphs. (vector copy assignment.)
   // std::vector<Graph_t> graph_copy{fRHists->fGraphs};   // Barfs on gcc 9.3.0 ????  TODO:: Figure out what is wrong here!!!
   std::vector<Graph_t> graph_copy;                        // So create the vector and fill it separately.
   graph_copy.insert(graph_copy.begin(),fRHists->fGraphs.begin(),fRHists->fGraphs.end());
   for(int i_tab=0; i_tab < fRHists->fTabs.size(); ++i_tab) {
      fRHists->FillGraphs(i_tab, graph_copy);       // Update the graph contents.
   }
   if(!prevstate) fRHists->UnPause(); // Don't unpause if we were paused before already.

   for(int i_tab=0; i_tab < fRHists->fTabs.size(); ++i_tab) {
      auto tab = fRHists->fTabs.at(i_tab);
      string cname = "print_" + to_string(i_tab);
      fRHists->fDrawLock.lock();
      gROOT->SetBatch(true);
      auto canvas = new TCanvas(cname.c_str(), cname.c_str(), 2000, 2000);
      fRHists->SetupCanvas(tab, canvas);
      gROOT->SetBatch(false);
      fRHists->fDrawLock.unlock();
      // Fill those canvasses with the histograms.
      fRHists->DrawCanvas(i_tab, canvas, histo_copy, graph_copy, true);
      canvs.push_back(canvas);
   }


   // Construct the filename as rastermon_2022_05_15_16_04_23_1.png
   char buf[50];
   time_t t_now = time(NULL);
   strftime(buf, 50, "rastermon_%Y_%m_%d_%H_%M_%S_", localtime(&t_now));
   string filename(buf);
   fAttachments = fRHists->SaveCanvasesToImageFiles(fHistogramPath + "/" + filename,"png", &canvs);
   //fRHists->SaveCanvasesToPDF(directory+filename, &canvs);
   canvs.clear();
   fAlreadyWritingImages = false;
   // End of thread.
}