/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGraph.cxx

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

#include "vtkGraph.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkCellArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkDistributedGraphHelper.h"
#include "vtkEdgeListIterator.h"
#include "vtkGraphEdge.h"
#include "vtkGraphInternals.h"
#include "vtkIdTypeArray.h"
#include "vtkInEdgeIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutEdgeIterator.h"
#include "vtkPoints.h"
#include "vtkVertexListIterator.h"
#include "vtkVariantArray.h"
#include "vtkStringArray.h"

#include <assert.h>
#include <vtksys/stl/vector>

double vtkGraph::DefaultPoint[3] = {0, 0, 0};

//----------------------------------------------------------------------------
// private class vtkGraphEdgePoints
//----------------------------------------------------------------------------
class vtkGraphEdgePoints : public vtkObject
{
public:
  static vtkGraphEdgePoints *New();
  vtkTypeRevisionMacro(vtkGraphEdgePoints, vtkObject);
  vtksys_stl::vector< vtksys_stl::vector<double> > Storage;

protected:
  vtkGraphEdgePoints() { }
  ~vtkGraphEdgePoints() { }

private:
  vtkGraphEdgePoints(const vtkGraphEdgePoints&);  // Not implemented.
  void operator=(const vtkGraphEdgePoints&);  // Not implemented.
};
vtkStandardNewMacro(vtkGraphEdgePoints);
vtkCxxRevisionMacro(vtkGraphEdgePoints, "1.33");

//----------------------------------------------------------------------------
// class vtkGraph
//----------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkGraph, Points, vtkPoints);
vtkCxxSetObjectMacro(vtkGraph, Internals, vtkGraphInternals);
vtkCxxSetObjectMacro(vtkGraph, EdgePoints, vtkGraphEdgePoints);
vtkCxxSetObjectMacro(vtkGraph, EdgeList, vtkIdTypeArray);
vtkCxxRevisionMacro(vtkGraph, "1.33");
//----------------------------------------------------------------------------
vtkGraph::vtkGraph()
{
  this->VertexData = vtkDataSetAttributes::New();
  this->EdgeData = vtkDataSetAttributes::New();
  this->Points = 0;
  vtkMath::UninitializeBounds(this->Bounds);

  this->Information->Set(vtkDataObject::DATA_EXTENT_TYPE(), VTK_PIECES_EXTENT);
  this->Information->Set(vtkDataObject::DATA_PIECE_NUMBER(), -1);
  this->Information->Set(vtkDataObject::DATA_NUMBER_OF_PIECES(), 1);
  this->Information->Set(vtkDataObject::DATA_NUMBER_OF_GHOST_LEVELS(), 0);

  this->Internals = vtkGraphInternals::New();
  this->DistributedHelper = 0;
  this->EdgePoints = 0;
  this->EdgeList = 0;
}

//----------------------------------------------------------------------------
vtkGraph::~vtkGraph()
{
  this->VertexData->Delete();
  this->EdgeData->Delete();
  if (this->Points)
    {
    this->Points->Delete();
    }
  this->Internals->Delete();
  if (this->DistributedHelper)
    {
    this->DistributedHelper->Delete();
    }
  if (this->EdgeList)
    {
    this->EdgeList->Delete();
    }
  if (this->EdgePoints)
    {
    this->EdgePoints->Delete();
    }
}

//----------------------------------------------------------------------------
double *vtkGraph::GetPoint(vtkIdType ptId)
{
  if (this->Points)
    {
    return this->Points->GetPoint(ptId);
    }
  return this->DefaultPoint;
}

//----------------------------------------------------------------------------
void vtkGraph::GetPoint(vtkIdType ptId, double x[3])
{
  if (this->Points)
    {
    vtkIdType index = ptId;
    if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
      {
        int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
        if (myRank != helper->GetVertexOwner(ptId))
          {
          vtkErrorMacro("vtkGraph cannot retrieve a point for a non-local vertex");
          return;
          }

        index = helper->GetVertexIndex(ptId);
      }

    this->Points->GetPoint(index, x);
    }
  else
    {
    for (int i = 0; i < 3; i++)
      {
      x[i] = this->DefaultPoint[i];
      }
    }
}

//----------------------------------------------------------------------------
vtkPoints *vtkGraph::GetPoints()
{
  if (!this->Points)
    {
    this->Points = vtkPoints::New();
    }
  if (this->Points->GetNumberOfPoints() != this->GetNumberOfVertices())
    {
    this->Points->SetNumberOfPoints(this->GetNumberOfVertices());
    for (vtkIdType i = 0; i < this->GetNumberOfVertices(); i++)
      {
      this->Points->SetPoint(i, 0, 0, 0);
      }
    }
  return this->Points;
}

