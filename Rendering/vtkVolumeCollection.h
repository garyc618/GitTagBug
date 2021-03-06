/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkVolumeCollection.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVolumeCollection - a list of volumes
// .SECTION Description
// vtkVolumeCollection represents and provides methods to manipulate a 
// list of volumes (i.e., vtkVolume and subclasses). The list is unsorted 
// and duplicate entries are not prevented.

// .SECTION see also
// vtkCollection vtkVolume

#ifndef __vtkVolumeCollection_h
#define __vtkVolumeCollection_h

#include "vtkPropCollection.h"

#include "vtkVolume.h"  // Needed for static cast

class VTK_RENDERING_EXPORT vtkVolumeCollection : public vtkPropCollection
{
 public:
  static vtkVolumeCollection *New();
  vtkTypeRevisionMacro(vtkVolumeCollection,vtkPropCollection);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a Volume to the list.
  void AddItem(vtkVolume *a)
    {
      this->vtkCollection::AddItem(a);
    }
    
  // Description:
  // Get the next Volume in the list. Return NULL when at the end of the 
  // list.
  vtkVolume *GetNextVolume() {
      return static_cast<vtkVolume *>(this->GetNextItemAsObject());};


  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextVolume() variant
  // where possible.
  vtkVolume *GetNextItem() { return this->GetNextVolume(); };

  //BTX
  // Description: 
  // Reentrant safe way to get an object in a collection. Just pass the
  // same cookie back and forth. 
  vtkVolume *GetNextVolume(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkVolume *>(this->GetNextItemAsObject(cookie));};
  //ETX

protected:
  vtkVolumeCollection() {};
  ~vtkVolumeCollection() {};

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };
  void AddItem(vtkProp *o) { this->vtkPropCollection::AddItem(o); };

private:
  vtkVolumeCollection(const vtkVolumeCollection&);  // Not implemented.
  void operator=(const vtkVolumeCollection&);  // Not implemented.
};

#endif
