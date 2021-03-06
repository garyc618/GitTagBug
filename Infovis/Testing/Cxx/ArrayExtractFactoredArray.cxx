/*=========================================================================

  Program:   Visualization Toolkit
  Module:    ArrayExtractFactoredArray.cxx
  
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
#include <vtkExtractFactoredArray.h>
#include <vtkFactoredArrayData.h>
#include <vtkSmartPointer.h>
#include <vtkSparseArray.h>

#include <vtksys/ios/iostream>
#include <vtksys/stl/stdexcept>

#define test_expression(expression) \
{ \
  if(!(expression)) \
    throw vtkstd::runtime_error("Expression failed: " #expression); \
}

int ArrayExtractFactoredArray(int argc, char* argv[])
{
  try
    {
    vtkSmartPointer<vtkSparseArray<double> > a = vtkSmartPointer<vtkSparseArray<double> >::New();
    vtkSmartPointer<vtkSparseArray<double> > b = vtkSmartPointer<vtkSparseArray<double> >::New();

    vtkSmartPointer<vtkFactoredArrayData> factored = vtkSmartPointer<vtkFactoredArrayData>::New();
    factored->AddArray(a);
    factored->AddArray(b);

    vtkSmartPointer<vtkExtractFactoredArray> extract = vtkSmartPointer<vtkExtractFactoredArray>::New();
    extract->AddInputConnection(factored->GetProducerPort());

    extract->SetIndex(0);
    extract->Update();
    test_expression(extract->GetOutput()->GetArray() == a.GetPointer());

    extract->SetIndex(1);
    extract->Update();
    test_expression(extract->GetOutput()->GetArray() == b.GetPointer());

    return 0;
    }
  catch(vtkstd::exception& e)
    {
    cout << e.what() << endl;
    return 1;
    }
}

