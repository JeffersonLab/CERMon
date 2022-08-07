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

void Default_Initialize_Histograms(CERMonHists *r);

#endif //RASTERMON_RASTERMONDEFAULTCONFIG_H
