/*
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories 
// for implementing this test.

#include "vtkDoubleArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPCAStatistics.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTestUtilities.h"

#include "vtksys/SystemTools.hxx"

//=============================================================================
int TestPCAStatistics( int argc, char* argv[] )
{
  char* normScheme = vtkTestUtilities::GetArgOrEnvOrDefault(
    "-normalize-covariance", argc, argv, "VTK_NORMALIZE_COVARIANCE", "None" );
  int testStatus = 0;

  /* */
  double mingledData[] = 
    {
    46, 45,
    47, 49,
    46, 47,
    46, 46,
    47, 46,
    47, 49,
    49, 49,
    47, 45,
    50, 50,
    46, 46,
    51, 50,
    48, 48,
    52, 54,
    48, 47,
    52, 52,
    49, 49,
    53, 54,
    50, 50,
    53, 54,
    50, 52,
    53, 53,
    50, 51,
    54, 54,
    49, 49,
    52, 52,
    50, 51,
    52, 52,
    49, 47,
    48, 48,
    48, 50,
    46, 48,
    47, 47
    };
  int nVals = 32;

  const char m0Name[] = "M0";
  vtkDoubleArray* dataset1Arr = vtkDoubleArray::New();
  dataset1Arr->SetNumberOfComponents( 1 );
  dataset1Arr->SetName( m0Name );

  const char m1Name[] = "M1";
  vtkDoubleArray* dataset2Arr = vtkDoubleArray::New();
  dataset2Arr->SetNumberOfComponents( 1 );
  dataset2Arr->SetName( m1Name );

  const char m2Name[] = "M2";
  vtkDoubleArray* dataset3Arr = vtkDoubleArray::New();
  dataset3Arr->SetNumberOfComponents( 1 );
  dataset3Arr->SetName( m2Name );

  for ( int i = 0; i < nVals; ++ i )
    {
    int ti = i << 1;
    dataset1Arr->InsertNextValue( mingledData[ti] );
    dataset2Arr->InsertNextValue( mingledData[ti + 1] );
    dataset3Arr->InsertNextValue( i != 12 ? -1. : -1.001 );
    }

  vtkTable* datasetTable = vtkTable::New();
  datasetTable->AddColumn( dataset1Arr );
  dataset1Arr->Delete();
  datasetTable->AddColumn( dataset2Arr );
  dataset2Arr->Delete();
  datasetTable->AddColumn( dataset3Arr );
  dataset3Arr->Delete();

  vtkPCAStatistics* haruspex = vtkPCAStatistics::New();
  haruspex->SetInput( 0, datasetTable );
  haruspex->SetNormalizationSchemeByName( normScheme );
  haruspex->SetBasisSchemeByName( "FixedBasisEnergy" );
  haruspex->SetFixedBasisEnergy( 1. - 1e-8 );

  datasetTable->Delete();

  // -- Select Column Pairs of Interest ( Learn Mode ) -- 
  haruspex->SetColumnStatus( m0Name, 1 );
  haruspex->SetColumnStatus( m1Name, 1 );
  haruspex->RequestSelectedColumns();
  haruspex->ResetAllColumnStates();
  haruspex->SetColumnStatus( m0Name, 1 );
  haruspex->SetColumnStatus( m1Name, 1 );
  haruspex->SetColumnStatus( m2Name, 1 );
  haruspex->SetColumnStatus( m2Name, 0 );
  haruspex->SetColumnStatus( m2Name, 1 );
  haruspex->RequestSelectedColumns();
  haruspex->RequestSelectedColumns(); // Try a duplicate entry. This should have no effect.
  haruspex->SetColumnStatus( m0Name, 0 );
  haruspex->SetColumnStatus( m2Name, 0 );
  haruspex->SetColumnStatus( "Metric 3", 1 ); // An invalid name. This should result in a request for metric 1's self-correlation.
  // haruspex->RequestSelectedColumns(); will get called in RequestData()

  // -- Test Learn Mode -- 
  haruspex->SetLearn( true );
  haruspex->SetDerive( true );
  haruspex->SetAssess( false );

  haruspex->Update();
  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( haruspex->GetOutputDataObject( 1 ) );
  for ( unsigned int b = 0; b < outputMetaDS->GetNumberOfBlocks(); ++ b )
    {
    vtkTable* outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( b ) );

    if ( b == 0 )
      {
      cout << "Raw sums\n";
      }
    else
      {
      cout << "Request " << ( b - 1 ) << "\n";
      }

    outputMeta->Dump();
    }

  // -- Test Assess Mode -- 
  vtkMultiBlockDataSet* paramsTables = vtkMultiBlockDataSet::New();
  paramsTables->ShallowCopy( outputMetaDS );

  haruspex->SetInput( 1, paramsTables );
  paramsTables->Delete();
  haruspex->SetLearn( false );
  haruspex->SetDerive( false ); // Do not recalculate nor rederive a model
  haruspex->SetAssess( true );
  haruspex->Update();

  vtkTable* outputData = haruspex->GetOutput();
  outputData->Dump();

  haruspex->Delete();
  delete [] normScheme;

  return testStatus;
}
