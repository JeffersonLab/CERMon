//
// Created by Maurik Holtrop on 5/28/22.
//

#ifndef RASTERMON_RASTERMONDEFAULTCONFIG_H
#define RASTERMON_RASTERMONDEFAULTCONFIG_H

#include "RasterHists.h"

#ifndef RASTER_CRATE
#define RASTER_CRATE 66
#endif
#ifndef RASTER_SLOT
#define RASTER_SLOT 19
#endif

void Default_Setup_Raster_Tab(RasterHists *r);
void Default_Setup_RasterBit32_Tab(RasterHists *r);
void Default_Setup_Raw_Raster_Tab(RasterHists *r);
void Default_Setup_Raw_Raster_Tab2(RasterHists *r);
void Default_Setup_Raw_Raster_Tab3(RasterHists *r);
void Default_Setup_1_Channel_Scope(RasterHists *r);
void Default_Setup_2_Channel_Scope(RasterHists *r);
void Default_Setup_3_Channel_Scope(RasterHists *r);
void Default_Setup_Helicity(RasterHists *r);
void Default_Setup_Trigger(RasterHists *r);
void Default_Initialize_Histograms(RasterHists *r);

#endif //RASTERMON_RASTERMONDEFAULTCONFIG_H
