# if we have rendering add the following tests
IF (VTK_USE_RENDERING AND VTK_USE_DISPLAY)
  SET(KIT Infovis)
  # add tests that do not require data
  SET(MyTests  
    TestArcEdges
    TestAssignCoordinates
    TestContingencyStatistics
    TestCorrelativeStatistics
    TestCosmicTreeLayoutStrategy
    TestDataObjectToTable
    TestDescriptiveStatistics
    TestExtractSelectedGraph
    TestGraph
    TestGraphAttributes
    TestGraphAlgorithms   
    TestGraphHierarchicalBundle
    TestGraphLayoutStrategy
    TestGroupLeafVertices
    TestInteractorStyleTreeMapHover
    TestKdTreeBoxSelection
    TestMultiCorrelativeStatistics
    TestOrderStatistics
    TestPassThrough
    TestPCAStatistics
    TestPruneTreeFilter
    TestRandomGraphSource
    TestStringToCategory
    TestTable
    TestTreeMapLayoutStrategy
    TestThresholdTable
    TestVariantArray
    TestVariant
    )
  IF (VTK_DATA_ROOT)
    # add tests that require data
    SET(MyTests ${MyTests}
      TestChacoGraphReader
      TestDelimitedTextReader
      TestISIReader
      TestFixedWidthTextReader
      TestRISReader
      TestStringToNumeric
      TestTreeMapViewer
      TestTulipReader
      )
    IF (VTK_USE_VIEWS)
      SET(MyTests ${MyTests}
        TestTableToGraph
      )
    ENDIF (VTK_USE_VIEWS)
  ENDIF (VTK_DATA_ROOT)
  IF (VTK_USE_BOOST)
    # add boost tests
    SET(MyTests ${MyTests}
      TestBoostAdapter
      TestBoostAlgorithms
      )

    IF (VTK_USE_PARALLEL_BGL)
      # The variant-serialization test is kept separate, because it
      # needs to link against the serialization library.
      ADD_EXECUTABLE(TestVariantSerialization TestVariantSerialization.cxx)
      TARGET_LINK_LIBRARIES(TestVariantSerialization vtkInfovis ${Boost_SERIALIZATION_LIBRARY})
      ADD_TEST(TestVariantSerialization ${CXX_TEST_PATH}/TestVariantSerialization)
    ENDIF (VTK_USE_PARALLEL_BGL)

    IF(VTK_DATA_ROOT)
      SET(MyTests ${MyTests}
        TestBoostSplitTableField
        )
    ENDIF(VTK_DATA_ROOT)

    IF (VTK_USE_PARALLEL_BGL)
      # Add tests using the Parallel BGL
      ADD_EXECUTABLE(TestPBGLAlgorithms TestPBGLAlgorithms.cxx)
      TARGET_LINK_LIBRARIES(TestPBGLAlgorithms vtkInfovis ${Boost_LIBRARIES})

      ADD_TEST(TestPBGLAlgorithms
        ${VTK_MPIRUN_EXE} ${VTK_MPI_PRENUMPROC_FLAGS} ${VTK_MPI_NUMPROC_FLAG} ${VTK_MPI_MAX_NUMPROCS} 
        ${VTK_MPI_PREFLAGS} 
        ${CXX_TEST_PATH}/TestPBGLAlgorithms
        ${VTK_MPI_POSTFLAGS})
    ENDIF (VTK_USE_PARALLEL_BGL)   
  ENDIF (VTK_USE_BOOST)

  IF(VTK_USE_PARALLEL AND VTK_USE_MPI)
    # Add parallel tests only if PARALLEL and MPI are turned ON
    # These tests are kept separate, because they must be run with mpirun or mpiexec
    ADD_EXECUTABLE(TestParallelRandomStatisticsMPI TestParallelRandomStatisticsMPI.cxx)
    TARGET_LINK_LIBRARIES(TestParallelRandomStatisticsMPI vtkInfovis vtkParallel)

    IF (VTK_MPIRUN_EXE)
      ADD_TEST(TestParallelRandomStatisticsMPI
        ${VTK_MPIRUN_EXE} ${VTK_MPI_PRENUMPROC_FLAGS} ${VTK_MPI_NUMPROC_FLAG} ${VTK_MPI_MAX_NUMPROCS}
        ${VTK_MPI_PREFLAGS}
        ${CXX_TEST_PATH}/TestParallelRandomStatisticsMPI
        ${VTK_MPI_POSTFLAGS}
        )
    ENDIF (VTK_MPIRUN_EXE)

  ENDIF (VTK_USE_PARALLEL AND VTK_USE_MPI)

  SET(VTK_HAS_UINT64_ARRAY 0)
  IF(VTK_SIZEOF_LONG_LONG MATCHES "^8$")
    SET(VTK_HAS_UINT64_ARRAY 1)
  ELSE(VTK_SIZEOF_LONG_LONG MATCHES "^8$")
    IF(VTK_SIZEOF_LONG MATCHES "^8$")
      SET(VTK_HAS_UINT64_ARRAY 1)
    ELSE(VTK_SIZEOF_LONG MATCHES "^8$")
      IF(VTK_SIZEOF___INT64 MATCHES "^8$")
        IF(VTK_TYPE_CONVERT_UI64_TO_DOUBLE)
          SET(VTK_HAS_UINT64_ARRAY 1)
        ENDIF(VTK_TYPE_CONVERT_UI64_TO_DOUBLE)
      ENDIF(VTK_SIZEOF___INT64 MATCHES "^8$")
    ENDIF(VTK_SIZEOF_LONG MATCHES "^8$")
  ENDIF(VTK_SIZEOF_LONG_LONG MATCHES "^8$")
  IF(VTK_HAS_UINT64_ARRAY)
    # add these tests only if vtkTypeUInt64Array exists
    SET(MyTests ${MyTests}
      TestSQLGraphReader
      TestTimePoint
      )    
  ENDIF(VTK_HAS_UINT64_ARRAY)
  CREATE_TEST_SOURCELIST(Tests ${KIT}CxxTests.cxx ${MyTests}
                         EXTRA_INCLUDE vtkTestDriver.h)
  ADD_EXECUTABLE(${KIT}CxxTests ${Tests})
  TARGET_LINK_LIBRARIES(${KIT}CxxTests vtkInfovis vtkRendering vtkIO vtkCommon)
  SET (TestsToRun ${Tests})
  REMOVE (TestsToRun ${KIT}CxxTests.cxx)

  #
  # Add all the executables 
  FOREACH (test ${TestsToRun})
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    IF (VTK_DATA_ROOT)
      ADD_TEST(Infovis-${TName} ${CXX_TEST_PATH}/${KIT}CxxTests ${TName}
          -D ${VTK_DATA_ROOT}
          -T ${VTK_BINARY_DIR}/Testing/Temporary
          -V Baseline/${KIT}/${TName}.png)
    ELSE (VTK_DATA_ROOT)
      ADD_TEST(Infovis-${TName} ${CXX_TEST_PATH}/${KIT}CxxTests ${TName}) 
    ENDIF (VTK_DATA_ROOT)
  ENDFOREACH (test) 
