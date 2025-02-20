include(GenerateExportHeader)

if(FAST_MODULE_TensorFlow)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleTensorFlow.cmake)

    if(FAST_BUILD_TensorFlow_CUDA)
        add_library(InferenceEngineTensorFlowCUDA SHARED TensorFlowCUDAEngine.hpp TensorFlowCUDAEngine.cpp TensorFlowEngine.hpp TensorFlowEngine.cpp)
        target_link_libraries(InferenceEngineTensorFlowCUDA FAST ${TensorFlow_CUDA_LIBRARIES})
        message("-- Linking to ${TensorFlow_CUDA_LIBRARIES}")
        target_include_directories(InferenceEngineTensorFlowCUDA PRIVATE ${FAST_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
        generate_export_header(InferenceEngineTensorFlowCUDA EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorFlowCUDAExport.hpp)
        add_dependencies(InferenceEngineTensorFlowCUDA tensorflow_CUDA)
        fast_add_inference_engine(TensorFlowCUDA)
    endif()
    if(FAST_BUILD_TensorFlow_ROCm)
        add_library(InferenceEngineTensorFlowROCm SHARED TensorFlowROCmEngine.hpp TensorFlowROCmEngine.cpp TensorFlowEngine.hpp TensorFlowEngine.cpp)
        target_link_libraries(InferenceEngineTensorFlowROCm FAST ${TensorFlow_ROCm_LIBRARIES})
        message("-- Linking to ${TensorFlow_ROCm_LIBRARIES}")
        target_include_directories(InferenceEngineTensorFlowROCm PRIVATE ${FAST_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
        generate_export_header(InferenceEngineTensorFlowROCm EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorFlowROCmExport.hpp)
        add_dependencies(InferenceEngineTensorFlowROCm tensorflow_ROCm)
        fast_add_inference_engine(TensorFlowROCm)
    endif()
    if(FAST_BUILD_TensorFlow_CPU)
        add_library(InferenceEngineTensorFlowCPU SHARED TensorFlowCPUEngine.hpp TensorFlowCPUEngine.cpp TensorFlowEngine.hpp TensorFlowEngine.cpp)
        target_link_libraries(InferenceEngineTensorFlowCPU FAST ${TensorFlow_CPU_LIBRARIES})
        message("-- Linking to ${TensorFlow_CPU_LIBRARIES}")
        target_include_directories(InferenceEngineTensorFlowCPU PRIVATE ${FAST_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
        generate_export_header(InferenceEngineTensorFlowCPU EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorFlowCPUExport.hpp)
        add_dependencies(InferenceEngineTensorFlowCPU tensorflow_CPU)
        fast_add_inference_engine(TensorFlowCPU)
    endif()
    if(NOT FAST_BUILD_TensorFlow_CPU AND NOT FAST_BUILD_TensorFlow_CUDA AND NOT FAST_BUILD_TensorFlow_ROCm)
        message(ERROR "You selected the FAST module TensorFlow, but no engines to build, see FAST_BUILD_TensorFlow_CUDA, FAST_BUILD_TensorFlow_CPU, FAST_BUILD_TensorFlow_ROCm")
    endif()

endif()
if(FAST_MODULE_TensorRT)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleTensorRT.cmake)

    add_library(InferenceEngineTensorRT SHARED TensorRTEngine.hpp TensorRTEngine.cpp)
    target_include_directories(InferenceEngineTensorRT PRIVATE ${FAST_INCLUDE_DIRS} ${TensorRT_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS})
    target_link_libraries(InferenceEngineTensorRT FAST ${TensorRT_LIBRARIES})
    generate_export_header(InferenceEngineTensorRT EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/TensorRTExport.hpp)

    fast_add_inference_engine(TensorRT)
endif()
if(FAST_MODULE_OpenVINO)
    include(${PROJECT_SOURCE_DIR}/cmake/ModuleOpenVINO.cmake)

    add_library(InferenceEngineOpenVINO SHARED OpenVINOEngine.hpp OpenVINOEngine.cpp)
    target_include_directories(InferenceEngineOpenVINO PRIVATE ${FAST_INCLUDE_DIRS} ${InferenceEngine_INCLUDE_DIRS})
    target_link_libraries(InferenceEngineOpenVINO FAST ${InferenceEngine_LIBRARIES} ${IntelTBB})
    generate_export_header(InferenceEngineOpenVINO EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/OpenVINOExport.hpp)
    add_dependencies(InferenceEngineOpenVINO IE::ie_cpu_extension)

    fast_add_inference_engine(OpenVINO)
endif()
