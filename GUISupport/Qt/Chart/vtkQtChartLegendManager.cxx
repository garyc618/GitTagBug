/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkQtChartLegendManager.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

/// \file vtkQtChartLegendManager.cxx
/// \date January 8, 2009

#ifdef _MSC_VER
// Disable warnings that Qt headers give.
#pragma warning(disable:4127)
#endif

#include "vtkQtChartLegendManager.h"

#include "vtkQtChartArea.h"
#include "vtkQtChartLayer.h"
#include "vtkQtChartLegend.h"
#include "vtkQtChartLegendModel.h"
#include "vtkQtChartSeriesLayer.h"
#include "vtkQtChartSeriesModel.h"

#include <QList>
#include <QPixmap>


class vtkQtChartLegendManagerInternal
{
public:
  vtkQtChartLegendManagerInternal();
  ~vtkQtChartLegendManagerInternal() {}

public:
  QList<vtkQtChartSeriesLayer *> Layers;
};


//-----------------------------------------------------------------------------
vtkQtChartLegendManagerInternal::vtkQtChartLegendManagerInternal()
  : Layers()
{
}


//-----------------------------------------------------------------------------
vtkQtChartLegendManager::vtkQtChartLegendManager(QObject *parentObject)
  : QObject(parentObject)
{
  this->Internal = new vtkQtChartLegendManagerInternal();
  this->Area = 0;
  this->Legend = 0;
}

vtkQtChartLegendManager::~vtkQtChartLegendManager()
{
}

void vtkQtChartLegendManager::setChartArea(vtkQtChartArea *area)
{
  if(this->Area)
    {
    // Disconnect from the chart layers.
    vtkQtChartSeriesModel *model = 0;
    QList<vtkQtChartSeriesLayer *>::Iterator iter =
        this->Internal->Layers.begin();
    for( ; iter != this->Internal->Layers.end(); ++iter)
      {
      if(*iter)
        {
        this->disconnect(*iter, 0, this, 0);
        model = (*iter)->getModel();
        if(model)
          {
          this->disconnect(model, 0, this, 0);
          }
        }
      }

    this->Internal->Layers.clear();

    // Disconnect from the chart area.
    this->disconnect(this->Area, 0, this, 0);
    }

  this->Area = area;
  if(this->Area)
    {
    // Listen for chart layer changes.
    this->connect(this->Area, SIGNAL(layerInserted(int, vtkQtChartLayer *)),
        this, SLOT(insertLayer(int, vtkQtChartLayer *)));
    this->connect(this->Area, SIGNAL(removingLayer(int, vtkQtChartLayer *)),
        this, SLOT(removeLayer(int, vtkQtChartLayer *)));

    // Add each of the chart layers to the list.
    int layers = this->Area->getNumberOfLayers();
    for(int i = 0; i < layers; i++)
      {
      this->insertLayer(i, this->Area->getLayer(i));
      }
    }
}

void vtkQtChartLegendManager::setChartLegend(vtkQtChartLegend *legend)
{
  if(this->Legend)
    {
    // Clean up the previous model.
    this->Legend->getModel()->removeAllEntries();
    }

  this->Legend = legend;
  if(this->Legend && this->Area)
    {
    // Add the chart layer series to the legend model.
    int index = 0;
    vtkQtChartSeriesModel *model = 0;
    vtkQtChartLegendModel *legendModel = this->Legend->getModel();
    QList<vtkQtChartSeriesLayer *>::Iterator iter =
        this->Internal->Layers.begin();
    for( ; iter != this->Internal->Layers.end(); ++iter)
      {
      if(*iter)
        {
        model = (*iter)->getModel();
        if(model)
          {
          int last = model->getNumberOfSeries() - 1;
          if(last >= 0)
            {
            this->insertLegendEntries(legendModel, index, *iter, model,
                0, last);
            index += last + 1;
            }
          }
        }
      }
    }
}

