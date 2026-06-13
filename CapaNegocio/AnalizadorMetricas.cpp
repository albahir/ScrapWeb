// Archivo: CapaNegocio/AnalizadorMetricas.cpp
#include "AnalizadorMetricas.h"

/**
 * @brief Genera un reporte estadístico detallado sobre las métricas topológicas y de rendimiento del grafo web.
 * @details Procesa de forma exhaustiva el grafo web para calcular la densidad de conexiones, el volumen de datos
 * transferidos transformado a unidades legibles (KB/MB), la cantidad de páginas sumidero (dead ends), así como la
 * identificación de los nodos con mayor grado de salida (out-degree) y de entrada (in-degree).
 * @param grafo Puntero constante a la estructura GrafoWeb que almacena la red de páginas mapeadas.
 * @param tiempo Cadena de texto formateada que representa la duración total del rastreo.
 * @param bytesTotales Cantidad acumulada de bytes descargados a través de las peticiones de red.
 * @return MetricasSitio Objeto estructurado que agrupa todos los indicadores estadísticos calculados.
 */
MetricasSitio AnalizadorMetricas::generarReporte(const GrafoWeb* grafo, const QString& tiempo, qint64 bytesTotales) {
    MetricasSitio reporte;
    reporte.tiempoEjecucion = tiempo;
    reporte.paginasEncontradas = grafo->cantidadNodos();
    reporte.enlacesDetectados = grafo->cantidadAristas();

    // Calcular Densidad de Conexiones
    if (reporte.paginasEncontradas > 0) {
        // Fórmula simple: Aristas / Nodos
        reporte.densidadConexiones = static_cast<double>(reporte.enlacesDetectados) / reporte.paginasEncontradas;
    } else {
        reporte.densidadConexiones = 0.0;
    }
    // Formatear peso de descarga
    if (bytesTotales < 1024 * 1024) {
        reporte.tamanoTotal = QString::number(bytesTotales / 1024.0, 'f', 2) + " KB";
    } else {
        reporte.tamanoTotal = QString::number(bytesTotales / (1024.0 * 1024.0), 'f', 2) + " MB";
    }

    const QHash<QString, QStringList>& estructura = grafo->obtenerEstructuraCompleta();

    // -------------------------------------------------------------
    // CÁLCULO 1: Páginas Sumidero (Dead Ends) - Sin salidas
    // -------------------------------------------------------------
    int sumideros = 0;
    for (auto it = estructura.constBegin(); it != estructura.constEnd(); ++it) {
        if (it.value().isEmpty()) {
            sumideros++;
        }
    }
    reporte.paginasSumidero = sumideros;

    // -------------------------------------------------------------
    // CÁLCULO 2: Página con más enlaces salientes (Out-degree)
    // -------------------------------------------------------------
    int maxEnlacesSalientes = 0;
    for (auto it = estructura.constBegin(); it != estructura.constEnd(); ++it) {
        if (it.value().size() > maxEnlacesSalientes) {
            maxEnlacesSalientes = it.value().size();
            reporte.paginaMasConectada = it.key();
        }
    }

    // -------------------------------------------------------------
    // CÁLCULO 3: Página más referenciada hacia ella (In-degree)
    // -------------------------------------------------------------
    QHash<QString, int> conteoEntradas;

    for (auto it = estructura.constBegin(); it != estructura.constEnd(); ++it) {
        const QStringList& destinos = it.value();
        for (const QString& urlDestino : destinos) {
            conteoEntradas[urlDestino]++;
        }
    }

    for (auto it = conteoEntradas.constBegin(); it != conteoEntradas.constEnd(); ++it) {
        if (it.value() > reporte.maxEnlacesRecibidos) {
            reporte.maxEnlacesRecibidos = it.value();
            reporte.paginaMasReferenciada = it.key();
        }
    }

    return reporte;
}