PROJECT(vtkQtChart)

# Set up Qt if it hasn't been done already.
IF(NOT QT4_FOUND)
  SET(QT_REQUIRED TRUE)
  SET(QT_MIN_VERSION "4.2.2")
  FIND_PACKAGE(Qt4)
ENDIF(NOT QT4_FOUND)

IF(NOT QT4_FOUND)
  MESSAGE(SEND_ERROR "Qt ${QT_MIN_VERSION} or greater not found.  Please check the QT_QMAKE_EXECUTABLE variable.")
ELSE(NOT QT4_FOUND)

  # Decide whether to use QtOpenGL
  IF(VTK_USE_QVTK_QTOPENGL)
    ADD_DEFINITIONS(-DVTK_USE_QVTK_QTOPENGL)
    SET(QT_USE_QTOPENGL 1)
  ENDIF(VTK_USE_QVTK_QTOPENGL)

  INCLUDE(${QT_USE_FILE})

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

  SET(vtkQtChart_SOURCES
    vtkQtBarChart.cxx
    vtkQtBarChart.h
    vtkQtBarChartOptions.cxx
    vtkQtBarChartOptions.h
    vtkQtBarChartSeriesOptions.cxx
    vtkQtBarChartSeriesOptions.h
    vtkQtChartArea.cxx
    vtkQtChartArea.h
    vtkQtChartAxis.cxx
    vtkQtChartAxis.h
    vtkQtChartAxisCornerDomain.cxx
    vtkQtChartAxisCornerDomain.h
    vtkQtChartAxisDomain.cxx
    vtkQtChartAxisDomain.h
    vtkQtChartAxisDomainPriority.cxx
    vtkQtChartAxisDomainPriority.h
    vtkQtChartAxisLayer.cxx
    vtkQtChartAxisLayer.h
    vtkQtChartAxisModel.cxx
    vtkQtChartAxisModel.h
    vtkQtChartAxisOptions.cxx
    vtkQtChartAxisOptions.h
    vtkQtChartBar.cxx
    vtkQtChartBar.h
    vtkQtChartBarLocator.cxx
    vtkQtChartBarLocator.h
    vtkQtChartColors.cxx
    vtkQtChartColors.h
    vtkQtChartColorStyleGenerator.cxx
    vtkQtChartColorStyleGenerator.h
    vtkQtChartContentsArea.cxx
    vtkQtChartContentsArea.h
    vtkQtChartContentsSpace.cxx
    vtkQtChartContentsSpace.h
    vtkQtChartGridLayer.cxx
    vtkQtChartGridLayer.h
    vtkQtChartHelpFormatter.cxx
    vtkQtChartHelpFormatter.h
    vtkQtChartInteractor.cxx
    vtkQtChartInteractor.h
    vtkQtChartInteractorSetup.cxx
    vtkQtChartInteractorSetup.h
    vtkQtChartLayer.cxx
    vtkQtChartLayer.h
    vtkQtChartLayerDomain.cxx
    vtkQtChartLayerDomain.h
    vtkQtChartLegend.cxx
    vtkQtChartLegend.h
    vtkQtChartLegendManager.cxx
    vtkQtChartLegendManager.h
    vtkQtChartLegendModel.cxx
    vtkQtChartLegendModel.h
    vtkQtChartMouseBox.cxx
    vtkQtChartMouseBox.h
    vtkQtChartMouseFunction.cxx
    vtkQtChartMouseFunction.h
    vtkQtChartMousePan.cxx
    vtkQtChartMousePan.h
    vtkQtChartMouseSelection.cxx
    vtkQtChartMouseSelection.h
    vtkQtChartMouseSelectionHandler.cxx
    vtkQtChartMouseSelectionHandler.h
    vtkQtChartMouseZoom.cxx
    vtkQtChartMouseZoom.h
    vtkQtChartPenBrushGenerator.cxx
    vtkQtChartPenBrushGenerator.h
    vtkQtChartQuad.cxx
    vtkQtChartQuad.h
    vtkQtChartScene.cxx
    vtkQtChartScene.h
    vtkQtChartSeriesDomain.cxx
    vtkQtChartSeriesDomain.h
    vtkQtChartSeriesDomainGroup.cxx
    vtkQtChartSeriesDomainGroup.h
    vtkQtChartSeriesLayer.cxx
    vtkQtChartSeriesLayer.h
    vtkQtChartSeriesModel.cxx
    vtkQtChartSeriesModel.h
    vtkQtChartSeriesModelCollection.cxx
    vtkQtChartSeriesModelCollection.h
    vtkQtChartSeriesModelRange.cxx
    vtkQtChartSeriesModelRange.h
    vtkQtChartSeriesOptions.cxx
    vtkQtChartSeriesOptions.h
    vtkQtChartSeriesSelection.cxx
    vtkQtChartSeriesSelection.h
    vtkQtChartSeriesSelectionHandler.cxx
    vtkQtChartSeriesSelectionHandler.h
    vtkQtChartSeriesSelectionModel.cxx
    vtkQtChartSeriesSelectionModel.h
    vtkQtChartShape.cxx
    vtkQtChartShape.h
    vtkQtChartShapeLocator.cxx
    vtkQtChartShapeLocator.h
    vtkQtChartStyleGenerator.cxx
    vtkQtChartStyleGenerator.h
    vtkQtChartStyleManager.cxx
    vtkQtChartStyleManager.h
    vtkQtChartTableSeriesModel.cxx
    vtkQtChartTableSeriesModel.h
    vtkQtChartTitle.cxx
    vtkQtChartTitle.h
    vtkQtChartWidget.cxx
    vtkQtChartWidget.h
    vtkQtChartZoomHistory.cxx
    vtkQtChartZoomHistory.h
    vtkQtLineChart.cxx
    vtkQtLineChart.h
    vtkQtLineChartOptions.cxx
    vtkQtLineChartOptions.h
    vtkQtLineChartSeriesOptions.cxx
    vtkQtLineChartSeriesOptions.h
    vtkQtPointMarker.cxx
    vtkQtPointMarker.h
    vtkQtPolylineItem.cxx
    vtkQtPolylineItem.h
    vtkQtSeriesFilterLineEdit.cxx
    vtkQtSeriesFilterLineEdit.h
    vtkQtStackedChart.cxx
    vtkQtStackedChart.h
    vtkQtStackedChartOptions.cxx
    vtkQtStackedChartOptions.h
    vtkQtStackedChartSeriesOptions.cxx
    vtkQtStackedChartSeriesOptions.h
    vtkQtStatisticalBoxChart.cxx
    vtkQtStatisticalBoxChart.h
    vtkQtStatisticalBoxChartOptions.cxx
    vtkQtStatisticalBoxChartOptions.h
    vtkQtStatisticalBoxChartSeriesOptions.cxx
    vtkQtStatisticalBoxChartSeriesOptions.h
  )
  
  SET(vtkQtChart_MOCHEADERS
    vtkQtBarChart.h
    vtkQtBarChartOptions.h
    vtkQtBarChartSeriesOptions.h
    vtkQtChartArea.h
    vtkQtChartAxis.h
    vtkQtChartAxisLayer.h
    vtkQtChartAxisModel.h
    vtkQtChartAxisOptions.h
    vtkQtChartColorStyleGenerator.h
    vtkQtChartStyleGenerator.h
    vtkQtChartContentsSpace.h
    vtkQtChartGridLayer.h
    vtkQtChartInteractor.h
    vtkQtChartLayer.h
    vtkQtChartLegend.h
    vtkQtChartLegendManager.h
    vtkQtChartLegendModel.h
    vtkQtChartMouseBox.h
    vtkQtChartMouseFunction.h
    vtkQtChartMouseSelection.h
    vtkQtChartPenBrushGenerator.h
    vtkQtChartSeriesLayer.h
    vtkQtChartSeriesModel.h
    vtkQtChartSeriesModelCollection.h
    vtkQtChartSeriesModelRange.h
    vtkQtChartSeriesOptions.h
    vtkQtChartSeriesSelectionModel.h
    vtkQtChartTableSeriesModel.h
    vtkQtChartTitle.h
    vtkQtChartWidget.h
    vtkQtLineChart.h
    vtkQtLineChartOptions.h
    vtkQtLineChartSeriesOptions.h
    vtkQtSeriesFilterLineEdit.h
    vtkQtStackedChart.h
    vtkQtStackedChartOptions.h
    vtkQtStackedChartSeriesOptions.h
    vtkQtStatisticalBoxChart.h
    vtkQtStatisticalBoxChartOptions.h
    vtkQtStatisticalBoxChartSeriesOptions.h
    )
  SET(vtkQtChart_HEADERS
    vtkQtChartGraphicsItemTypes.h
    )

