from vtk import *

data_dir = "../../../../VTKData/Data/Infovis/SQLite/"
sqlite_file = data_dir + "SmallEmailTest.db"

database = vtkSQLDatabase.CreateFromURL("sqlite://" + sqlite_file)
database.Open("")

edge_query = database.GetQueryInstance()
edge_query.SetQuery("select source, target from emails")

vertex_query = database.GetQueryInstance()
vertex_query.SetQuery("select Name, Job, Age from employee")

edge_table = vtkRowQueryToTable()
edge_table.SetQuery(edge_query)

vertex_table = vtkRowQueryToTable()
vertex_table.SetQuery(vertex_query)

graph = vtkTableToGraph()
graph.AddInputConnection(edge_table.GetOutputPort())
graph.AddLinkVertex("source", "Name", False)
graph.AddLinkVertex("target", "Name", False)
graph.AddLinkEdge("source", "target")
graph.SetVertexTableConnection(vertex_table.GetOutputPort())


view = vtkGraphLayoutView()
view.AddRepresentationFromInputConnection(graph.GetOutputPort())
view.SetVertexLabelArrayName("label")
view.SetVertexLabelVisibility(True)
view.SetVertexColorArrayName("Age")
view.SetColorVertices(True)
view.SetLayoutStrategyToSimple2D()

# Add my new lay strategy
# myFoo = vtkCircularLayoutStrategy()
# view.SetLayoutStrategy(myFoo)

theme = vtkViewTheme.CreateMellowTheme()
theme.SetCellColor(.2,.2,.6)
theme.SetLineWidth(5)
theme.SetPointSize(10)
view.ApplyViewTheme(theme)
view.SetVertexLabelFontSize(20)

window = vtkRenderWindow()
window.SetSize(600, 600)
view.SetupRenderWindow(window)
view.GetRenderer().ResetCamera()
window.GetInteractor().Start()
