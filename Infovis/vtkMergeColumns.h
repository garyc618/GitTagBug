/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMergeColumns.h

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
// .NAME vtkMergeColumns - merge two columns into a single column
//
// .SECTION Description
// vtkMergeColumns replaces two columns in a table with a single column
// containing data in both columns.  The columns are set using
//
//   SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_ROWS, "col1")
//
// and
//
//   SetInputArrayToProcess(1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_ROWS, "col2")
//
// where "col1" and "col2" are the names of the columns to merge.
// The user may also specify the name of the merged column.
// The arrays must be of the same type.
// If the arrays are numeric, the values are summed in the merged column.
// If the arrays are strings, the values are concatenated.  The strings are
// separated by a space if they are both nonempty.

#ifndef __vtkMergeColumns_h
#define __vtkMergeColumns_h

#include "vtkTableAlgorithm.h"

class VTK_INFOVIS_EXPORT vtkMergeColumns : public vtkTableAlgorithm
{
public:
  static vtkMergeColumns* New();
  vtkTypeRevisionMacro(vtkMergeColumns,vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // The name to give the merged column created by this filter.
  vtkSetStringMacro(MergedColumnName);
  vtkGetStringMacro(MergedColumnName);

protected:
  vtkMergeColumns();
  ~vtkMergeColumns();
  
  char* MergedColumnName;

  int RequestData(
    vtkInformation*, 
    vtkInformationVector**, 
    vtkInformationVector*);

private:
  vtkMergeColumns(const vtkMergeColumns&); // Not implemented
  void operator=(const vtkMergeColumns&);   // Not implemented
};

#endif

