from vtk import *

source = vtkRandomGraphSource()
source.DirectedOff()
source.SetNumberOfVertices(100)
source.SetEdgeProbability(0.1)
source.SetUseEdgeProbability(True)
source.AllowParallelEdgesOn()
source.AllowSelfLoopsOn() 
source.SetStartWithTree(True)

# Connect to the centrality filter.
centrality = vtkBoostBrandesCentrality ()
centrality.SetInputConnection(source.GetOutputPort())

# Find the minimal spanning tree
mstTreeSelection = vtkBoostKruskalMinimumSpanningTree()
mstTreeSelection.SetInputConnection(centrality.GetOutputPort())
mstTreeSelection.SetEdgeWeightArrayName("centrality")
mstTreeSelection.NegateEdgeWeightsOn()
mstTreeSelection.Update()

# Take selection and extract a graph
extract_graph = vtkExtractSelectedGraph()
extract_graph.AddInputConnection(centrality.GetOutputPort())
extract_graph.SetSelectionConnection(mstTreeSelection.GetOutputPort())

# Create a graph layout view
view = vtkGraphLayoutView()
view.AddRepresentationFromInputConnection(centrality.GetOutputPort())
view.SetVertexLabelArrayName("centrality")
view.SetVertexLabelVisibility(True)
view.SetVertexColorArrayName("centrality")
view.SetColorVertices(True)
view.SetEdgeColorArrayName("centrality")
view.SetColorEdges(True)
view.SetLayoutStrategyToSimple2D()

# Create a second shrubery!
view2 = vtkGraphLayoutView()
view2.AddRepresentationFromInputConnection(extract_graph.GetOutputPort())
view2.SetVertexLabelArrayName("centrality")
view2.SetVertexLabelVisibility(True)
view2.SetVertexColorArrayName("centrality")
view2.SetColorVertices(True)
view2.SetEdgeColorArrayName("centrality")
view2.SetColorEdges(True)
view2.SetLayoutStrategyToSimple2D()

# Make sure all views are using a pedigree id selection
view.SetSelectionType(2)
view2.SetSelectionType(2)

# Create a selection link and set both view to use it
#selectionLink = vtkSelectionLink()
#view.GetRepresentation(0).SetSelectionLink(selectionLink)
#view2.GetRepresentation(0).SetSelectionLink(selectionLink)

# Set the selection to be the MST
view.GetRepresentation(0).GetSelectionLink().SetSelection(mstTreeSelection.GetOutput())


# Set the theme on the view
theme = vtkViewTheme.CreateMellowTheme()
theme.SetLineWidth(4)
theme.SetPointSize(8)
theme.SetSelectedCellColor(1,0,1)
theme.SetSelectedPointColor(1,0,1)
view.ApplyViewTheme(theme)
view2.ApplyViewTheme(theme)

window = vtkRenderWindow()
window.SetSize(600, 600)
view.SetupRenderWindow(window)
view.GetRenderer().ResetCamera()

window2 = vtkRenderWindow()
window2.SetSize(600, 600)
view2.SetupRenderWindow(window2)
view2.GetRenderer().ResetCamera()

window.GetInteractor().Start()

