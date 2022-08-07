//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                        CERMon
//                  CLAS12 EVIO Rapid Monitoring
//
// Author: Maurik Holtrop  - UNH - 2022
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Some notes:
// In general you want a GUI to be multithreaded. It turns out that this is trickier than it should be, for many
// reasons, the biggest of which is that we are dealing with lots of legacy code. XLib, the windowing used on Linux
// is still not properly thread safe! (Yes, Metal, Cocoa and Windows are, but we use Linux at the lab.)
// We are not using ROOT 7 yet, and ROOT 6.x is also not properly thread safe, see
// https://root.cern.ch/root/htmldoc/guides/users-guide/Threads.html
// Also note that officially the TThread class in ROOT is obsolete, so probably don't use it.
//
// The main implementations issues then for a GUI code are that *all the GUI stuff* has to be done from the main
// thread. This can be okay with cooperative multi-tasking, i.e. a loop that checks for signals, which is how the
// GUI is implemented, Qt style. (I am not using Qt here, though I would like to, but because of the difficulty with
// embedding a TCanvas in a Qt app, which seems fragile at this point, I decided not to.
// ROOT7 and QtWeb may improve this in the the future.)
//
// TODO:: Currently the histograms are not following the ROOT::TThreadedObject<TH1D> to create the histograms.
//        We are filling histograms in a single thread right now.
//
// TODO:: Make this work properly with ROOT7, and the ROOT7 browser based histograms.
//        Useful thread: https://root-forum.cern.ch/t/is-one-canvas-per-thread-possible/47014/27
//


#include <TROOT.h>
#include <TApplication.h>
#include <TGClient.h>
#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include "CERMon.h"
#include "CERMonConfigInfo.h"

extern void Initialize_Histograms(CERHists *r);