//----------------------------------------------------------------------------
void vtkGraph::ComputeBounds()
{
  double *bounds;

  if ( this->Points )
    {
    bounds = this->Points->GetBounds();
    for (int i=0; i<6; i++)
      {
      this->Bounds[i] = bounds[i];
      }
    // TODO: how to compute the bounds for a distributed graph?
    this->ComputeTime.Modified();
    }
}

//----------------------------------------------------------------------------
double *vtkGraph::GetBounds()
{
  this->ComputeBounds();
  return this->Bounds;
}

//----------------------------------------------------------------------------
void vtkGraph::GetBounds(double bounds[6])
{
  this->ComputeBounds();
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

//----------------------------------------------------------------------------
unsigned long int vtkGraph::GetMTime()
{
  unsigned long int doTime = vtkDataObject::GetMTime();

  if ( this->VertexData->GetMTime() > doTime )
    {
    doTime = this->VertexData->GetMTime();
    }
  if ( this->EdgeData->GetMTime() > doTime )
    {
    doTime = this->EdgeData->GetMTime();
    }
  if ( this->Points ) 
    {
    if ( this->Points->GetMTime() > doTime )
      {
      doTime = this->Points->GetMTime();
      }
    }

  return doTime;
}

//----------------------------------------------------------------------------
void vtkGraph::Initialize()
{
  this->ForceOwnership();
  Superclass::Initialize();
  this->EdgeData->Initialize();
  this->VertexData->Initialize();
  this->Internals->NumberOfEdges = 0;
  this->Internals->Adjacency.clear();
  if (this->EdgePoints)
    {
    this->EdgePoints->Storage.clear();
    }
}

//----------------------------------------------------------------------------
void vtkGraph::GetOutEdges(vtkIdType v, vtkOutEdgeIterator *it)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the out edges for non-local vertex " << v);
      int* p = 0;
      *p = 17;
      return;
      }
    }

  if (it)
    {
    it->Initialize(this, v);
    }
}

//----------------------------------------------------------------------------
vtkOutEdgeType vtkGraph::GetOutEdge(vtkIdType v, vtkIdType i)
{
  vtkIdType index = v;
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the out edges for non-local vertex " << v);
      int* p = 0;
      *p = 17;
      return vtkOutEdgeType();
      }
    index = helper->GetVertexIndex(v);
    }

  if (i < this->GetOutDegree(v))
    {
    return this->Internals->Adjacency[index].OutEdges[i];
    }
  vtkErrorMacro("Out edge index out of bounds");
  return vtkOutEdgeType();
}

//----------------------------------------------------------------------------
void vtkGraph::GetOutEdge(vtkIdType v, vtkIdType i, vtkGraphEdge* e)
{
  vtkOutEdgeType oe = this->GetOutEdge(v, i);
  e->SetId(oe.Id);
  e->SetSource(v);
  e->SetTarget(oe.Target);
}

//----------------------------------------------------------------------------
void vtkGraph::GetOutEdges(vtkIdType v, const vtkOutEdgeType *& edges, vtkIdType & nedges)
{
  vtkIdType index = v;
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the out edges for non-local vertex " << v);
      int* p = 0;
      *p = 17;
      return;
      }

    index = helper->GetVertexIndex(v);
    }

  nedges = this->Internals->Adjacency[index].OutEdges.size();
  if (nedges > 0)
    {
    edges = &(this->Internals->Adjacency[index].OutEdges[0]);
    }
  else
    {
    edges = 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetOutDegree(vtkIdType v)
{
  vtkIdType index = v;
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot determine the out degree for a non-local vertex");
      return 0;
      }

    index = helper->GetVertexIndex(v);
    }
  return this->Internals->Adjacency[index].OutEdges.size();
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetDegree(vtkIdType v)
{
  vtkIdType index = v;
 
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot determine the degree for a non-local vertex");
      return 0;
      }

    index = helper->GetVertexIndex(v);
    }

  return this->Internals->Adjacency[index].InEdges.size() +
         this->Internals->Adjacency[index].OutEdges.size();
}

//----------------------------------------------------------------------------
void vtkGraph::GetInEdges(vtkIdType v, vtkInEdgeIterator *it)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the in edges for a non-local vertex");
      return;
      }
    }

  if (it)
    {
    it->Initialize(this, v);
    }
}

//----------------------------------------------------------------------------
vtkInEdgeType vtkGraph::GetInEdge(vtkIdType v, vtkIdType i)
{
  vtkIdType index = v;
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the in edges for a non-local vertex");
      return vtkInEdgeType();
      }
    index = helper->GetVertexIndex(v);
    }

  if (i < this->GetInDegree(v))
    {
    return this->Internals->Adjacency[index].InEdges[i];
    }
  vtkErrorMacro("In edge index out of bounds");
  return vtkInEdgeType();
}

