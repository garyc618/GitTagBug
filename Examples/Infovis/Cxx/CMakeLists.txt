PROJECT(Infovis)

FIND_PACKAGE(VTK REQUIRED)
IF(NOT VTK_USE_RENDERING)
  MESSAGE(FATAL_ERROR "Example ${PROJECT_NAME} requires VTK_USE_RENDERING.")
ENDIF(NOT VTK_USE_RENDERING)
IF(NOT VTK_USE_INFOVIS)
  MESSAGE(FATAL_ERROR "Example ${PROJECT_NAME} requires VTK_USE_INFOVIS.")
ENDIF(NOT VTK_USE_INFOVIS)
INCLUDE(${VTK_USE_FILE})

ADD_EXECUTABLE(TreeLayout TreeLayout.cxx)
TARGET_LINK_LIBRARIES(TreeLayout vtkViews vtkInfovis vtkRendering vtkImaging vtkIO)

ADD_EXECUTABLE(HelloWorld HelloWorld.cxx)
TARGET_LINK_LIBRARIES(HelloWorld vtkViews vtkInfovis vtkRendering vtkImaging vtkIO)

ADD_EXECUTABLE(Theme Theme.cxx)
TARGET_LINK_LIBRARIES(Theme vtkViews vtkInfovis vtkRendering vtkImaging vtkIO)

ADD_EXECUTABLE(CreateTree CreateTree.cxx)
TARGET_LINK_LIBRARIES(CreateTree vtkViews vtkInfovis vtkRendering vtkImaging vtkIO)

ADD_EXECUTABLE(MultiView MultiView.cxx)
TARGET_LINK_LIBRARIES(MultiView vtkViews vtkInfovis vtkRendering vtkImaging vtkIO)

# If Qt is enabled include the EasyView example
IF(VTK_USE_QVTK)
  SUBDIRS(ChartView)
  SUBDIRS(EasyView)
  SUBDIRS(StatsView)
ENDIF(VTK_USE_QVTK)

# Excluding from build for now because there is currently no way to check
# if VTK_USE_PARALLEL_BGL is on.
#ADD_EXECUTABLE(ParallelBFS ParallelBFS.cxx)
#TARGET_LINK_LIBRARIES(ParallelBFS vtkParallel vtkViews)
