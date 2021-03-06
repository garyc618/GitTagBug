/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGeoMath.h

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
// .NAME vtkGeoMath - Useful geographic calculations
//
// .SECTION Description
// vtkGeoMath provides some useful geographic calculations.
   
#ifndef __vtkGeoMath_h
#define __vtkGeoMath_h

#include "vtkObject.h"

class VTK_GEOVIS_EXPORT vtkGeoMath : public vtkObject
{
public:
  static vtkGeoMath *New();
  vtkTypeRevisionMacro(vtkGeoMath, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Returns the average radius of the earth in meters.
  static float EarthRadiusMeters() {return 6356750.0f;};

  // Description:
  // Returns the squared distance between two points.
  static double DistanceSquared(double pt0[3], double pt1[3]);

  // Description:
  // Converts a (longitude, latitude, altitude) triple to
  // world coordinates where the center of the earth is at the origin.
  // Units are in meters.
  // Note that having altitude realtive to sea level causes issues.
  static void   LongLatAltToRect(double lla[3], double rect[3]);
  
protected:
  vtkGeoMath();
  ~vtkGeoMath();

private:
  vtkGeoMath(const vtkGeoMath&);  // Not implemented.
  void operator=(const vtkGeoMath&);  // Not implemented.
};

#endif