ENDIF (VTK_USE_RENDERING AND VTK_USE_DISPLAY)
IF(VTK_USE_N_WAY_ARRAYS)

  CREATE_TEST_SOURCELIST(ArrayTests ArrayCxxTests.cxx
    ArrayAPIDenseCoordinates.cxx
    ArrayCosineSimilarity.cxx
    ArrayDiagonalMatrixSource.cxx
    ArrayExtractFactoredArray.cxx
    ArrayMatricizeArray.cxx
    ArrayMatrixColumnNorm.cxx
    ArrayNormalizeMatrixVectors.cxx
    ArrayTableToSparseArray.cxx
    ArrayToTable.cxx
    ArrayTransposeMatrix.cxx
    )

  ADD_EXECUTABLE(InfovisArrayCxxTests ${ArrayTests})
  TARGET_LINK_LIBRARIES(InfovisArrayCxxTests vtkInfovis)

  SET(ArrayTestsToRun ${ArrayTests})
  REMOVE(ArrayTestsToRun ArrayCxxTests.cxx)

  FOREACH(test ${ArrayTestsToRun})
    GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
    ADD_TEST(Array-${TName} ${CXX_TEST_PATH}/InfovisArrayCxxTests ${TName})
  ENDFOREACH(test)

  IF(VTK_USE_BOOST)

    CREATE_TEST_SOURCELIST(BoostArrayTests BoostArrayCxxTests.cxx
      BoostArrayLogWeighting.cxx
      BoostArrayRandomSparseArraySource.cxx
      )

    ADD_EXECUTABLE(InfovisBoostArrayCxxTests ${BoostArrayTests})
    TARGET_LINK_LIBRARIES(InfovisBoostArrayCxxTests vtkInfovis)

    SET(BoostArrayTestsToRun ${BoostArrayTests})
    REMOVE(BoostArrayTestsToRun BoostArrayCxxTests.cxx)

    FOREACH(test ${BoostArrayTestsToRun})
      GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
      ADD_TEST(Boost-Array-${TName} ${CXX_TEST_PATH}/InfovisBoostArrayCxxTests ${TName})
    ENDFOREACH(test)

  ENDIF(VTK_USE_BOOST)
ENDIF(VTK_USE_N_WAY_ARRAYS)

