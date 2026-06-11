// Archivo: CapaNegocio/AnalizadorMetricas.cpp
#include "AnalizadorMetricas.h"

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
