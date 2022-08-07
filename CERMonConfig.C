//
// Created by Maurik Holtrop on 5/24/22.
//
// This is a RasterMon Configuration file.
// The code is essentially a ROOT C++ macro that will be executed inside RasterMon so that it can be configured fully.
// Inside RasterMon, the code is executed as:  .x RMConfig.C(0x123456789)  - where 0x123456789 is a long integer that
// contains the location of the RHists class.
// This means several things:
// 1) You have to write valid C++ statements.
// 2) Just about anything can be modified in the config file. Too much to make a manual for it.
// 3) If you make a mistake, the code will most likely crash.
//
// Layout of this file:
// The first section is needed so that the library and header files can be found.
// Next is the name of the function, which *must* be the same as the name of the file (except the .C).
// The first line of the function is needed to convert the long integer to a pointer. The pointer then allows
// you to manipulate all configurable aspects of the program.
//
#include "TROOT.h"
R__ADD_INCLUDE_PATH(/home/clasrun/RasterMon)   // Path to the code for locating the header files.
R__ADD_LIBRARY_PATH(/home/clasrun/RasterMon/lib)
R__LOAD_LIBRARY(libRasterLib)

#include "RasterMonDefaultConfig.h"

void RMConfig(long r_h_pointer_long) {  // Function name *must* be the same as the filename. Argument must be long.
   RasterHists *r = (RasterHists *)r_h_pointer_long;  // This is like, super dangerous and makes me really unhappy.

   if(r->fDebug) printf("We are configuring RasterHists from pointer 0x%lx \n",r_h_pointer_long);

   r->SetAutoClearRate(30.);
   r->SetAutoClearRateOff();

   // Note: PAD numbering starts at 1, with 0 being the Canvas (i.e. only one object on the screen)
   Default_Setup_Raster_Tab(r);
   Default_Setup_RasterBit32_Tab(r);
   Default_Setup_Raw_Raster_Tab(r);
   Default_Setup_Raw_Raster_Tab2(r);
   Default_Setup_Raw_Raster_Tab3(r);
   Default_Setup_1_Channel_Scope(r);
   Default_Setup_3_Channel_Scope(r);
   Default_Setup_Helicity(r);
   Default_Setup_Trigger(r);

   // Overrides
   cout << "Changing the Scope buffer size.\n";
   if (r->fEvio != nullptr) r->fEvio->UpdateBufferSize(10000);
}