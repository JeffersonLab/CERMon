//
// Created by Maurik Holtrop on 5/28/22.
//

#ifndef RASTERMON_RASTERMONDEFAULTCONFIG_H
#define RASTERMON_RASTERMONDEFAULTCONFIG_H

#include "CERMonHists.h"

#ifndef RASTER_CRATE
#define RASTER_CRATE 66
#endif
#ifndef RASTER_SLOT
#define RASTER_SLOT 19
#endif

void Default_Setup_Raster_Tab(CERMonHists *r);
void Default_Setup_RasterBit32_Tab(CERMonHists *r);
void Default_Setup_Raw_Raster_Tab(CERMonHists *r);
void Default_Setup_Raw_Raster_Tab2(CERMonHists *r);
void Default_Setup_Raw_Raster_Tab3(CERMonHists *r);
void Default_Setup_1_Channel_Scope(CERMonHists *r);
void Default_Setup_2_Channel_Scope(CERMonHists *r);
void Default_Setup_3_Channel_Scope(CERMonHists *r);
void Default_Setup_Helicity(CERMonHists *r);
void Default_Setup_Trigger(CERMonHists *r);
void Default_Initialize_Histograms(CERMonHists *r);

#endif //RASTERMON_RASTERMONDEFAULTCONFIG_H
