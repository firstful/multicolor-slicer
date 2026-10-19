cmake_minimum_required(VERSION 3.13)
project(libslic3r)

include(PrecompiledHeader)

string(TIMESTAMP COMPILE_TIME %Y%m%d-%H%M%S)
set(SLIC3R_BUILD_TIME ${COMPILE_TIME})

if("${BBL_RELEASE_TO_PUBLIC}" STREQUAL "0")
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_SHA1
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(SLIC3R_COMPILE_VERSION "${GIT_SHA1}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libslic3r_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h @ONLY)

if (MINGW)
    add_compile_options(-Wa,-mbig-obj)
endif ()

set(OpenVDBUtils_SOURCES "")
# find_package(OpenVDB) creates the OpenVDB::openvdb imported target even when the
# package is missing (with NOTFOUND include/library paths), so TARGET alone is not
# a sufficient test. Require OpenVDB_FOUND as well.
if (OpenVDB_FOUND AND TARGET OpenVDB::openvdb)
    set(OpenVDBUtils_SOURCES OpenVDBUtils.cpp OpenVDBUtils.hpp)
endif()

set(lisbslic3r_sources
    ArcFitter.cpp
    ArcFitter.hpp
    pchheader.cpp
    pchheader.hpp
    AABBTreeIndirect.hpp
    AABBTreeLines.hpp
    Algorithm/LineSegmentation/LineSegmentation.cpp
    Algorithm/LineSegmentation/LineSegmentation.hpp
    AnyPtr.hpp
    AStar.hpp
    LogSink.hpp
    LogSink.cpp
    BoundingBox.cpp
    BoundingBox.hpp
    BridgeDetector.cpp
    BridgeDetector.hpp
    ByObjectPrintData.hpp
    ByObjectPrintData.cpp
    FaceDetector.cpp
    FaceDetector.hpp
    Brim.cpp
    Brim.hpp
    BuildVolume.cpp
    BuildVolume.hpp
	Calib.cpp
    Calib.hpp
    Circle.cpp
    Circle.hpp
    clipper.cpp
    clipper.hpp
    ClipperUtils.cpp
    ClipperUtils.hpp
    Clipper2Utils.cpp
    Clipper2Utils.hpp
    Color.cpp
    Color.hpp
    CommonDefs.hpp
    Config.cpp
    Config.hpp
    CurveAnalyzer.cpp
    CurveAnalyzer.hpp
    CutUtils.cpp
    CutUtils.hpp
    PaintReproject.cpp
    PaintReproject.hpp
    EdgeGrid.cpp
    EdgeGrid.hpp
    ElephantFootCompensation.cpp
    ElephantFootCompensation.hpp

    enum_bitmask.hpp
    ExPolygon.cpp
    ExPolygon.hpp
    ExPolygonSerialize.hpp
    ExPolygonsIndex.cpp
    ExPolygonsIndex.hpp
    Extruder.cpp
    Extruder.hpp
    ExtrusionEntity.cpp
    ExtrusionEntity.hpp
    ExtrusionEntityCollection.cpp
    ExtrusionEntityCollection.hpp
    ExtrusionSimulator.cpp
    ExtrusionSimulator.hpp
    FileParserError.hpp
    Fill/Fill.cpp
    Fill/Fill.hpp
    Fill/Fill3DHoneycomb.cpp
    Fill/Fill3DHoneycomb.hpp
    Fill/FillAdaptive.cpp
    Fill/FillAdaptive.hpp
    Fill/FillBase.cpp
    Fill/FillBase.hpp
    Fill/FillConcentric.cpp
    Fill/FillConcentric.hpp
    Fill/FillConcentricInternal.cpp
    Fill/FillConcentricInternal.hpp
    Fill/FillCrossHatch.cpp
    Fill/FillCrossHatch.hpp
    Fill/FillHoneycomb.cpp
    Fill/FillHoneycomb.hpp
    Fill/FillGyroid.cpp
    Fill/FillGyroid.hpp
    Fill/FillPlanePath.cpp
    Fill/FillPlanePath.hpp
    Fill/FillLine.cpp
    Fill/FillLine.hpp
    Fill/FillLightning.cpp
    Fill/FillLightning.hpp
    Fill/Lightning/DistanceField.cpp
    Fill/Lightning/DistanceField.hpp
    Fill/Lightning/Generator.cpp
    Fill/Lightning/Generator.hpp
    Fill/Lightning/Layer.cpp
    Fill/Lightning/Layer.hpp
    Fill/Lightning/TreeNode.cpp
    Fill/Lightning/TreeNode.hpp
    Fill/FillRectilinear.cpp
    Fill/FillRectilinear.hpp
    Fill/FillConformal.cpp
    Fill/FillConformal.hpp
    Fill/FillConformalChart.cpp
    Fill/FillConformalChart.hpp
    Fill/FillRadialZigZag.cpp
    Fill/FillRadialZigZag.hpp
    Fill/FillFloatingConcentric.hpp
    Fill/FillFloatingConcentric.cpp
    Flow.cpp
    Flow.hpp
    Frustum.cpp
    Frustum.hpp
    FlushVolCalc.cpp
    FlushVolCalc.hpp
    FuzzySkin.cpp
    FuzzySkin.hpp
    format.hpp
    Format/3mf.cpp
    Format/3mf.hpp
    Format/AssemblyStepsJson.cpp
    Format/AssemblyStepsJson.hpp
    Format/AssemblyTreeJson.cpp
    Format/AssemblyTreeJson.hpp
    Format/bbs_3mf.cpp
    Format/bbs_3mf.hpp
    Format/OBJ.cpp
    Format/OBJ.hpp
    Format/objparser.cpp
    Format/objparser.hpp
    Format/STL.cpp
    Format/STL.hpp




    GCode/ThumbnailData.cpp
    GCode/ThumbnailData.hpp
    GCode/GCodeEditor.cpp
    GCode/GCodeEditor.hpp

#    GCode/PressureEqualizer.cpp
#    GCode/PressureEqualizer.hpp
    GCode/PrintExtents.cpp
    GCode/PrintExtents.hpp
    GCode/RetractWhenCrossingPerimeters.cpp
    GCode/RetractWhenCrossingPerimeters.hpp
    GCode/SpiralVase.cpp
    GCode/SpiralVase.hpp
    GCode/SeamPlacer.cpp
    GCode/SeamPlacer.hpp
    GCode/ToolOrdering.cpp
    GCode/ToolOrdering.hpp
    GCode/WipeTower.cpp
    GCode/WipeTower.hpp
    GCode/GCodeProcessor.cpp
    GCode/GCodeProcessor.hpp
    GCode/AvoidCrossingPerimeters.cpp
    GCode/AvoidCrossingPerimeters.hpp
    GCode/ConflictChecker.cpp
    GCode/ConflictChecker.hpp
    GCode/BedExcludeChecker.cpp
    GCode/BedExcludeChecker.hpp
    GCode/Smoothing.cpp
    GCode/Smoothing.hpp
    GCode/CoolingBuffer.cpp
    GCode/CoolingBuffer.hpp
    GCode/TimelapsePosPicker.cpp
    GCode/TimelapsePosPicker.hpp
    GCode.cpp
    GCode.hpp
    GCodeReader.cpp
    GCodeReader.hpp
    # GCodeSender.cpp
    # GCodeSender.hpp
    GCodeWriter.cpp
    GCodeWriter.hpp
    Geometry.cpp
    Geometry.hpp
    Geometry/Bicubic.hpp
    Geometry/Circle.cpp
    Geometry/Circle.hpp
    Geometry/ConvexHull.cpp
    Geometry/ConvexHull.hpp
    Geometry/Curves.hpp
    Geometry/MedialAxis.cpp
    Geometry/MedialAxis.hpp
	Geometry/Voronoi.cpp
    Geometry/Voronoi.hpp
    Geometry/VoronoiOffset.cpp
    Geometry/VoronoiOffset.hpp
	Geometry/VoronoiUtils.hpp
    Geometry/VoronoiUtils.cpp

    Geometry/VoronoiVisualUtils.hpp
    Int128.hpp
    InternalBridgeDetector.cpp
    InternalBridgeDetector.hpp
    JumpPointSearch.hpp
    JumpPointSearch.cpp
    KDTreeIndirect.hpp
    Layer.cpp
    Layer.hpp
    LayerRegion.cpp
    libslic3r.h
    Line.cpp
    Line.hpp
    BlacklistedLibraryCheck.cpp
    BlacklistedLibraryCheck.hpp
    LocalesUtils.cpp
    LocalesUtils.hpp
    Model.cpp
    Model.hpp
    ModelArrange.hpp
    ModelArrange.cpp
    MultiMaterialSegmentation.cpp
    MultiMaterialSegmentation.hpp
    Measure.hpp
    Measure.cpp
    MeasureUtils.hpp
    CustomGCode.cpp
    CustomGCode.hpp
    Arrange.hpp
    Arrange.cpp
    NormalUtils.cpp
    NormalUtils.hpp
    Orient.hpp
    Orient.cpp
    MultiPoint.cpp
    MultiPoint.hpp
    MutablePriorityQueue.hpp
    ObjectID.cpp
    ObjectID.hpp

	ParameterUtils.cpp
    ParameterUtils.hpp
    PerimeterGenerator.cpp
    PerimeterGenerator.hpp
    OverhangDetector.hpp
    OverhangDetector.cpp
    PlaceholderParser.cpp
    PlaceholderParser.hpp
    Platform.cpp
    Platform.hpp
    Point.cpp
    Point.hpp
    Polygon.cpp
    Polygon.hpp
    MutablePolygon.cpp
    MutablePolygon.hpp
    PolygonTrimmer.cpp
    PolygonTrimmer.hpp
    Polyline.cpp
    Polyline.hpp
    Preset.cpp
    Preset.hpp
    PresetBundle.cpp
    PresetBundle.hpp
    ProjectTask.cpp
    ProjectTask.hpp
    PrincipalComponents2D.hpp
    PrincipalComponents2D.cpp
    AppConfig.cpp
    AppConfig.hpp

    Print.cpp
    Print.hpp
    PrintApply.cpp
    PrintBase.cpp
    PrintBase.hpp
    PrintConfig.cpp
    PrintConfig.hpp
    PrintObject.cpp
    PrintObjectSlice.cpp
    PrintRegion.cpp
    PNGReadWrite.hpp
    PNGReadWrite.cpp
    QuadricEdgeCollapse.cpp
    QuadricEdgeCollapse.hpp
    Semver.cpp
    ShortEdgeCollapse.cpp
    ShortEdgeCollapse.hpp
    ShortestPath.cpp
    ShortestPath.hpp
    Slicing.cpp
    Slicing.hpp
    SlicesToTriangleMesh.hpp
    SlicesToTriangleMesh.cpp
    SlicingAdaptive.cpp
    SlicingAdaptive.hpp
    Support/SupportCommon.cpp
    Support/SupportCommon.hpp
    Support/SupportMaterial.cpp
    Support/SupportMaterial.hpp
    Support/SupportParameters.hpp
    MinimumSpanningTree.hpp
    MinimumSpanningTree.cpp
    Surface.cpp
    Surface.hpp
    SurfaceCollection.cpp
    SurfaceCollection.hpp
    SurfaceMesh.hpp
    SVG.cpp
    SVG.hpp
    Technologies.hpp
    Tesselate.cpp
    Tesselate.hpp
    TriangleMesh.cpp
    TriangleMesh.hpp
    MeshDiagnostics.hpp
    MeshDiagnostics.cpp
    MeshRaycast.hpp
    MeshRaycast.cpp
    TriangleMeshDeal.cpp
    TriangleMeshDeal.hpp
    TriangleMeshSlicer.cpp
    Triangulation.cpp
    Triangulation.hpp
    TriangleMeshSlicer.hpp
    MeshSplitImpl.hpp
    TriangulateWall.hpp
    TriangulateWall.cpp
    utils.cpp
    Utils.hpp
    Time.cpp
    Time.hpp
    Timer.cpp
    Timer.hpp
    Thread.cpp
    Thread.hpp
    TriangleSelector.cpp
    TriangleSelector.hpp
    TriangleSetSampling.cpp
    TriangleSetSampling.hpp
    MTUtils.hpp
    VariableWidth.cpp
    VariableWidth.hpp
    Zipper.hpp
    Zipper.cpp
    MinAreaBoundingBox.hpp
    MinAreaBoundingBox.cpp
    miniz_extension.hpp
    miniz_extension.cpp
    MarchingSquares.hpp
    Execution/Execution.hpp
    Execution/ExecutionSeq.hpp
    Execution/ExecutionTBB.hpp
    Optimize/Optimizer.hpp
    Optimize/NLoptOptimizer.hpp
    Optimize/BruteforceOptimizer.hpp
    SLA/IndexedMesh.hpp
    SLA/IndexedMesh.cpp
    Arachne/BeadingStrategy/BeadingStrategy.hpp
    Arachne/BeadingStrategy/BeadingStrategy.cpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.hpp
    Arachne/BeadingStrategy/BeadingStrategyFactory.cpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.hpp
    Arachne/BeadingStrategy/DistributedBeadingStrategy.cpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.hpp
    Arachne/BeadingStrategy/LimitedBeadingStrategy.cpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.hpp
    Arachne/BeadingStrategy/OuterWallInsetBeadingStrategy.cpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.hpp
    Arachne/BeadingStrategy/RedistributeBeadingStrategy.cpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.hpp
    Arachne/BeadingStrategy/WideningBeadingStrategy.cpp
    Arachne/BeadingStrategy/OuterWallContourStrategy.hpp
    Arachne/BeadingStrategy/OuterWallContourStrategy.cpp
    Arachne/utils/ExtrusionJunction.hpp
    Arachne/utils/ExtrusionJunction.cpp
    Arachne/utils/ExtrusionLine.hpp
    Arachne/utils/ExtrusionLine.cpp
    Arachne/utils/HalfEdge.hpp
    Arachne/utils/HalfEdgeGraph.hpp
    Arachne/utils/HalfEdgeNode.hpp
    Arachne/utils/SparseGrid.hpp
    Arachne/utils/SparsePointGrid.hpp
    Arachne/utils/SparseLineGrid.hpp
    Arachne/utils/SquareGrid.hpp
    Arachne/utils/SquareGrid.cpp
    Arachne/utils/PolygonsPointIndex.hpp
    Arachne/utils/PolygonsSegmentIndex.hpp
    Arachne/utils/PolylineStitcher.hpp
    Arachne/utils/PolylineStitcher.cpp
    Arachne/SkeletalTrapezoidation.hpp
    Arachne/SkeletalTrapezoidation.cpp
    Arachne/SkeletalTrapezoidationEdge.hpp
    Arachne/SkeletalTrapezoidationGraph.hpp
    Arachne/SkeletalTrapezoidationGraph.cpp
    Arachne/SkeletalTrapezoidationJoint.hpp
    Arachne/WallToolPaths.hpp
    Arachne/WallToolPaths.cpp
    RegionExpansion.hpp
    RegionExpansion.cpp
    ClipperZUtils.hpp
    FlushVolPredictor.hpp
    FlushVolPredictor.cpp
    FilamentGroup.hpp
    FilamentGroup.cpp
    FilamentGroupUtils.hpp
    FilamentGroupUtils.cpp
    GCode/ToolOrderUtils.hpp
    GCode/ToolOrderUtils.cpp
    Interlocking/InterlockingGenerator.hpp
    Interlocking/InterlockingGenerator.cpp
    Interlocking/VoxelUtils.hpp
    Interlocking/VoxelUtils.cpp
    MultiNozzleUtils.hpp
    MultiNozzleUtils.cpp
    FilamentMixer.hpp
    FilamentMixer.cpp
    FilamentMixerModel.hpp
    ColorDecomposeRecipe.hpp
    ColorDecomposeRecipe.cpp
)

if (APPLE)
    list(APPEND lisbslic3r_sources
			MacUtils.mm
            Format/ModelIO.hpp
            Format/ModelIO.mm
        )
endif ()

add_library(libslic3r STATIC ${lisbslic3r_sources}
    "${CMAKE_CURRENT_BINARY_DIR}/libslic3r_version.h"
    ${OpenVDBUtils_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${lisbslic3r_sources})


if(MSVC AND MSVC_VERSION GREATER_EQUAL 1950 AND NOT DEFINED OpenCV_RUNTIME)
    # OpenCV 4.6.0 doesn't recognize MSVC 19.5x (VS2026). Pick the installed runtime
    # layout under CMAKE_PREFIX_PATH; vc16/vc17 static libs are ABI-compatible.
    foreach(_opencv_runtime vc17 vc16)
        foreach(_prefix IN LISTS CMAKE_PREFIX_PATH)
            if(EXISTS "${_prefix}/x64/${_opencv_runtime}/staticlib/OpenCVConfig.cmake")
                set(OpenCV_RUNTIME ${_opencv_runtime})
                break()
            endif()
        endforeach()
        if(DEFINED OpenCV_RUNTIME)
            break()
        endif()
    endforeach()
    if(NOT DEFINED OpenCV_RUNTIME)
        set(OpenCV_RUNTIME vc17)
    endif()
endif()
if(MSVC AND NOT DEFINED OpenCV_ARCH)
    # OpenCVConfig.cmake's arch dispatcher matches CMAKE_GENERATOR_PLATFORM against
    # "ARM64" case-sensitively, but build_win.bat passes -A arm64 (lowercase), so it
    # falls back to x64 and misses the ARM64/<runtime>/staticlib install layout.
    # Select the arch explicitly for any spelling of an ARM64 target. The dispatcher
    # only honours pre-set values when BOTH OpenCV_ARCH and OpenCV_RUNTIME are
    # defined, so pin the runtime too (our deps build installs the vc17 layout).
    set(_slic3r_gen_platform "${CMAKE_GENERATOR_PLATFORM}")
    if(NOT _slic3r_gen_platform)
        set(_slic3r_gen_platform "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    string(TOUPPER "${_slic3r_gen_platform}" _slic3r_gen_platform)
    if(_slic3r_gen_platform MATCHES "^(ARM64|AARCH64)$")
        set(OpenCV_ARCH ARM64)
        if(NOT DEFINED OpenCV_RUNTIME)
            set(OpenCV_RUNTIME vc17)
        endif()
    elseif(_slic3r_gen_platform STREQUAL "ARM64EC")
        # ARM64EC consumes either dependency flavor (EC and x64 code
        # interoperate): the in-repo ARM64EC deps bundle installs OpenCV under
        # ARM64/vc17 (its recipe treats EC like arm64), while an x64 deps
        # prefix uses x64/vc17. Pick whichever layout the prefix provides.
        # MSVC 19.4x also falls in a gap in OpenCV 4.6's runtime table, so the
        # runtime must be pinned here too.
        set(OpenCV_ARCH x64)
        foreach(_slic3r_prefix ${CMAKE_PREFIX_PATH})
            if(EXISTS "${_slic3r_prefix}/ARM64/vc17")
                set(OpenCV_ARCH ARM64)
            endif()
        endforeach()
        unset(_slic3r_prefix)
        if(NOT DEFINED OpenCV_RUNTIME)
            set(OpenCV_RUNTIME vc17)
        endif()
    endif()
    unset(_slic3r_gen_platform)
endif()


encoding_check(libslic3r)

target_compile_definitions(libslic3r PUBLIC -DUSE_TBB -DTBB_USE_CAPTURED_EXCEPTION=0)
target_include_directories(libslic3r PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(libslic3r PUBLIC ${EXPAT_INCLUDE_DIRS})

# Find the OCCT and related libraries.
# CMAKE_PREFIX_PATH may be a list (e.g. after sibling dep probes); do not
# concatenate "/lib/cmake/occt" onto the whole list string.
if(NOT OpenCASCADE_DIR)
    foreach(_occt_prefix IN LISTS CMAKE_PREFIX_PATH)
        if(EXISTS "${_occt_prefix}/lib/cmake/occt")
            set(OpenCASCADE_DIR "${_occt_prefix}/lib/cmake/occt")
            break()
        endif()
    endforeach()
    unset(_occt_prefix)
endif()
target_link_libraries(libslic3r
    noise::noise
    libnest2d
    admesh
    cereal
    libigl
    miniz
    boost_libs
    clipper
    nowide
    ${EXPAT_LIBRARIES}
    glu-libtess
    qhull
    semver
    TBB::tbb
    TBB::tbbmalloc
    OpenSSL::Crypto
    ${CMAKE_DL_LIBS}
    PNG::PNG
    ZLIB::ZLIB
    Clipper2
    mcut
    )

if(NOT WIN32)
    find_package(Freetype REQUIRED)
    target_link_libraries(libslic3r Freetype::Freetype)
    if (NOT APPLE)
        target_link_libraries(libslic3r fontconfig)
    endif()
endif()

if (APPLE)
    find_library(FOUNDATION Foundation REQUIRED)
    find_library(MODELIO ModelIO REQUIRED)
    target_link_libraries(libslic3r ${FOUNDATION} ${MODELIO})
endif ()

if (OpenVDB_FOUND AND TARGET OpenVDB::openvdb)
    target_link_libraries(libslic3r OpenVDB::openvdb)
endif()

if(WIN32)
    target_link_libraries(libslic3r Psapi.lib)
endif()

if(SLIC3R_PROFILE)
    target_link_libraries(libslic3r Shiny)
endif()

if (SLIC3R_PCH AND NOT SLIC3R_SYNTAXONLY)
    add_precompiled_header(libslic3r pchheader.hpp FORCEINCLUDE)
endif ()