void vtkQtChartLegendManager::insertLayer(int index, vtkQtChartLayer *chart)
{
  // Add the chart layer to the list.
  vtkQtChartSeriesLayer *seriesLayer =
      qobject_cast<vtkQtChartSeriesLayer *>(chart);
  this->Internal->Layers.insert(index, seriesLayer);
  if(seriesLayer)
    {
    // Listen for series model and options changes.
    this->connect(seriesLayer,
        SIGNAL(modelChanged(vtkQtChartSeriesModel *, vtkQtChartSeriesModel *)),
        this,
        SLOT(changeModel(vtkQtChartSeriesModel *, vtkQtChartSeriesModel *)));
    this->connect(seriesLayer, SIGNAL(modelSeriesChanged(int, int)),
        this, SLOT(updateModelEntries(int, int)));
    this->connect(seriesLayer, SIGNAL(modelSeriesVisibilityChanged(int, bool)),
        this, SLOT(updateSeriesVisibility(int, bool)));

    vtkQtChartSeriesModel *model = seriesLayer->getModel();
    if(model)
      {
      this->connect(model, SIGNAL(modelAboutToBeReset()),
          this, SLOT(removeModelEntries()));
      this->connect(model, SIGNAL(modelReset()),
          this, SLOT(insertModelEntries()));
      this->connect(model, SIGNAL(seriesInserted(int, int)),
          this, SLOT(insertModelEntries(int, int)));
      this->connect(model, SIGNAL(seriesAboutToBeRemoved(int, int)),
          this, SLOT(removeModelEntries(int, int)));

      // Add the model's series to the legend model.
      int last = model->getNumberOfSeries() - 1;
      if(this->Legend && last >= 0)
        {
        int start = this->getLegendIndex(seriesLayer);
        vtkQtChartLegendModel *legend = this->Legend->getModel();
        legend->startModifyingData();
        this->insertLegendEntries(legend, start, seriesLayer, model, 0, last);
        legend->finishModifyingData();
        }
      }
    }
}

void vtkQtChartLegendManager::removeLayer(int index, vtkQtChartLayer *)
{
  if(index < 0 || index >= this->Internal->Layers.size())
    {
    return;
    }

  // Remove the chart layer from the list.
  vtkQtChartSeriesLayer *seriesLayer = this->Internal->Layers.takeAt(index);
  if(seriesLayer)
    {
    // Disconnect from the chart layer signals.
    this->disconnect(seriesLayer, 0, this, 0);

    vtkQtChartSeriesModel *model = seriesLayer->getModel();
    if(model)
      {
      // Disconnect from the model signals.
      this->disconnect(model, 0, this, 0);

      // Remove the model's series from the legend model.
      int last = model->getNumberOfSeries() - 1;
      if(this->Legend && last >= 0)
        {
        int start = this->getLegendIndex(seriesLayer);
        vtkQtChartLegendModel *legend = this->Legend->getModel();
        legend->startModifyingData();
        this->removeLegendEntries(legend, start, 0, last);
        legend->finishModifyingData();
        }
      }
    }
}

void vtkQtChartLegendManager::setLayerVisible(vtkQtChartLayer *chart,
    bool visible)
{
  vtkQtChartSeriesLayer *seriesLayer =
      qobject_cast<vtkQtChartSeriesLayer *>(chart);
  vtkQtChartSeriesModel *model = seriesLayer ? seriesLayer->getModel() : 0;
  if(model)
    {
    int last = model->getNumberOfSeries() - 1;
    if(last >= 0)
      {
      // Determine the starting index for the layer series.
      int index = this->getLegendIndex(seriesLayer);

      // Set the legend entry visibility.
      this->Legend->setEntriesVisible(index, index + last, visible);
      }
    }
}

void vtkQtChartLegendManager::changeModel(vtkQtChartSeriesModel *previous,
    vtkQtChartSeriesModel *current)
{
  // Get the chart layer from the sender.
  vtkQtChartSeriesLayer *chart =
      qobject_cast<vtkQtChartSeriesLayer *>(this->sender());
  if(chart)
    {
    // Determine the starting index for the layer series.
    int last = 0;
    int index = this->getLegendIndex(chart);

    vtkQtChartLegendModel *legend = this->Legend->getModel();
    legend->startModifyingData();

    // Remove the previous model's series.
    if(previous)
      {
      last = previous->getNumberOfSeries() - 1;
      if(last >= 0)
        {
        this->removeLegendEntries(legend, index, 0, last);
        }
      }

    // Add series for the current model.
    if(current)
      {
      last = current->getNumberOfSeries() - 1;
      if(last >= 0)
        {
        this->insertLegendEntries(legend, index, chart, current, 0, last);
        }
      }

    legend->finishModifyingData();
    }
}

void vtkQtChartLegendManager::updateSeriesVisibility(int series, bool visible)
{
  // Get the chart layer from the sender.
  vtkQtChartSeriesLayer *chart =
      qobject_cast<vtkQtChartSeriesLayer *>(this->sender());
  if(chart)
    {
    // Determine the starting index for the layer series.
    int index = this->getLegendIndex(chart);

    // Set the legend entry visibility.
    this->Legend->setEntryVisible(index + series, visible);
    }
}

