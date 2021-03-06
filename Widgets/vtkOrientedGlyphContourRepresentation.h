/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOrientedGlyphContourRepresentation.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOrientedGlyphContourRepresentation - Default representation for the contour widget
// .SECTION Description
// This class provides the default concrete representation for the 
// vtkContourWidget. It works in conjunction with the 
// vtkContourLineInterpolator and vtkPointPlacer. See vtkContourWidget
// for details.
// .SECTION See Also
// vtkContourRepresentation vtkContourWidget vtkPointPlacer 
// vtkContourLineInterpolator

#ifndef __vtkOrientedGlyphContourRepresentation_h
#define __vtkOrientedGlyphContourRepresentation_h

#include "vtkContourRepresentation.h"

class vtkProperty;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkGlyph3D;
class vtkPoints;

class VTK_WIDGETS_EXPORT vtkOrientedGlyphContourRepresentation : public vtkContourRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkOrientedGlyphContourRepresentation *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeRevisionMacro(vtkOrientedGlyphContourRepresentation,vtkContourRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the cursor shape. Keep in mind that the shape will be
  // aligned with the  constraining plane by orienting it such that
  // the x axis of the geometry lies along the normal of the plane.
  void SetCursorShape(vtkPolyData *cursorShape);
  vtkPolyData *GetCursorShape();

  // Description:
  // Specify the shape of the cursor (handle) when it is active.
  // This is the geometry that will be used when the mouse is
  // close to the handle or if the user is manipulating the handle.
  void SetActiveCursorShape(vtkPolyData *activeShape);
  vtkPolyData *GetActiveCursorShape();

  // Description:
  // This is the property used when the handle is not active 
  // (the mouse is not near the handle)
  vtkGetObjectMacro(Property,vtkProperty);
  
  // Description:
  // This is the property used when the user is interacting
  // with the handle.
  vtkGetObjectMacro(ActiveProperty,vtkProperty);
  
  // Description:
  // This is the property used by the lines.
  vtkGetObjectMacro(LinesProperty,vtkProperty);
  
  // Description:
  // Subclasses of vtkOrientedGlyphContourRepresentation must implement these methods. These
  // are the methods that the widget and its representation use to
  // communicate with each other.
  virtual void SetRenderer(vtkRenderer *ren);
  virtual void BuildRepresentation();
  virtual void StartWidgetInteraction(double eventPos[2]);
  virtual void WidgetInteraction(double eventPos[2]);
  virtual int ComputeInteractionState(int X, int Y, int modified=0);

  // Description:
  // Methods to make this class behave as a vtkProp.
  virtual void GetActors(vtkPropCollection *);
  virtual void ReleaseGraphicsResources(vtkWindow *);
  virtual int RenderOverlay(vtkViewport *viewport);
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);
  virtual int RenderTranslucentPolygonalGeometry(vtkViewport *viewport);
  virtual int HasTranslucentPolygonalGeometry();
  
  // Description:
  // Get the points in this contour as a vtkPolyData. 
  virtual vtkPolyData * GetContourRepresentationAsPolyData();

  // Description:
  // Controls whether the contour widget should always appear on top
  // of other actors in the scene. (In effect, this will disable OpenGL
  // Depth buffer tests while rendering the contour).
  // Default is to set it to false.
  vtkSetMacro( AlwaysOnTop, int );
  vtkGetMacro( AlwaysOnTop, int );
  vtkBooleanMacro( AlwaysOnTop, int );
  
protected:
  vtkOrientedGlyphContourRepresentation();
  ~vtkOrientedGlyphContourRepresentation();

  // Render the cursor
  vtkActor             *Actor;
  vtkPolyDataMapper    *Mapper;
  vtkGlyph3D           *Glypher;
  vtkActor             *ActiveActor;
  vtkPolyDataMapper    *ActiveMapper;
  vtkGlyph3D           *ActiveGlypher;
  vtkPolyData          *CursorShape;
  vtkPolyData          *ActiveCursorShape;
  vtkPolyData          *FocalData;
  vtkPoints            *FocalPoint;
  vtkPolyData          *ActiveFocalData;
  vtkPoints            *ActiveFocalPoint;

  vtkPolyData          *Lines;
  vtkPolyDataMapper    *LinesMapper;
  vtkActor             *LinesActor;
  
  // Support picking
  double LastPickPosition[3];
  double LastEventPosition[2];
  
  // Methods to manipulate the cursor
  void Translate(double eventPos[2]);
  void Scale(double eventPos[2]);
  void ShiftContour(double eventPos[2]);
  void ScaleContour(double eventPos[2]);

  void ComputeCentroid(double* ioCentroid);

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty   *Property;
  vtkProperty   *ActiveProperty;
  vtkProperty   *LinesProperty;
  void           CreateDefaultProperties();
  
  
  // Distance between where the mouse event happens and where the
  // widget is focused - maintain this distance during interaction.
  double InteractionOffset[2];

  int AlwaysOnTop;

  void BuildLines();
  
private:
  vtkOrientedGlyphContourRepresentation(const vtkOrientedGlyphContourRepresentation&);  //Not implemented
  void operator=(const vtkOrientedGlyphContourRepresentation&);  //Not implemented
};

#endif