SET(MOC_INCLUDE_DIRS ${MOC_INCLUDE_DIRS} ${QT_INCLUDE_DIR})

# Reduce the number of dirs that get included on moc command line
# since it causes issues on Windows 2000.
GET_DIRECTORY_PROPERTY(include_dirs_tmp INCLUDE_DIRECTORIES)
SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${MOC_INCLUDE_DIRS}")

  QT4_WRAP_CPP(vtkQtChart_MOC_SOURCES ${vtkQtChart_MOCHEADERS})

SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${include_dirs_tmp}")

  SOURCE_GROUP("Generated" FILES
    ${vtkQtChart_MOC_SOURCES}
  )

  ADD_LIBRARY(vtkQtChart
    ${vtkQtChart_SOURCES}
    ${vtkQtChart_HEADERS}
    ${vtkQtChart_MOC_SOURCES}
  )

  # Apply user-defined properties to the library target.
  IF(VTK_LIBRARY_PROPERTIES)
    SET_TARGET_PROPERTIES(vtkQtChart PROPERTIES ${VTK_LIBRARY_PROPERTIES})
  ENDIF(VTK_LIBRARY_PROPERTIES)

  TARGET_LINK_LIBRARIES(vtkQtChart
    ${QT_LIBRARIES}
  )

  SET(VTKQTCHART_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/vtkQtChartExport.h.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/vtkQtChartExport.h")

  IF(NOT VTK_INSTALL_NO_DEVELOPMENT AND VTK_INSTALL_INCLUDE_DIR_CM24)
    INSTALL(FILES ${vtkQtChart_MOCHEADERS} ${vtkQtChart_HEADERS}
      DESTINATION ${VTK_INSTALL_INCLUDE_DIR_CM24} COMPONENT Development)
  ENDIF(NOT VTK_INSTALL_NO_DEVELOPMENT AND VTK_INSTALL_INCLUDE_DIR_CM24)

  IF(NOT VTK_INSTALL_NO_LIBRARIES AND VTK_INSTALL_BIN_DIR_CM24 AND VTK_INSTALL_LIB_DIR_CM24)
    INSTALL(TARGETS vtkQtChart
      RUNTIME DESTINATION ${VTK_INSTALL_BIN_DIR_CM24} COMPONENT RuntimeLibraries
      LIBRARY DESTINATION ${VTK_INSTALL_LIB_DIR_CM24} COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION ${VTK_INSTALL_LIB_DIR_CM24} COMPONENT Development)
  ENDIF(NOT VTK_INSTALL_NO_LIBRARIES AND VTK_INSTALL_BIN_DIR_CM24 AND VTK_INSTALL_LIB_DIR_CM24)

ENDIF(NOT QT4_FOUND)

