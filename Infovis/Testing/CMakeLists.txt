SUBDIRS(Cxx)

IF (VTK_WRAP_TCL)
  SUBDIRS(Tcl)
ENDIF (VTK_WRAP_TCL)

#IF (VTK_WRAP_PYTHON)
#  SUBDIRS(Python)
#ENDIF (VTK_WRAP_PYTHON)

IF(PYTHON_EXECUTABLE)
  ADD_TEST(HeaderTesting-Infovis ${PYTHON_EXECUTABLE}
    ${VTK_SOURCE_DIR}/Common/Testing/HeaderTesting.py
    "${VTK_SOURCE_DIR}/Infovis"
    VTK_INFOVIS_EXPORT
    vtkBivariateStatisticsAlgorithmPrivate.h 
    vtkMultiCorrelativeStatisticsAssessFunctor.h 
    vtkBoostGraphAdapter.h
    vtkBoostRandomSparseArraySource.h
    vtkSQLDatabaseGraphSource.h
    vtkSQLDatabaseTableSource.h
    vtkStatisticsAlgorithmPrivate.h 
    vtkUnivariateStatisticsAlgorithmPrivate.h
    )
ENDIF(PYTHON_EXECUTABLE)