//----------------------------------------------------------------------------
void vtkGraph::GetInEdge(vtkIdType v, vtkIdType i, vtkGraphEdge* e)
{
  vtkInEdgeType ie = this->GetInEdge(v, i);
  e->SetId(ie.Id);
  e->SetSource(ie.Source);
  e->SetTarget(v);
}

//----------------------------------------------------------------------------
void vtkGraph::GetInEdges(vtkIdType v, const vtkInEdgeType *& edges, vtkIdType & nedges)
{
  vtkIdType index = v;

  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the in edges for a non-local vertex");
      return;
      }

    index = helper->GetVertexIndex(v);
    }

  nedges = this->Internals->Adjacency[index].InEdges.size();
  if (nedges > 0)
    {
    edges = &(this->Internals->Adjacency[index].InEdges[0]);
    }
  else
    {
    edges = 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetInDegree(vtkIdType v)
{
  vtkIdType index = v;

  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot determine the in degree for a non-local vertex");
      return 0;
      }

    index = helper->GetVertexIndex(v);
    }

  return this->Internals->Adjacency[index].InEdges.size();
}

//----------------------------------------------------------------------------
void vtkGraph::GetAdjacentVertices(vtkIdType v, vtkAdjacentVertexIterator *it)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot retrieve the adjacent vertices for a non-local vertex");
      return;
      }
    }

  if (it)
    {
    it->Initialize(this, v);
    }
}

