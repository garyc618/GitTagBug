/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkUnivariateStatisticsAlgorithm.h

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
// .NAME vtkUnivariateStatistics - Base class for univariate statistics 
// algorithms
//
// .SECTION Description
// This class specializes statistics algorithms to the univariate case, where
// a number of columns of interest can be selected in the input data set.
// This is done by the means of the following functions:
//
// ResetColumns() - reset the list of columns of interest.
// Add/RemoveColum( namCol ) - try to add/remove column with name namCol to/from
// the list.
// SetColumnStatus ( namCol, status ) - mostly for UI wrapping purposes, try to 
// add/remove (depending on status) namCol from the list of columns of interest.
// The verb "try" is used in the sense that neither attempting to 
// repeat an existing entry nor to remove a non-existent entry will work.
// 
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this class.

#ifndef __vtkUnivariateStatisticsAlgorithm_h
#define __vtkUnivariateStatisticsAlgorithm_h

#include "vtkStatisticsAlgorithm.h"

class vtkUnivariateStatisticsAlgorithmPrivate;
class vtkTable;

class VTK_INFOVIS_EXPORT vtkUnivariateStatisticsAlgorithm : public vtkStatisticsAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkUnivariateStatisticsAlgorithm, vtkStatisticsAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reset list of columns of interest
  void ResetColumns();

  // Description:
  // Add column name \p namCol to the list of columns of interest
  // Warning: no name checking is performed on \p namCol; it is the user's
  // responsibility to use valid column names.
  void AddColumn( const char* namCol );

  // Description:
  // Remove (if it exists) column name \p namCol to the list of columns of interest
  void RemoveColumn( const char* namCol );

  // Description:
  // Method for UI to call to add/remove columns to/from the list
  void SetColumnStatus( const char* namCol, int status );

  // Description:
  // Execute the calculations required by the Assess option.
  virtual void ExecuteAssess( vtkTable* inData,
                              vtkDataObject* inMeta,
                              vtkTable* outData,
                              vtkDataObject* outMeta ); 

protected:
  vtkUnivariateStatisticsAlgorithm();
  ~vtkUnivariateStatisticsAlgorithm();

  vtkUnivariateStatisticsAlgorithmPrivate* Internals;

private:
  vtkUnivariateStatisticsAlgorithm(const vtkUnivariateStatisticsAlgorithm&); // Not implemented
  void operator=(const vtkUnivariateStatisticsAlgorithm&);   // Not implemented
};

#endif

