/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPExtractArraysOverTime.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPExtractArraysOverTime.h"

#include "vtkDataSetAttributes.h"
#include "vtkInformationVector.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkTable.h"
#include "vtkUnsignedCharArray.h"

vtkCxxRevisionMacro(vtkPExtractArraysOverTime, "1.8");
vtkStandardNewMacro(vtkPExtractArraysOverTime);

vtkCxxSetObjectMacro(vtkPExtractArraysOverTime, Controller, vtkMultiProcessController);

//----------------------------------------------------------------------------
vtkPExtractArraysOverTime::vtkPExtractArraysOverTime()
{
  this->Controller = 0;
  this->SetController(vtkMultiProcessController::GetGlobalController());
}

//----------------------------------------------------------------------------
vtkPExtractArraysOverTime::~vtkPExtractArraysOverTime()
{
  this->SetController(0);
}
//----------------------------------------------------------------------------
void vtkPExtractArraysOverTime::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Controller: " << this->Controller << endl;
}

//----------------------------------------------------------------------------
void vtkPExtractArraysOverTime::PostExecute(
  vtkInformation* request,
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkTable *output = vtkTable::GetData(outInfo);

  int procid = 0;
  int numProcs = 1;
  if ( this->Controller )
    {
    procid = this->Controller->GetLocalProcessId();
    numProcs = this->Controller->GetNumberOfProcesses();
    }

  if (numProcs > 1)
    {
    if (procid == 0)
      {
      for (int i = 1; i < numProcs; i++)
        {
        vtkTable* remoteOutput = vtkTable::New();
        this->Controller->Receive(remoteOutput, i, EXCHANGE_DATA);
        this->AddRemoteData(remoteOutput, output);
        remoteOutput->Delete();
        }

      // Zero out invalid time steps and report error if necessary.
      bool error = false;
      vtkUnsignedCharArray* validPts = vtkUnsignedCharArray::SafeDownCast(
        output->GetRowData()->GetArray("vtkValidPointMask"));
      if (validPts)
        {
        vtkIdType numRows = output->GetNumberOfRows();
        for (vtkIdType i=0; i<numRows; i++)
          {
          if (!validPts->GetValue(i))
            {
            error = true;
            vtkDataSetAttributes* outRowData = output->GetRowData();
            int numArrays = outRowData->GetNumberOfArrays();
            for (int aidx=0; aidx<numArrays; aidx++)
              {
              vtkDataArray* array = outRowData->GetArray(aidx);
              // If array is not null and it is not the time array
              if (array &&
                  (!array->GetName() ||
                   strncmp(array->GetName(), "Time", 4) != 0))
                {
                int numComps = array->GetNumberOfComponents();
                if (numComps > 0)
                  {
                  // This should also initialize to 0
                  double* val = new double[numComps];
                  array->SetTuple(i, val);
                  delete[] val;
                  }
                }
              }
            }
          }
        }
      if (error)
        {
        //this is not necessarily an error
        //when the probe misses invalid and zeroing is correct
        //vtkErrorMacro("One or more selected items could not be found. "
        //              "Array values for those items are set to 0");
        }
      }
    else
      {
      this->Controller->Send(output, 0, EXCHANGE_DATA);
      }
    }

  this->Superclass::PostExecute(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
void vtkPExtractArraysOverTime::AddRemoteData(vtkTable* routput,
                                              vtkTable* output)
{
  vtkIdType rDims = routput->GetNumberOfRows();
  vtkIdType dims = output->GetNumberOfRows();
  if (dims != rDims)
    {
    vtkWarningMacro("Tried to add remote dataset of different length. "
                    "Skipping");
    return;
    }
  
  vtkUnsignedCharArray* rValidPts = vtkUnsignedCharArray::SafeDownCast(
    routput->GetRowData()->GetArray("vtkValidPointMask"));

  // Copy the valid values
  if (rValidPts)
    {
    for (vtkIdType i=0; i<dims; i++)
      {
      if (rValidPts->GetValue(i))
        {
        vtkDataSetAttributes* outRowData = output->GetRowData();
        vtkDataSetAttributes* remoteRowData = routput->GetRowData();
        // Copy arrays from remote to current
        int numRArrays = remoteRowData->GetNumberOfArrays();
        for (int aidx=0; aidx<numRArrays; aidx++)
          {
          const char* name = 0;
          vtkAbstractArray* raa = remoteRowData->GetAbstractArray(aidx);
          if (raa)
            {
            name = raa->GetName();
            }
          if (name)
            {
            vtkAbstractArray* aa = outRowData->GetAbstractArray(name);
            // Create the output array if necessary
            if (!aa)
              {
              aa = raa->NewInstance();
              aa->DeepCopy(raa);
              aa->SetName(name);
              outRowData->AddArray(aa);
              aa->UnRegister(0);
              }
            if (raa->GetNumberOfTuples() > i)
              {
              aa->InsertTuple(i, i, raa);
              }
            else
              {
              //cerr << (raa->GetName()?raa->GetName():"NONAME") <<" has only " << raa->GetNumberOfTuples() << " and looking for value at " << i << endl;
              }
            }
          }
        }
      }
    }
}
