/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDataObjectGenerator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDataObjectGenerator - produces simple (composite or atomic) data 
// sets for testing.
// .SECTION Description
// vtkDataObjectGenerator parses a string and produces dataobjects from the
// dataobject template names it sees in the string. For example, if the string
// contains "ID1" the generator will create a vtkImageData. "UF1", "RG1", 
// "SG1", "PD1", and "UG1" will produce vtkUniformGrid, vtkRectilinearGrid,
// vtkStructuredGrid, vtkPolyData and vtkUnstructuredGrid respectively. You 
// can compose composite datasets from the atomic ones listed above
// by putting them between "(" and ")" in the string to create groups, and 
// then placing a collection of groups together inside one of the two 
// composite dataset identifiers - "MB{}" or "HB[]".
// "HB[ (UF1)(UF1)(UF1) ]" will create a vtkHierarchicalBoxDataSet representing
// an octree, in which the firstmost cell is refined, and then the firstmost 
// refined cell is refined itself.
// "MB{ ID1 PD1 MB{} }" will create a vtkMultiBlockDataSet consisting
// of three blocks: image data, poly data, multi-block (empty).

#ifndef __vtkDataObjectGenerator_h
#define __vtkDataObjectGenerator_h

#include "vtkDataObjectAlgorithm.h"

class vtkInternalStructureCache;

class VTK_GRAPHICS_EXPORT vtkDataObjectGenerator 
: public vtkDataObjectAlgorithm
{
 public:
  static vtkDataObjectGenerator *New();
  vtkTypeRevisionMacro(vtkDataObjectGenerator,vtkDataObjectAlgorithm);
  void PrintSelf(ostream &os, vtkIndent indent);
  
  // Description:
  // The string that will be parsed to specify a dataobject structure.
  vtkSetStringMacro(Program);
  vtkGetStringMacro(Program);

protected:
  vtkDataObjectGenerator();
  ~vtkDataObjectGenerator();

  virtual int RequestData(vtkInformation *req,
                  vtkInformationVector **inV,
                  vtkInformationVector *outV);
  virtual int RequestDataObject(vtkInformation *req,
                  vtkInformationVector **inV,
                  vtkInformationVector *outV);
  virtual int RequestInformation(vtkInformation *req,
                  vtkInformationVector **inV,
                  vtkInformationVector *outV);
  virtual int RequestUpdateExtent(vtkInformation *req,
                  vtkInformationVector **inV,
                  vtkInformationVector *outV);

  //the string to parse to create a structure
  char *Program;
  //a record of the structure
  vtkInternalStructureCache *Structure;

  //Helper for RequestDataObject
  vtkDataObject * 
    CreateOutputDataObjects(vtkInternalStructureCache *structure);
  //Helper for RequestData
  vtkDataObject * 
    FillOutputDataObjects(vtkInternalStructureCache *structure,
                          int level,
                          int stripe=0);

  //to determine which composite data stripe to fill in
  vtkIdType Rank;
  vtkIdType Processors;

  //create the templated atomic data sets
  void MakeImageData1(vtkDataSet *ds);
  void MakeUniformGrid1(vtkDataSet *ds);
  void MakeRectilinearGrid1(vtkDataSet *ds);
  void MakeStructuredGrid1(vtkDataSet *ds);
  void MakePolyData1(vtkDataSet *ds);
  void MakeUnstructuredGrid1(vtkDataSet *ds);

  //used to spatially separate sub data sets within composites
  double XOffset; //increases for each dataset index
  double YOffset; //increases for each sub data set
  double ZOffset; //increases for each group index

  //used to filling in point and cell values with unique Ids
  vtkIdType CellIdCounter;
  vtkIdType PointIdCounter;

  //assign point and cell values to each point and cell
  void MakeValues(vtkDataSet *ds);

private:
  vtkDataObjectGenerator(const vtkDataObjectGenerator&);  // Not implemented.
  void operator=(const vtkDataObjectGenerator&);  // Not implemented.
};

#endif
