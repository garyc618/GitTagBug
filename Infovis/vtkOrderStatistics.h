/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOrderStatistics.h

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
// .NAME vtkOrderStatistics - A class for univariate order statistics
//
// .SECTION Description
// Given a selection of columns of interest in an input data table, this 
// class provides the following functionalities, depending on the
// execution mode it is executed in:
// * Learn: calculate 5-point statistics (minimum, 1st quartile, median, third
//   quartile, maximum) and all other deciles (1,2,3,4,6,7,8,9).
// * Assess: given an input data set in port 0, and two percentiles p1 < p2,
//   assess all entries in the data set which are outside of [p1,p2].
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkOrderStatistics_h
#define __vtkOrderStatistics_h

#include "vtkUnivariateStatisticsAlgorithm.h"

class vtkStringArray;
class vtkTable;

class VTK_INFOVIS_EXPORT vtkOrderStatistics : public vtkUnivariateStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkOrderStatistics, vtkUnivariateStatisticsAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkOrderStatistics* New();

  // Description:
  // The type of quantile definition.
  //BTX
  enum QuantileDefinitionType {
    InverseCDF              = 0,
    InverseCDFAveragedSteps = 1,
    };
  //ETX

  // Description:
  // Set the number of quantiles (with uniform spacing).
  vtkSetMacro( NumberOfIntervals, vtkIdType );

  // Description:
  // Get the number of quantiles (with uniform spacing).
  vtkGetMacro( NumberOfIntervals, vtkIdType );

  // Description:
  // Set the quantile definition.
  vtkSetMacro( QuantileDefinition, QuantileDefinitionType );
  void SetQuantileDefinition ( int );

  // Description:
  // Get the quantile definition.
  vtkIdType GetQuantileDefinition() { return static_cast<vtkIdType>( this->QuantileDefinition ); }

//BTX  
  // Description:
  // Provide the appropriate assessment functor.
  virtual void SelectAssessFunctor( vtkTable* outData, 
                                    vtkDataObject* inMeta,
                                    vtkStringArray* rowNames,
                                    AssessFunctor*& dfunc );
//ETX

protected:
  vtkOrderStatistics();
  ~vtkOrderStatistics();

  // Description:
  // Execute the calculations required by the Learn option.
  virtual void ExecuteLearn( vtkTable* inData,
                             vtkDataObject* outMeta );

  // Description:
  // Execute the calculations required by the Derive option.
  virtual void ExecuteDerive( vtkDataObject* );

  vtkIdType NumberOfIntervals;
  QuantileDefinitionType QuantileDefinition;

private:
  vtkOrderStatistics(const vtkOrderStatistics&); // Not implemented
  void operator=(const vtkOrderStatistics&);   // Not implemented
};

#endif

