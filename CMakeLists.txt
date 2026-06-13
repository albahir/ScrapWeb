cmake_minimum_required(VERSION 3.19)
project(ProyectoScrapWeb LANGUAGES CXX)

# Quitamos LinguistTools por ahora para evitar el error de traducciones
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network)

qt_standard_project_setup()

# 1. ARCHIVOS DE PRESENTACIÓN
set(CAPA_PRESENTACION
    CapaPresentacion/VentanaPrincipal.cpp
    CapaPresentacion/VentanaPrincipal.h
    CapaPresentacion/adaptadorgrafoarbol.h
    CapaPresentacion/adaptadorgrafoarbol.cpp
)

# 2. ARCHIVOS DE NEGOCIO
set(CAPA_NEGOCIO
    CapaNegocio/GrafoWeb.cpp
    CapaNegocio/GrafoWeb.h
    CapaNegocio/RastreadorWeb.cpp
    CapaNegocio/RastreadorWeb.h
    CapaNegocio/IndiceInvertido.h
    CapaNegocio/IndiceInvertido.cpp
    CapaNegocio/MetricasSitio.h
    CapaNegocio/AnalizadorMetricas.cpp
)

# 3. ARCHIVOS DE DATOS (Corregido el error de sintaxis)
set(CAPA_DATOS
    CapaDatos/gestorArchivos.cpp
    CapaDatos/gestorArchivos.h
)

# 4. CREACIÓN DEL EJECUTABLE
qt_add_executable(ProyectoScrapWeb
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${CAPA_PRESENTACION}
    ${CAPA_NEGOCIO}
    ${CAPA_DATOS}
)

# 5. INCLUSIÓN DE DIRECTORIOS
target_include_directories(ProyectoScrapWeb PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaPresentacion
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaNegocio
    ${CMAKE_CURRENT_SOURCE_DIR}/CapaDatos
)

# Traducciones desactivadas hasta que compile la base
# qt_add_translations(TARGETS ProyectoScrapWeb TS_FILES ProyectoScrapWeb_es_419.ts)

# 6. LIBRERÍAS
target_link_libraries(ProyectoScrapWeb
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)
