/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkDescriptiveStatistics.h

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
// .NAME vtkDescriptiveStatistics - A class for univariate descriptive statistics
//
// .SECTION Description
// Given a selection of columns of interest in an input data table, this 
// class provides the following functionalities, depending on the
// execution mode it is executed in:
// * Learn: calculate extremal values, arithmetic mean, unbiased variance 
//   estimator, skewness estimator, and both sample and G2 estimation of the 
//   kurtosis excess. More precisely, ExecuteLearn calculates the sums; if
//   \p finalize is set to true (default), the final statistics are calculated
//   with CalculateFromSums. Otherwise, only raw sums are output; this 
//   option is made for efficient parallel calculations.
//   Note that CalculateFromSums is a static function, so that it can be used
//   directly with no need to instantiate a vtkDescriptiveStatistics object.
// * Assess: given an input data set in port 0, and a reference value x along
//   with an acceptable deviation d>0, assess all entries in the data set which
//   are outside of [x-d,x+d].
//
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkDescriptiveStatistics_h
#define __vtkDescriptiveStatistics_h

#include "vtkUnivariateStatisticsAlgorithm.h"

class vtkStringArray;
class vtkTable;

class VTK_INFOVIS_EXPORT vtkDescriptiveStatistics : public vtkUnivariateStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkDescriptiveStatistics, vtkUnivariateStatisticsAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkDescriptiveStatistics* New();

  // Description:
  // Set/get whether the deviations returned should be signed, or should
  // only have their magnitude reported.
  // The default is that signed deviations will be computed.
  vtkSetMacro(SignedDeviations,int);
  vtkGetMacro(SignedDeviations,int);
  vtkBooleanMacro(SignedDeviations,int);

  // Description:
  // A convenience method (in particular for UI wrapping) to set the name of the
  // column that contains the nominal value for the Assess option.
  void SetNominalParameter( const char* name );

  // Description:
  // A convenience method (in particular for UI wrapping) to set the name of the
  // column that contains the deviation for the Assess option.
  void SetDeviationParameter( const char* name );

protected:
  vtkDescriptiveStatistics();
  ~vtkDescriptiveStatistics();

  // Description:
  // Execute the calculations required by the Learn option.
  virtual void ExecuteLearn( vtkTable* inData,
                             vtkDataObject* outMeta );

  // Description:
  // Execute the calculations required by the Derive option.
  virtual void ExecuteDerive( vtkDataObject* );

  int SignedDeviations;

//BTX  
  // Description:
  // Provide the appropriate assessment functor.
  virtual void SelectAssessFunctor( vtkTable* outData, 
                                    vtkDataObject* inMeta,
                                    vtkStringArray* rowNames,
                                    AssessFunctor*& dfunc );
//ETX

private:
  vtkDescriptiveStatistics( const vtkDescriptiveStatistics& ); // Not implemented
  void operator = ( const vtkDescriptiveStatistics& );   // Not implemented
};

#endif

