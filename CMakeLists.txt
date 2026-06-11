cmake_minimum_required(VERSION 3.19)
project(ProyectoScrapWeb LANGUAGES CXX)


find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Network)


qt_standard_project_setup()

# ========================================================================
# 1. GENERACIÓN AUTOMÁTICA DE CARPETAS DE ARQUITECTURA
# Esto crea las carpetas físicas en tu directorio de código si no existen.
# ========================================================================
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/CapaPresentacion)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/CapaNegocio)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/CapaDatos)

# ========================================================================
# 2. DEFINICIÓN DE ARCHIVOS POR CAPA
# Agrega aquí los archivos .cpp y .h a medida que los vayas creando.
# ========================================================================
set(CAPA_PRESENTACION
    CapaPresentacion/VentanaPrincipal.cpp
    CapaPresentacion/VentanaPrincipal.h
)

set(CAPA_NEGOCIO

    CapaNegocio/GrafoWeb.cpp
    CapaNegocio/GrafoWeb.h
    CapaNegocio/RastreadorWeb.cpp
    CapaNegocio/RastreadorWeb.h

)

set(CAPA_DATOS
    # Aquí irán: CapaDatos/GestorArchivos.cpp, etc.
)

# ========================================================================
# 3. CREACIÓN DEL EJECUTABLE
# Unimos el main.cpp con todos los archivos de las distintas capas.
# ========================================================================
qt_add_executable(ProyectoScrapWeb
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${CAPA_PRESENTACION}
    ${CAPA_NEGOCIO}
    ${CAPA_DATOS}

    CapaNegocio/GrafoWeb.h
    CapaNegocio/GrafoWeb.cpp
    CapaNegocio/RastreadorWeb.h
    CapaNegocio/RastreadorWeb.cpp
    CapaNegocio/IndiceInvertido.h
    CapaNegocio/IndiceInvertido.cpp
    CapaDatos/gestorArchivos.h
    CapaDatos/gestorArchivos.cpp
    CapaPresentacion/adaptadorgrafoarbol.h CapaPresentacion/adaptadorgrafoarbol.cpp
    CapaNegocio/MetricasSitio.h
    CapaNegocio/AnalizadorMetricas.cpp

)

# ========================================================================
# 4. INCLUSIÓN DE DIRECTORIOS (MUY IMPORTANTE)
# Esto permite hacer #include "VentanaPrincipal.h" en lugar de
# tener que escribir #include "CapaPresentacion/VentanaPrincipal.h"
# ========================================================================
target_include_directories(ProyectoScrapWeb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaPresentacion
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaNegocio
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaDatos
)

qt_add_translations(
    TARGETS ProyectoScrapWeb
    TS_FILES ProyectoScrapWeb_es_419.ts
)

target_link_libraries(ProyectoScrapWeb
    PRIVATE

        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network

)

include(GNUInstallDirs)

install(TARGETS ProyectoScrapWeb
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ProyectoScrapWeb
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