void vtkQtChartLegendManager::updateModelEntries(int first, int last)
{
  // Get the chart layer from the sender.
  vtkQtChartSeriesLayer *chart =
      qobject_cast<vtkQtChartSeriesLayer *>(this->sender());
  if(chart)
    {
    vtkQtChartSeriesModel *model = chart->getModel();
    if(model)
      {
      // Determine the starting index for the layer series.
      int index = this->getLegendIndex(chart);

      // Update the icon and text for the given series.
      vtkQtChartLegendModel *legend = this->Legend->getModel();
      for(int i = first; i <= last; i++)
        {
        legend->setText(index + i, model->getSeriesName(i).toString());
        legend->setIcon(index + i, chart->getSeriesIcon(i));
        }
      }
    }
}

void vtkQtChartLegendManager::insertModelEntries()
{
  // Get the series model from the sender.
  vtkQtChartSeriesModel *model =
      qobject_cast<vtkQtChartSeriesModel *>(this->sender());
  if(model)
    {
    int last = model->getNumberOfSeries() - 1;
    if(last >= 0)
      {
      vtkQtChartSeriesLayer *chart = 0;
      int index = this->getLegendIndex(model, &chart);
      vtkQtChartLegendModel *legend = this->Legend->getModel();
      legend->startModifyingData();
      this->insertLegendEntries(legend, index, chart, model, 0, last);
      legend->finishModifyingData();
      }
    }
}

void vtkQtChartLegendManager::insertModelEntries(int first, int last)
{
  // Get the series model from the sender.
  vtkQtChartSeriesModel *model =
      qobject_cast<vtkQtChartSeriesModel *>(this->sender());
  if(model)
    {
    vtkQtChartSeriesLayer *chart = 0;
    int index = this->getLegendIndex(model, &chart);
    vtkQtChartLegendModel *legend = this->Legend->getModel();
    legend->startModifyingData();
    this->insertLegendEntries(legend, index, chart, model, first, last);
    legend->finishModifyingData();
    }
}

void vtkQtChartLegendManager::removeModelEntries()
{
  // Get the series model from the sender.
  vtkQtChartSeriesModel *model =
      qobject_cast<vtkQtChartSeriesModel *>(this->sender());
  if(model)
    {
    int last = model->getNumberOfSeries() - 1;
    if(last >= 0)
      {
      int index = this->getLegendIndex(model);
      vtkQtChartLegendModel *legend = this->Legend->getModel();
      legend->startModifyingData();
      this->removeLegendEntries(legend, index, 0, last);
      legend->finishModifyingData();
      }
    }
}

void vtkQtChartLegendManager::removeModelEntries(int first, int last)
{
  // Get the series model from the sender.
  vtkQtChartSeriesModel *model =
      qobject_cast<vtkQtChartSeriesModel *>(this->sender());
  if(model)
    {
    int index = this->getLegendIndex(model);
    vtkQtChartLegendModel *legend = this->Legend->getModel();
    legend->startModifyingData();
    this->removeLegendEntries(legend, index, first, last);
    legend->finishModifyingData();
    }
}

int vtkQtChartLegendManager::getLegendIndex(vtkQtChartSeriesLayer *chart)
{
  int index = 0;
  QList<vtkQtChartSeriesLayer *>::Iterator iter =
      this->Internal->Layers.begin();
  for( ; iter != this->Internal->Layers.end(); ++iter)
    {
    if(*iter == chart)
      {
      break;
      }
    else if(*iter)
      {
      vtkQtChartSeriesModel *model = (*iter)->getModel();
      if(model)
        {
        index += model->getNumberOfSeries();
        }
      }
    }

  return index;
}

int vtkQtChartLegendManager::getLegendIndex(vtkQtChartSeriesModel *model,
    vtkQtChartSeriesLayer **chart)
{
  int index = 0;
  QList<vtkQtChartSeriesLayer *>::Iterator iter =
      this->Internal->Layers.begin();
  for( ; iter != this->Internal->Layers.end(); ++iter)
    {
    if(*iter)
      {
      vtkQtChartSeriesModel *seriesModel = (*iter)->getModel();
      if(seriesModel == model)
        {
        if(chart)
          {
          *chart = *iter;
          }

        break;
        }
      else if(seriesModel)
        {
        index += seriesModel->getNumberOfSeries();
        }
      }
    }

  return index;
}

void vtkQtChartLegendManager::insertLegendEntries(
    vtkQtChartLegendModel *legend, int index, vtkQtChartSeriesLayer *chart,
    vtkQtChartSeriesModel *model, int first, int last)
{
  for(int i = first; i <= last; i++)
    {
    legend->insertEntry(index + i, chart->getSeriesIcon(i),
        model->getSeriesName(i).toString());
    }
}

void vtkQtChartLegendManager::removeLegendEntries(
    vtkQtChartLegendModel *legend, int index, int first, int last)
{
  first += index;
  for(int i = last + index; i >= first; i--)
    {
    legend->removeEntry(i);
    }
}