int main(int argc, char **argv) {
   ROOT::EnableThreadSafety();

   // This is a nicer way to do options in C++. See cxxopts.hpp file.
   std::string help_string =
         GREEN " ===============  CERMon ===============\n" ENDC
         " This is a GUI code that will monitor EVIO properties for CLAS12. \n"
         " You should be able to just run the code and the defaults will work with the CLAS12 ET ring.\n"
         " You can also supply a file name, or a list of filenames, on the command line, or through the GUI \n"
         " (File -> Open ) to have the code analyze the data in these EVIO files.\n"
         "You can specify a detailed config file with --config. This file is a ROOT macro that will be parsed.\n"
         "Please look at the examples and the code for details, since this is fairly expert level. \n\n"
         " Version: " CERMON_VERSION ", using ROOT version: ";
   help_string += gROOT->GetVersion();
   help_string += "\n Compiled with gcc " __VERSION__ " with ABI " + std::to_string(__GXX_ABI_VERSION) + " \n";
#ifdef HAS_LOGBOOK
   help_string += " Code submits directly to the logbook using the libelog c++ api.\n";
#else
   help_string += "Code submits to the logbook using the logentry cli program. \n\n";
#endif

   cxxopts::Options options(argv[0], help_string);
   options
         .positional_help(" infile1 infile2 ...")
         .show_positional_help();

   std::string host;
   unsigned int port = 11111;
   std::string etname;
   std::string station;
   std::string config_file;
   std::string json_config_file;

   options.add_options()
      ("d,debug", "Increase debug level")
      ("q,quiet", "Run extra quiet.")
      ("et", "Connect to ET instead of reading from file. ")
      ("h,host", "Host computer for the ET system, default: " ET_DEFAULT_HOST,
            cxxopts::value(host)->default_value(ET_DEFAULT_HOST))
      ("p,port", "Port to use for the ET system, default: " + to_string(ET_DEFAULT_PORT),
            cxxopts::value(port)->default_value(to_string(ET_DEFAULT_PORT)))
      ("n,etname", "filename for ET system direct reads, default: " ET_DEFAULT_NAME,
            cxxopts::value(etname)->default_value(ET_DEFAULT_NAME))
      ("s,station", "The ET station name, default: " ET_DEFAULT_STATION,
            cxxopts::value(station)->default_value(ET_DEFAULT_STATION))
      ("C,config", "Configuration root macro (C) file. default (none)",
            cxxopts::value(config_file)->default_value(""))
      ("j,json", "JSON Configuration file to set Config panel initial values. (CERMonConfig.json)",
            cxxopts::value(json_config_file)->default_value(RASTERMON_DEFAULT_JSON_CONFIG))
      ("i,inputfiles","List of input evio files. This option is optional. ",
            cxxopts::value<std::vector<std::string>>())
      ("help", "Print help")
      ;

   options.parse_positional({"inputfiles"});

   try {
      auto args = options.parse(argc, argv);
      if (args.count("help")) {
         std::cout << options.help() << std::endl;
         exit(0);
      }

      int debug = args.count("debug");       // Debug: 0 = info,  1 = more info, 2+ debug info.
      if (args.count("quiet") > 0) debug = -1; // Debug: -1 = no info.
      
      TApplication theApp("App", &argc, argv);
      // TRint theApp("App", &argc, argv);


      auto evio = new CERMonEvioTool();
      auto RHists = new CERHists(evio);
      auto info = new CERMonMonConfigInfo(RHists, evio);
      auto CERMonmon = new CERMonMonGui(info, RHists, gClient->GetRoot(), 800, 600);

      if (debug == 1) evio->fDebug = 0;
      if (debug > 1) evio->fDebug = EvioTool::EvioTool_Debug_Info;
      if (debug > 2) evio->fDebug += EvioTool::EvioTool_Debug_Info2;
      if (debug > 3) evio->fDebug += EvioTool::EvioTool_Debug_L1;
      if (debug > 4) evio->fDebug += EvioTool::EvioTool_Debug_L2;
      RHists->SetDebug(debug);

      // Parse the Config file FIRST. Then have command line options potentially override some of them (i.e. for ET)
      if( args.count("config")){
         std::cout << "Config file: " << config_file << " specified on command line. Parsing it.\n";
         if(std::filesystem::exists( std::filesystem::path(config_file))){
            std::ostringstream process_line;
            process_line << ".x " << config_file << "(";
            process_line << RHists;
            process_line << ")";
            std::cout << "Parsing config file with line: " << process_line.str() << std::endl;
            gROOT->ProcessLine(process_line.str().c_str());
         }else{
            std::cout << YELLOW "Configuration file " << config_file << " not found. Please check the name and path. Exit. \n" ENDC;
            return 1;
         }
      }else {
         Default_Initialize_Histograms(RHists);
      }
      RHists->SetupData();

      if( args.count("json")) {
         std::cout << GREEN "Initializing config values from JSON file: " << json_config_file << "\n" ENDC;
      }

      auto path_json_config_file = std::filesystem::path(json_config_file);
      try {
         if (!std::filesystem::exists(path_json_config_file)) {     // File not found.
            string home_dir{std::getenv("HOME")};
            auto home_path = std::filesystem::path(home_dir);
            auto filename = path_json_config_file.filename();
            auto check_path = home_path / filename;
            if (std::filesystem::exists(check_path)) {
               json_config_file = check_path.string();     // The file was found in the home dir.
               std::cout << BLUE "Initializing config values from JSON file: " << json_config_file << "\n" ENDC;
            }
         }
      }catch(std::filesystem::filesystem_error const& ex) {
            std::cout << RED "Filesystem exception \n"
                  << "what():  " << ex.what() << '\n'
                  << "path1(): " << ex.path1() << '\n'
                  << "path2(): " << ex.path2() << '\n'
                  << "code().value():    " << ex.code().value() << '\n'
                  << "code().message():  " << ex.code().message() << '\n'
                  << "code().category(): " << ex.code().category().name() << "\n" ENDC;
      }

      try {
         if (std::filesystem::exists(std::filesystem::path(json_config_file))) {
            if (debug)
               std::cout << GREEN "Initializing config values from JSON file: " << json_config_file << "\n" ENDC;
            info->fJSONFile = json_config_file;
            info->LoadFromJSON();
            info->PutValues();
         } else {
            if (args.count("json")) {
               std::cout << RED "Specified JSON configuration file not found. Exit. \n" ENDC;
               return 1;
            } else {
               std::cout
                     << BLUE "Default JSON configuration file not found. Continue with hard coded values.\n" ENDC;
            }
         }
      }catch(std::filesystem::filesystem_error const& ex) {
         std::cout << RED << "Filesystem Exception\n"
               << "what():  " << ex.what() << '\n'
               << "path1(): " << ex.path1() << '\n'
               << "path2(): " << ex.path2() << '\n'
               << "code().value():    " << ex.code().value() << '\n'
               << "code().message():  " << ex.code().message() << '\n'
               << "code().category(): " << ex.code().category().name() << '\n';
         std::cout << "The error prevented a JSON configuration to be loaded. \n" ENDC;
         return 1;
      }

      // Add the commandline files to the CERMonEvioTool
      if (args.count("inputfiles")) {
         auto &infiles = args["inputfiles"].as<std::vector<std::string>>();
         for (auto &v: infiles) {
            if (debug) cout << "Adding " << v << " \n";
            evio->AddFile(v.c_str());
         }
      }

      if(args.count("host") || evio->fETHost.empty() ) evio->SetETHost(host);
      if(args.count("port") || evio->fETPort == 0) evio->SetETPort(port);
      if(args.count("etname") || evio->fETName.empty()) evio->SetETName(etname);
      if(args.count("station") || evio->fETStationName.empty()) evio->SetETStation(station);

      evio->SetWaitMode(ET_ASYNC);

      if (args.count("et") || args.count("inputfiles") == 0) {
         if (debug)
            cout << "Using the ET system with host: " << host << ", port: " << port << " , et file: " << etname
                 << ". \n";
         try {
            int stat = evio->OpenEt(station, etname, host, port);
            if (stat != 0) {
               cout << RED "ERROR -- could not attach to ET system. abort. \n" ENDC;
            }else{
               cermon->StartEvioStatusCheckTimer();
            }
         }catch(exception e){
            std::cout << RED "Error connecting to ET caused exception.\n";
            std::cout << e.what() << "\n" ENDC;
         }
      }

      cermon->SetupGUI();
      cermon->fDebug = debug;

      theApp.Run();

   }catch(const cxxopts::OptionException &e){
      cout << e.what() << endl;
      std::cout << options.help() << std::endl;
      return 1;
   }

   return 0;
}