//----------------------------------------------------------------------------
void vtkGraph::GetEdges(vtkEdgeListIterator *it)
{
  if (it)
    {
    it->SetGraph(this);
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetNumberOfEdges()
{
  return this->Internals->NumberOfEdges;
}

//----------------------------------------------------------------------------
void vtkGraph::GetVertices(vtkVertexListIterator *it)
{
  if (it)
    {
    it->SetGraph(this);
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetNumberOfVertices()
{
  return this->Internals->Adjacency.size();
}

//----------------------------------------------------------------------------
void vtkGraph::SetDistributedGraphHelper(vtkDistributedGraphHelper *helper)
{
  if (this->DistributedHelper)
    this->DistributedHelper->AttachToGraph(0);

  this->DistributedHelper = helper;
  if (this->DistributedHelper)
    {
    this->DistributedHelper->Register(this);
    this->DistributedHelper->AttachToGraph(this);
    }
}

//----------------------------------------------------------------------------
vtkDistributedGraphHelper *vtkGraph::GetDistributedGraphHelper()
{
  return this->DistributedHelper;
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::FindVertex(const vtkVariant& pedigreeId)
{
  vtkAbstractArray *pedigrees = this->GetVertexData()->GetPedigreeIds();
  if (pedigrees == NULL)
    {
    return -1;
    }

  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    vtkIdType myRank 
      = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (helper->GetVertexOwnerByPedigreeId(pedigreeId) 
          != myRank)
      {
      // The vertex is remote; ask the distributed graph helper to find it.
      return helper->FindVertex(pedigreeId);
      }

    vtkIdType result = pedigrees->LookupValue(pedigreeId);
    if (result == -1) 
      {
      return -1;
      }
    return helper->MakeDistributedId(myRank, result);
    }

  return pedigrees->LookupValue(pedigreeId);
}

//----------------------------------------------------------------------------
bool vtkGraph::CheckedShallowCopy(vtkGraph *g)
{
  if (!g)
    {
    return false;
    }
  bool valid = this->IsStructureValid(g);
  if (valid)
    {
    this->CopyInternal(g, false);
    }
  return valid;
}

//----------------------------------------------------------------------------
bool vtkGraph::CheckedDeepCopy(vtkGraph *g)
{
  if (!g)
    {
    return false;
    }
  bool valid = this->IsStructureValid(g);
  if (valid)
    {
    this->CopyInternal(g, true);
    }
  return valid;
}

//----------------------------------------------------------------------------
void vtkGraph::ShallowCopy(vtkDataObject *obj)
{
  vtkGraph *g = vtkGraph::SafeDownCast(obj);
  if (!g)
    {
    vtkErrorMacro("Can only shallow copy from vtkGraph subclass.");
    return;
    }
  bool valid = this->IsStructureValid(g);
  if (valid)
    {
    this->CopyInternal(g, false);
    }
  else
    {
    vtkErrorMacro("Invalid graph structure for this type of graph.");
    }
}

//----------------------------------------------------------------------------
void vtkGraph::DeepCopy(vtkDataObject *obj)
{
  vtkGraph *g = vtkGraph::SafeDownCast(obj);
  if (!g)
    {
    vtkErrorMacro("Can only shallow copy from vtkGraph subclass.");
    return;
    }
  bool valid = this->IsStructureValid(g);
  if (valid)
    {
    this->CopyInternal(g, true);
    }
  else
    {
    vtkErrorMacro("Invalid graph structure for this type of graph.");
    }
}

//----------------------------------------------------------------------------
void vtkGraph::CopyStructure(vtkGraph *g)
{
  // Copy on write.
  this->SetInternals(g->Internals);
  if (g->Points)
    {
    if (!this->Points)
      {
      this->Points = vtkPoints::New();
      }
    this->Points->ShallowCopy(g->Points);
    }
  else if (this->Points)
    {
    this->Points->Delete();
    this->Points = 0;
    }

  // Propagate information used by distributed graphs
  this->Information->Set
    (vtkDataObject::DATA_PIECE_NUMBER(),
     g->Information->Get(vtkDataObject::DATA_PIECE_NUMBER()));
  this->Information->Set
    (vtkDataObject::DATA_NUMBER_OF_PIECES(),
     g->Information->Get(vtkDataObject::DATA_NUMBER_OF_PIECES()));
}

//----------------------------------------------------------------------------
void vtkGraph::CopyInternal(vtkGraph *g, bool deep)
{
  if (deep)
    {
    vtkDataObject::DeepCopy(g);
    }
  else
    {
    vtkDataObject::ShallowCopy(g);
    }
  if (g->DistributedHelper)
    {
    if (!this->DistributedHelper)
      {
      this->SetDistributedGraphHelper(g->DistributedHelper->Clone());
      }
    }
  else if (this->DistributedHelper)
    {
    this->SetDistributedGraphHelper(0);
    }

  // Copy on write.
  this->SetInternals(g->Internals);

  if (deep)
    {
    this->EdgeData->DeepCopy(g->EdgeData);
    this->VertexData->DeepCopy(g->VertexData);
    this->DeepCopyEdgePoints(g);
    }
  else
    {
    this->EdgeData->ShallowCopy(g->EdgeData);
    this->VertexData->ShallowCopy(g->VertexData);
    this->ShallowCopyEdgePoints(g);
    }

  // Copy points
  if (g->Points && deep)
    {
    if (!this->Points)
      {
      this->Points = vtkPoints::New();
      }
    this->Points->DeepCopy(g->Points);
    }
  else
    {
    this->SetPoints(g->Points);
    }

  // Copy edge list
  if (g->EdgeList && deep)
    {
    if (!this->EdgeList)
      {
      this->EdgeList = vtkIdTypeArray::New();
      }
    this->EdgeList->DeepCopy(g->EdgeList);
    }
  else
    {
    this->SetEdgeList(g->EdgeList);
    }

  // Propagate information used by distributed graphs
  this->Information->Set
    (vtkDataObject::DATA_PIECE_NUMBER(),
     g->Information->Get(vtkDataObject::DATA_PIECE_NUMBER()));
  this->Information->Set
    (vtkDataObject::DATA_NUMBER_OF_PIECES(),
     g->Information->Get(vtkDataObject::DATA_NUMBER_OF_PIECES()));
}

//----------------------------------------------------------------------------
void vtkGraph::Squeeze()
{
  if ( this->Points )
    {
    this->Points->Squeeze();
    }
  this->EdgeData->Squeeze();
  this->VertexData->Squeeze();
}

//----------------------------------------------------------------------------
vtkGraph *vtkGraph::GetData(vtkInformation *info)
{
  return info? vtkGraph::SafeDownCast(info->Get(DATA_OBJECT())) : 0;
}

//----------------------------------------------------------------------------
vtkGraph *vtkGraph::GetData(vtkInformationVector *v, int i)
{
  return vtkGraph::GetData(v->GetInformationObject(i));
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetSourceVertex(vtkIdType e)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
        if (e != this->Internals->LastRemoteEdgeId)
          {
          helper->FindEdgeSourceAndTarget
            (e, 
             &this->Internals->LastRemoteEdgeSource,
             &this->Internals->LastRemoteEdgeTarget);
          }

        return this->Internals->LastRemoteEdgeSource;
      }
    
    e = helper->GetEdgeIndex(e);
    }  

  if (e < 0 || e >= this->GetNumberOfEdges())
    {
    vtkErrorMacro("Edge index out of range.");
    return -1;
    }
  if (!this->EdgeList)
    {
    this->BuildEdgeList();
    }
  return this->EdgeList->GetValue(2*e);
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetTargetVertex(vtkIdType e)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
        if (e != this->Internals->LastRemoteEdgeId)
          {
          this->Internals->LastRemoteEdgeId = e;
          helper->FindEdgeSourceAndTarget
            (e, 
             &this->Internals->LastRemoteEdgeSource,
             &this->Internals->LastRemoteEdgeTarget);
          }

        return this->Internals->LastRemoteEdgeTarget;
      }
    
    e = helper->GetEdgeIndex(e);
    }  

  if (e < 0 || e >= this->GetNumberOfEdges())
    {
    vtkErrorMacro("Edge index out of range.");
    return -1;
    }
  if (!this->EdgeList)
    {
    this->BuildEdgeList();
    }
  return this->EdgeList->GetValue(2*e  + 1);
}

