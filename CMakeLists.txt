if(COMMAND idf_component_register)
    idf_component_register(INCLUDE_DIRS "src")
elseif(COMMAND register_component)
    set(COMPONENT_SRCDIRS "src")
    set(COMPONENT_ADD_INCLUDEDIRS "src")
    register_component()
else()
    add_library(ESPressio-System INTERFACE)
    target_include_directories(ESPressio-System INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    add_library(ESPressio::System ALIAS ESPressio-System)
endif()
