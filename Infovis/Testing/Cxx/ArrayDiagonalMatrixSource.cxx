/*=========================================================================

  Program:   Visualization Toolkit
  Module:    ArrayDiagonalMatrixSource.cxx
  
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

#include <vtkArrayData.h>
#include <vtkArrayPrint.h>
#include <vtkDenseArray.h>
#include <vtkDiagonalMatrixSource.h>
#include <vtkSmartPointer.h>
#include <vtkSparseArray.h>

#include <vtksys/ios/iostream>
#include <vtksys/stl/stdexcept>

#define test_expression(expression) \
{ \
  if(!(expression)) \
    throw vtkstd::runtime_error("Expression failed: " #expression); \
}

int ArrayDiagonalMatrixSource(int argc, char* argv[])
{
  try
    {
    vtkSmartPointer<vtkDiagonalMatrixSource> source = vtkSmartPointer<vtkDiagonalMatrixSource>::New();
    source->SetExtents(3);
    source->SetArrayType(vtkDiagonalMatrixSource::SPARSE);
    source->SetDiagonal(1.0);
    source->SetSuperDiagonal(0.5);
    source->SetSubDiagonal(-0.5);
    source->Update();

    vtkSparseArray<double>* const sparse_array = vtkSparseArray<double>::SafeDownCast(source->GetOutput()->GetArray());
    
    cout << "sparse diagonal matrix:\n";
    vtkPrintMatrixFormat(cout, sparse_array);

    test_expression(sparse_array);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(0, 0)) == 1.0);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(1, 0)) == -0.5);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(2, 0)) == 0.0);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(0, 1)) == 0.5);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(1, 1)) == 1.0);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(2, 1)) == -0.5);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(0, 2)) == 0.0);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(1, 2)) == 0.5);
    test_expression(sparse_array->GetValue(vtkArrayCoordinates(2, 2)) == 1.0);

    source->SetArrayType(vtkDiagonalMatrixSource::DENSE);
    source->Update();

    vtkDenseArray<double>* const dense_array = vtkDenseArray<double>::SafeDownCast(source->GetOutput()->GetArray());

    cout << "dense diagonal matrix:\n";
    vtkPrintMatrixFormat(cout, dense_array);

    test_expression(dense_array);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(0, 0)) == 1.0);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(1, 0)) == -0.5);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(2, 0)) == 0.0);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(0, 1)) == 0.5);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(1, 1)) == 1.0);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(2, 1)) == -0.5);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(0, 2)) == 0.0);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(1, 2)) == 0.5);
    test_expression(dense_array->GetValue(vtkArrayCoordinates(2, 2)) == 1.0);

    return 0;
    }
  catch(vtkstd::exception& e)
    {
    cerr << e.what() << endl;
    return 1;
    }
}

