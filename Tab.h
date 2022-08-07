//
// Created by Maurik Holtrop on 5/17/22.
//

#ifndef RASTERMON_TAB_H
#define RASTERMON_TAB_H

#include "TRootEmbeddedCanvas.h"
#include "Graph.h"
#include "Histogram.h"
#include <string>

struct TabSpace_t{  // Object for each of the tabs.
   std::string name;
   TRootEmbeddedCanvas *canvas;  // Canvas to hold the histograms and graphs in a tab.
   unsigned int nx;      // Divisions in x.
   unsigned int ny;      // Divisions in y.
   unsigned int npads;   // usually = to nx*ny, unless you do custom pad layout.
   float x_margin;
   float y_margin;
   std::vector<bool> logy; // true of false for each pad.
   std::vector<int> grid; // true of false for each pad.
   std::vector<int> calc;  // Number for any special calculation that may be needed for a pad.
   std::vector<int> pad_link; // List of how pads are linked for x-axis updates. for 3 pads, set to [2, 3, 1], so pad1 updates pad2, etc.
   std::vector< unsigned short > graphs;  // Index for the Scope graphs into fGraphs. (If you want pointers, then Graph_t needs to be more complicated.)
   std::vector< unsigned short > graph_pads; // The list of pads where each graph is drawn.
   std::vector< unsigned short > hists;     // Index for the Histogram into fHists.
   std::vector< unsigned short > hist_pads; // The list of pads where each histogram is drawn.
   TabSpace_t(std::string name,unsigned int nx, unsigned int ny, float x_margin=0.01, float y_margin = 0.01):
         name(std::move(name)), nx(nx), ny(ny), npads(nx*ny), x_margin(x_margin), y_margin(y_margin), canvas(nullptr){ };
};

#endif //RASTERMON_TAB_H