//----------------------------------------------------------------------------
void vtkGraph::BuildEdgeList()
{
  if (this->EdgeList)
    {
    this->EdgeList->SetNumberOfTuples(this->GetNumberOfEdges());
    }
  else
    {
    this->EdgeList = vtkIdTypeArray::New();
    this->EdgeList->SetNumberOfComponents(2);
    this->EdgeList->SetNumberOfTuples(this->GetNumberOfEdges());
    }
  vtkEdgeListIterator* it = vtkEdgeListIterator::New();
  this->GetEdges(it);
  while (it->HasNext())
    {
    vtkEdgeType e = it->Next();
    this->EdgeList->SetValue(2*e.Id, e.Source);
    this->EdgeList->SetValue(2*e.Id + 1, e.Target);
    }
  it->Delete();
}

//----------------------------------------------------------------------------
void vtkGraph::SetEdgePoints(vtkIdType e, vtkIdType npts, double* pts)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot set edge points for a non-local vertex");
      return;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return;
    }
  if (!this->EdgePoints)
    {
    this->EdgePoints = vtkGraphEdgePoints::New();
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  this->EdgePoints->Storage[e].clear();
  for (vtkIdType i = 0; i < 3*npts; ++i, ++pts)
    {
    this->EdgePoints->Storage[e].push_back(*pts);
    }
}

//----------------------------------------------------------------------------
void vtkGraph::GetEdgePoints(vtkIdType e, vtkIdType& npts, double*& pts)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot retrieve edge points for a non-local vertex");
      return;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return;
    }
  if (!this->EdgePoints)
    {
    npts = 0;
    pts = 0;
    return;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  npts = this->EdgePoints->Storage[e].size() / 3;
  if (npts > 0)
    {
    pts = &this->EdgePoints->Storage[e][0];
    }
  else
    {
    pts = 0;
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkGraph::GetNumberOfEdgePoints(vtkIdType e)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot retrieve edge points for a non-local vertex");
      return 0;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return 0;
    }
  if (!this->EdgePoints)
    {
    return 0;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  return this->EdgePoints->Storage[e].size() / 3;
}

//----------------------------------------------------------------------------
double* vtkGraph::GetEdgePoint(vtkIdType e, vtkIdType i)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot receive edge points for a non-local vertex");
      return 0;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return 0;
    }
  if (!this->EdgePoints)
    {
    vtkErrorMacro("No edge points defined.");
    return 0;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  vtkIdType npts = this->EdgePoints->Storage[e].size() / 3;
  if (i >= npts)
    {
    vtkErrorMacro("Edge point index out of range.");
    return 0;
    }
  return &this->EdgePoints->Storage[e][3*i];
}

//----------------------------------------------------------------------------
void vtkGraph::SetEdgePoint(vtkIdType e, vtkIdType i, double x[3])
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot set edge points for a non-local vertex");
      return;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return;
    }
  if (!this->EdgePoints)
    {
    vtkErrorMacro("No edge points defined.");
    return;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  vtkIdType npts = this->EdgePoints->Storage[e].size() / 3;
  if (i >= npts)
    {
    vtkErrorMacro("Edge point index out of range.");
    return;
    }
  for (int c = 0; c < 3; ++c)
    {
    this->EdgePoints->Storage[e][3*i + c] = x[c];
    }
}

//----------------------------------------------------------------------------
void vtkGraph::ClearEdgePoints(vtkIdType e)
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot clear edge points for a non-local vertex");
      return;
      }
    
    e = helper->GetEdgeIndex(e);
    }

  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return;
    }
  if (!this->EdgePoints)
    {
    vtkErrorMacro("No edge points defined.");
    return;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  this->EdgePoints->Storage[e].clear();
}

