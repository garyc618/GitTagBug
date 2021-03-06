/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTableToSparseArray.cxx
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLongLongArray.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSparseArray.h"
#include "vtkStdString.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableToSparseArray.h"

#include <vtksys/stl/algorithm>

class vtkTableToSparseArray::implementation
{
public:
  vtkstd::vector<vtkStdString> Coordinates;
  vtkStdString Values;
};

// ----------------------------------------------------------------------

vtkCxxRevisionMacro(vtkTableToSparseArray, "1.1");
vtkStandardNewMacro(vtkTableToSparseArray);

// ----------------------------------------------------------------------

vtkTableToSparseArray::vtkTableToSparseArray() :
  Implementation(new implementation())
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

// ----------------------------------------------------------------------

vtkTableToSparseArray::~vtkTableToSparseArray()
{
  delete this->Implementation;
}

// ----------------------------------------------------------------------

void vtkTableToSparseArray::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  for(vtkIdType i = 0; i != this->Implementation->Coordinates.size(); ++i)
    os << indent << "CoordinateColumn: " << this->Implementation->Coordinates[i] << endl;
  os << indent << "ValueColumn: " << this->Implementation->Values << endl;
}

void vtkTableToSparseArray::ClearCoordinateColumns()
{
  this->Implementation->Coordinates.clear();
  this->Modified();
}

void vtkTableToSparseArray::AddCoordinateColumn(const char* name)
{
  if(!name)
    {
    vtkErrorMacro(<< "cannot add coordinate column with NULL name");
    return;
    }
    
  this->Implementation->Coordinates.push_back(name);
  this->Modified();
}

void vtkTableToSparseArray::SetValueColumn(const char* name)
{
  if(!name)
    {
    vtkErrorMacro(<< "cannot set value column with NULL name");
    return;
    }

  this->Implementation->Values = name;
  this->Modified();
}

const char* vtkTableToSparseArray::GetValueColumn()
{
  return this->Implementation->Values.c_str();
}

int vtkTableToSparseArray::FillInputPortInformation(int port, vtkInformation* info)
{
  switch(port)
    {
    case 0:
      info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
      return 1;
    }

  return 0;
}

// ----------------------------------------------------------------------

int vtkTableToSparseArray::RequestData(
  vtkInformation*, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  vtkTable* const table = vtkTable::GetData(inputVector[0]);

  vtkstd::vector<vtkAbstractArray*> coordinates(this->Implementation->Coordinates.size());
  for(vtkIdType i = 0; i != this->Implementation->Coordinates.size(); ++i)
    {
    coordinates[i] = table->GetColumnByName(this->Implementation->Coordinates[i].c_str());
    if(!coordinates[i])
      {
      vtkErrorMacro(<< "missing coordinate array: " << this->Implementation->Coordinates[i].c_str());
      }
    }

  if(vtkstd::count(coordinates.begin(), coordinates.end(), static_cast<vtkAbstractArray*>(0)))
    {
    return 0;
    }
  
  vtkAbstractArray* const values = table->GetColumnByName(this->Implementation->Values.c_str());
  if(!values)
    {
    vtkErrorMacro(<< "missing value array: " << this->Implementation->Values.c_str());
    return 0;
    }

  vtkSparseArray<double>* const array = vtkSparseArray<double>::New();
  array->Resize(vtkArrayExtents::Uniform(coordinates.size(), 0));

  for(vtkIdType i = 0; i != coordinates.size(); ++i)
    array->SetDimensionLabel(i, coordinates[i]->GetName());

  vtkArrayCoordinates output_coordinates;
  output_coordinates.SetDimensions(coordinates.size());
  for(vtkIdType i = 0; i != table->GetNumberOfRows(); ++i)
    {
    for(vtkIdType j = 0; j != coordinates.size(); ++j)
      {
      output_coordinates[j] = coordinates[j]->GetVariantValue(i).ToInt();
      }
    array->AddValue(output_coordinates, values->GetVariantValue(i).ToDouble());
    }

  array->ResizeToContents();

  vtkArrayData* const output = vtkArrayData::GetData(outputVector);
  output->SetArray(array);
  array->Delete();

  return 1;
}

