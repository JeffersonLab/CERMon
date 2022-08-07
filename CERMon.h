#ifndef CERMon_h
#define CERMon_h

#define CERMON_VERSION "1.4"
#undef  ET_DEFAULT_STATION
#define ET_DEFAULT_STATION "CERMon"

#ifndef CERMON_DEFAULT_JSON_CONFIG
#define CERMON_DEFAULT_JSON_CONFIG "CERMonConfig.json"
#endif

#ifndef CERMON_DEFAULT_JSON_CONFIG_PATH
#define CERMON_DEFAULT_JSON_CONFIG_PATH "/home/clasrun"
#endif


#include <TROOT.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TRandom3.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGFrame.h>
#include "TH1D.h"
#include "TH2D.h"
#include "TTimer.h"

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include "CERMonConfigInfo.h"
#include "CERMonGui.h"
#include "CERMonEvioTool.h"
#include "CERMonEventInfo.h"
#include "CERMonDefaultConfig.h"
#include "cxxopts.hpp"

#define RED "\033[91m"
#define BLUE "\033[94m"
#define CYAN  "\033[96m"
#define GREEN  "\033[92m"
#define YELLOW "\033[93m"
#define RED "\033[91m"
#define ENDC "\033[0m"

#endif