//----------------------------------------------------------------------------
void vtkGraph::AddEdgePoint(vtkIdType e, double x[3])
{
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetEdgeOwner(e))
      {
      vtkErrorMacro("vtkGraph cannot set edge points for a non-local vertex");
      return;
      }
    
    e = helper->GetEdgeIndex(e);
    }
  if (e < 0 || e > this->Internals->NumberOfEdges)
    {
    vtkErrorMacro("Invalid edge id.");
    return;
    }
  if (!this->EdgePoints)
    {
    vtkErrorMacro("No edge points defined.");
    return;
    }
  vtksys_stl::vector< vtksys_stl::vector<double> >::size_type numEdges = this->Internals->NumberOfEdges;
  if (this->EdgePoints->Storage.size() < numEdges)
    {
    this->EdgePoints->Storage.resize(numEdges);
    }
  for (int c = 0; c < 3; ++c)
    {
    this->EdgePoints->Storage[e].push_back(x[c]);
    }
}

//----------------------------------------------------------------------------
void vtkGraph::ShallowCopyEdgePoints(vtkGraph* g)
{
  this->SetEdgePoints(g->EdgePoints);
}

//----------------------------------------------------------------------------
void vtkGraph::DeepCopyEdgePoints(vtkGraph* g)
{
  if (g->EdgePoints)
    {
    if (!this->EdgePoints)
      {
      this->EdgePoints = vtkGraphEdgePoints::New();
      }
    this->EdgePoints->Storage = g->EdgePoints->Storage;
    }
  else
    {
    this->SetEdgePoints(0);
    }
}
//----------------------------------------------------------------------------
void vtkGraph::AddVertexInternal(vtkVariantArray *propertyArr,
                                 vtkIdType *vertex)
{
  this->ForceOwnership();
  
  vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper();

  if (propertyArr)      // Add/replace vertex properties if passed in
    {
    vtkAbstractArray *peds = this->GetVertexData()->GetPedigreeIds();
    // If the properties include pedigreeIds, we need to see if this
    // pedigree already exists and, if so, simply update its properties.
    if (peds)
      {
      // Get the array index associated with pedIds.
      vtkIdType pedIdx = this->GetVertexData()->SetPedigreeIds(peds);
      vtkVariant pedigreeId = propertyArr->GetValue(pedIdx);
      if (helper)
        {
        vtkIdType myRank 
          = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
        if (helper->GetVertexOwnerByPedigreeId(pedigreeId) != myRank) 
          {
          helper->AddVertexInternal(propertyArr, vertex);
          return;
          }
        }

      vtkIdType existingVertex = this->FindVertex(pedigreeId);
      if (existingVertex != -1 && existingVertex < this->GetNumberOfVertices())
        {
        vtkIdType idx = existingVertex;
        if (helper)
          {
          idx = helper->GetVertexIndex(existingVertex);
          }
        for (int iprop=0; iprop<propertyArr->GetNumberOfValues(); iprop++)
          {
          vtkAbstractArray* arr = this->GetVertexData()->GetAbstractArray(iprop);
          arr->InsertVariantValue(idx, propertyArr->GetValue(iprop));
          }
        if (vertex)
          {
          *vertex = existingVertex;
          }
        return ;
        }
      
      this->Internals->Adjacency.push_back(vtkVertexAdjacencyList());  // Add a new (local) vertex
      vtkIdType index = this->Internals->Adjacency.size() - 1;   
      
      vtkDataSetAttributes *vertexData = this->GetVertexData();
      int numProps = propertyArr->GetNumberOfValues();
      assert(numProps == vertexData->GetNumberOfArrays());
      for (int iprop=0; iprop<numProps; iprop++)
        {
        vtkAbstractArray* arr = vertexData->GetAbstractArray(iprop);
        arr->InsertVariantValue(index, propertyArr->GetValue(iprop));
        }
      }  // end if (peds)
    //----------------------------------------------------------------
    else   // We have propArr, but not pedIds - just add the propArr
      {
      this->Internals->Adjacency.push_back(vtkVertexAdjacencyList()); 
      vtkIdType index = this->Internals->Adjacency.size() - 1;   
      
      vtkDataSetAttributes *vertexData = this->GetVertexData();
      int numProps = propertyArr->GetNumberOfValues();
      assert(numProps == vertexData->GetNumberOfArrays());
      for (int iprop=0; iprop<numProps; iprop++)
        {
        vtkAbstractArray* arr = vertexData->GetAbstractArray(iprop);
        arr->InsertVariantValue(index, propertyArr->GetValue(iprop));
        }
      }
    }
  else  // No properties, just add a new vertex
    {
    this->Internals->Adjacency.push_back(vtkVertexAdjacencyList()); 
    }
  
  if (vertex)
    {
    if (helper)
      {
      *vertex = helper->MakeDistributedId
                  (this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER()),
                   this->Internals->Adjacency.size() - 1);
      }
    else
      {
      *vertex = this->Internals->Adjacency.size() - 1;
      }
    }
}
//----------------------------------------------------------------------------
void vtkGraph::AddVertexInternal(const vtkVariant& pedigreeId, 
                                 vtkIdType *vertex)
{
  // Add vertex V, given a pedId:
  //   1) if a dist'd G and this proc doesn't own V, add it (via helper class) and RETURN.
  //   2) if V already exists for this pedId, RETURN it.
  //   3) add V locally and insert its pedId
  vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper();
  if (helper)
    {
    vtkIdType myRank 
      = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (helper->GetVertexOwnerByPedigreeId(pedigreeId) != myRank) 
      {
      helper->AddVertexInternal(pedigreeId, vertex);
      return;
      }
    }

  vtkIdType existingVertex = this->FindVertex(pedigreeId);
  if (existingVertex != -1 && existingVertex < this->GetNumberOfVertices())
    {
    // We found this vertex; nothing more to do.
    if (vertex)
      {
      *vertex = existingVertex;
      }
    return ;
    }

  // Add the vertex locally
  this->ForceOwnership();
  vtkIdType v;
  this->AddVertexInternal(0, &v);
  if (vertex)
    {
    *vertex = v;
    }

  // Add the pedigree ID of the vertex
  vtkAbstractArray *pedigrees = this->GetVertexData()->GetPedigreeIds();
  if (pedigrees == NULL)
    {
    vtkErrorMacro("Added a vertex with a pedigree ID to a vtkGraph with no pedigree ID array");
    return;
    }

  vtkIdType index = v;
  if (helper)
    {
    index = helper->GetVertexIndex(v);
    }

  pedigrees->InsertVariantValue(index, pedigreeId);
}
//----------------------------------------------------------------------------
void vtkGraph::AddEdgeInternal(vtkIdType u, vtkIdType v, bool directed, 
                               vtkVariantArray *propertyArr, vtkEdgeType *edge)
{
  this->ForceOwnership();
  if (this->DistributedHelper)
    {
    this->DistributedHelper->AddEdgeInternal(u, v, directed, 
                                             propertyArr, edge);
    return;
    }

  if (u >= this->GetNumberOfVertices() || v >= this->GetNumberOfVertices())
    {
    vtkErrorMacro(<< "Vertex index out of range");
    return;
    }

  vtkIdType edgeId = this->Internals->NumberOfEdges;
  vtkIdType edgeIndex = edgeId;
  this->Internals->NumberOfEdges++;
  this->Internals->Adjacency[u].OutEdges.push_back(vtkOutEdgeType(v, edgeId));
  if (directed)
    {
    this->Internals->Adjacency[v].InEdges.push_back(vtkInEdgeType(u, edgeId));
    }
  else if (u != v)
    {
    // Avoid storing self-loops twice in undirected graphs.
    this->Internals->Adjacency[v].OutEdges.push_back(vtkOutEdgeType(u, edgeId));
    }

  if (this->EdgeList)
    {
    this->EdgeList->InsertNextValue(u);
    this->EdgeList->InsertNextValue(v);
    }

  if (edge)
    {
    *edge = vtkEdgeType(u, v, edgeId);
    }

  if (propertyArr)
    {
    // Insert edge properties
    vtkDataSetAttributes *edgeData = this->GetEdgeData();
    int numProps = propertyArr->GetNumberOfValues();
    assert(numProps == edgeData->GetNumberOfArrays());
    for (int iprop=0; iprop<numProps; iprop++)
      {
      vtkAbstractArray* array = edgeData->GetAbstractArray(iprop);
      array->InsertVariantValue(edgeIndex, propertyArr->GetValue(iprop));
      }
    }
}

