from vtk import *

reader2 = vtkXMLTreeReader()
reader2.SetFileName("vtkclasses.xml")
reader2.Update()

reader3 = vtkXMLTreeReader()
reader3.SetFileName("vtklibrary.xml")
reader3.Update()

dummy = vtkIcicleView()
view2 = vtkIcicleView()
view2.SetRepresentationFromInput(reader2.GetOutput())
view2.SetAreaSizeArrayName("size")
view2.SetAreaColorArrayName("vertex id")
view2.SetAreaLabelArrayName("id")
view2.SetAreaLabelVisibility(True)
view2.SetAreaHoverArrayName("id")
view2.SetRootWidth(40.)
view2.SetLayerThickness(2.)
#view2.UseGradientColoring(False)
view2.Update()

view3 = vtkIcicleView()
view3.SetRepresentationFromInput(reader3.GetOutput())
view3.SetAreaSizeArrayName("size")
view3.SetAreaColorArrayName("vertex id")
view3.SetAreaLabelArrayName("id")
view3.SetAreaLabelVisibility(True)
view3.SetAreaHoverArrayName("id")
view3.SetRootWidth(20.)
view3.Update()

# Apply a theme to the views
theme = vtkViewTheme.CreateMellowTheme()
view2.ApplyViewTheme(theme)
view3.ApplyViewTheme(theme)
theme.FastDelete()

win2 = vtkRenderWindow()
win3 = vtkRenderWindow()
dummy.SetupRenderWindow(win2)
view2.SetupRenderWindow(win2)
view3.SetupRenderWindow(win3)
view2.GetRenderer().ResetCamera()
view3.GetRenderer().ResetCamera()

win2.GetInteractor().Initialize()
win3.GetInteractor().Initialize()
win2.GetInteractor().Start()
