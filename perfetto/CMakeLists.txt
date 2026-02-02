macro(func_link_libaries target)
    target_link_libraries(${target} PUBLIC MLLM_CPU fmt::fmt-header-only)
    if (MLLM_OPENMP)
        target_compile_options(${target} PRIVATE -fopenmp)
        if (ARM)
            target_link_libraries(${target} PUBLIC -fopenmp -static-openmp)
        else ()
            target_link_libraries(${target} PUBLIC -fopenmp)
        endif ()
    endif ()
    if (QNN)
        target_compile_definitions(${target} PRIVATE USE_QNN)
        target_link_libraries(${target} PUBLIC MLLM_QNN ${CMAKE_DL_LIBS})
    endif()
    if (MLLM_BUILD_XNNPACK_BACKEND)
        target_link_libraries(${target} PRIVATE MllmXnnpackBackend)
    endif()
    if(PERFETTO)
        target_link_libraries(${target} PRIVATE perfetto log)
        target_compile_definitions(perfetto PUBLIC PERFETTO_IMPLEMENTATION=1)
    endif()
endmacro()

macro(func_llm_add_executable target)
    add_executable(${target}
        ${PROJECT_SOURCE_DIR}/perfetto/${target}.cpp 
        ${DIR_SRC_CPU} 
        ${DIR_SRC_MEM_MANAGER} 
        ${DIR_SRC_EXP} 
        ${DIR_SRC}
        ${PROJECT_SOURCE_DIR}/src/tokenizers/Tokenizer.cpp
        ${PROJECT_SOURCE_DIR}/src/tokenizers/BPE/Bpe.cpp
        ${PROJECT_SOURCE_DIR}/src/tokenizers/WordPiece/WordPiece.cpp
        ${PROJECT_SOURCE_DIR}/src/tokenizers/Tiktoken/tiktoken.cpp
        ${PROJECT_SOURCE_DIR}/src/tokenizers/Unicode.cpp
        ${PROJECT_SOURCE_DIR}/src/tokenizers/UnicodeData.cpp
        ${PROJECT_SOURCE_DIR}/src/processor/PreProcess.cpp
        ${PROJECT_SOURCE_DIR}/src/utils/json.hpp
        ${PROJECT_SOURCE_DIR}/src/hardware/device.cpp
        ${PROJECT_SOURCE_DIR}/src/hardware/dvfs.cpp
        ${PROJECT_SOURCE_DIR}/src/hardware/record.cpp
        ${PROJECT_SOURCE_DIR}/src/hardware/utils.cpp
        ${PROJECT_SOURCE_DIR}/src/common/common.h
    )
    func_link_libaries(${target})
    if (IGNITE_USE_SYSTEM)
        target_compile_definitions(${target} PUBLIC IGNITE_USE_SYSTEM=1)
    else()
        target_compile_definitions(${target} PUBLIC IGNITE_USE_SYSTEM=0)
    endif()
endmacro()

func_llm_add_executable(qwen3_perf)