//----------------------------------------------------------------------------
void vtkGraph::AddEdgeInternal(const vtkVariant& uPedigreeId, vtkIdType v, 
                               bool directed, vtkVariantArray *propertyArr, 
                               vtkEdgeType *edge)
{
  this->ForceOwnership();
  if (this->DistributedHelper)
    {
    this->DistributedHelper->AddEdgeInternal(uPedigreeId, v, 
                                                        directed, propertyArr,
                                                        edge);
    return;
    }
  
  vtkIdType u;
  this->AddVertexInternal(uPedigreeId, &u);
  this->AddEdgeInternal(u, v, directed, propertyArr, edge);
}

//----------------------------------------------------------------------------
void vtkGraph::AddEdgeInternal(vtkIdType u, const vtkVariant& vPedigreeId, 
                               bool directed, vtkVariantArray *propertyArr, 
                               vtkEdgeType *edge)
{
  this->ForceOwnership();
  if (this->DistributedHelper)
    {
    this->DistributedHelper->AddEdgeInternal(u, vPedigreeId, 
                                                        directed, propertyArr,
                                                        edge);
    return;
    }
  
  vtkIdType v;
  this->AddVertexInternal(vPedigreeId, &v);
  this->AddEdgeInternal(u, v, directed, propertyArr, edge);
}

