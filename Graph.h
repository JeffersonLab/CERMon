//
// Created by Maurik Holtrop on 5/17/22.
//

#ifndef RASTERMON_GRAPH_H
#define RASTERMON_GRAPH_H

#include "TGraph.h"
#include <string>

struct Graph_t {   // Object to hold the information for each scope channel for FADC readout.
   unsigned char bank_tag;
   unsigned char slot;
   unsigned char adc_chan;  // Channel for the slot of the FADC module.
   std::string  legend;      // Legend entry.
   std::string draw_opt; // Draw option.
   unsigned int color;  // Line color, as in kRed = 632.
   unsigned char width; // Line width
   bool show;      // Set false to not draw the TGraph, but still accumulate the data.
   int data_index; // Index into the data buffers. Filled when EVIO is setup.
   TGraph *graph = nullptr; // The TGraph object that actually contains the data.
   Graph_t(){};
   Graph_t(unsigned int bank_tag, unsigned char slot, unsigned char adc_chan,
           const std::string &name, const std::string &title, const std::string &legend, unsigned int color, unsigned char width, bool show):
         bank_tag(bank_tag), slot(slot), adc_chan(adc_chan), legend(legend), color(color), width(width), show(show) {
      data_index = -1;
      graph = new TGraph();
      graph->SetTitle(title.c_str());
      graph->SetName(name.c_str());
      graph->SetLineColor(color);
      graph->SetLineWidth(width);
      graph->SetMinimum(-1.);
      graph->SetMaximum(4096.);
   };
   ~Graph_t(){
      delete graph;
   }
   // Copy constructor needs a full copy of the graph, not a pointer copy.
   Graph_t(Graph_t &that){
      bank_tag = that.bank_tag;
      slot = that.slot;
      adc_chan = that.adc_chan;
      legend = that.legend;
      draw_opt =that.draw_opt;
      color = that.color;
      width = that.width;
      show = that.show;
      data_index = that.data_index;
      graph = new TGraph( *that.graph);
   };
   // Move constructor needs to set the moved pointer to nullptr, else we have delete issues.
   Graph_t(Graph_t &&that) noexcept{
      bank_tag = that.bank_tag;
      slot = that.slot;
      adc_chan = that.adc_chan;
      legend = std::move(that.legend);
      draw_opt = std::move(that.draw_opt);
      color = that.color;
      width = that.width;
      show = that.show;
      data_index = that.data_index;
      graph = std::exchange(that.graph, nullptr);
   }
   Graph_t &operator=(const Graph_t &that){
      if (&that == this) return *this; // If self assignment.
      bank_tag= that.bank_tag;
      slot = that.slot;
      adc_chan = that.adc_chan;
      legend = that.legend;
      draw_opt = that.draw_opt;
      color = that.color;
      width = that.width;
      show = that.show;
      data_index = that.data_index;
      graph = new TGraph( *that.graph);
      return *this;
   };
   Graph_t &operator=(Graph_t && that){
      bank_tag = that.bank_tag;
      slot = that.slot;
      adc_chan = that.adc_chan;
      legend = std::move(that.legend);
      draw_opt = std::move(that.draw_opt);
      color = that.color;
      width = that.width;
      show = that.show;
      data_index = that.data_index;
      graph = std::exchange(that.graph, nullptr);
      return *this;
   };
};

#endif //RASTERMON_GRAPH_H