//----------------------------------------------------------------------------
void vtkGraph::AddEdgeInternal(const vtkVariant& uPedigreeId, 
                               const vtkVariant& vPedigreeId, 
                               bool directed, vtkVariantArray *propertyArr, 
                               vtkEdgeType *edge)
{
  this->ForceOwnership();
  if (this->DistributedHelper)
    {
    this->DistributedHelper->AddEdgeInternal(uPedigreeId, 
                                                        vPedigreeId, directed,
                                                        propertyArr, edge);
    return;
    }
  
  vtkIdType u, v;
  this->AddVertexInternal(uPedigreeId, &u);
  this->AddVertexInternal(vPedigreeId, &v);
  this->AddEdgeInternal(u, v, directed, propertyArr, edge);
}

//----------------------------------------------------------------------------
void vtkGraph::ReorderOutVertices(vtkIdType v, vtkIdTypeArray *vertices)
{
  vtkIdType index = v;
  if (vtkDistributedGraphHelper *helper = this->GetDistributedGraphHelper())
    {
    int myRank = this->Information->Get(vtkDataObject::DATA_PIECE_NUMBER());
    if (myRank != helper->GetVertexOwner(v))
      {
      vtkErrorMacro("vtkGraph cannot reorder the out vertices for a non-local vertex");
      return;
      }

    index = helper->GetVertexIndex(v);
    }

  this->ForceOwnership();
  vtksys_stl::vector<vtkOutEdgeType> outEdges;
  vtksys_stl::vector<vtkOutEdgeType>::iterator it, itEnd;
  itEnd = this->Internals->Adjacency[index].OutEdges.end();
  for (vtkIdType i = 0; i < vertices->GetNumberOfTuples(); ++i)
    {
    vtkIdType vert = vertices->GetValue(i);
    // Find the matching edge
    for (it = this->Internals->Adjacency[index].OutEdges.begin(); it != itEnd; ++it)
      {
      if (it->Target == vert)
        {
        outEdges.push_back(*it);
        break;
        }
      }
    }
  if (outEdges.size() != this->Internals->Adjacency[index].OutEdges.size())
    {
    vtkErrorMacro("Invalid reorder list.");
    return;
    }
  this->Internals->Adjacency[index].OutEdges = outEdges;
}

//----------------------------------------------------------------------------
bool vtkGraph::IsSameStructure(vtkGraph *other)
{
  return (this->Internals == other->Internals);
}

//----------------------------------------------------------------------------
vtkGraphInternals* vtkGraph::GetGraphInternals(bool modifying)
{
  if (modifying)
    {
    this->ForceOwnership();
    }
  return this->Internals;
}

//----------------------------------------------------------------------------
void vtkGraph::ForceOwnership()
{
  // If the reference count == 1, we own it and can change it.
  // If the reference count > 1, we must make a copy to avoid
  // changing the structure of other graphs.
  if (this->Internals->GetReferenceCount() > 1)
    {
    vtkGraphInternals *internals = vtkGraphInternals::New();
    internals->Adjacency = this->Internals->Adjacency;
    internals->NumberOfEdges = this->Internals->NumberOfEdges;
    this->SetInternals(internals);
    internals->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkGraph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "VertexData: " << (this->VertexData ? "" : "(none)") << endl;
  if (this->VertexData)
    {
    this->VertexData->PrintSelf(os, indent.GetNextIndent());
    }
  os << indent << "EdgeData: " << (this->EdgeData ? "" : "(none)") << endl;
  if (this->EdgeData)
    {
    this->EdgeData->PrintSelf(os, indent.GetNextIndent());
    }
  if (this->Internals)
    {
    os << indent << "DistributedHelper: " 
       << (this->DistributedHelper ? "" : "(none)") << endl;
    if (this->DistributedHelper)
      {
      this->DistributedHelper->PrintSelf(os, indent.GetNextIndent());
      }
    }
}

//----------------------------------------------------------------------------
// Supporting operators
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
bool operator==(vtkEdgeBase e1, vtkEdgeBase e2)
{
  return e1.Id == e2.Id;
}

//----------------------------------------------------------------------------
bool operator!=(vtkEdgeBase e1, vtkEdgeBase e2)
{
  return e1.Id != e2.Id;
}

//----------------------------------------------------------------------------
ostream& operator<<(ostream& out, vtkEdgeBase e)
{
  return out << e.Id;
}
