// Archivo: CapaNegocio/MetricaSitio.h
#ifndef METRICASSITIO_H
#define METRICASSITIO_H

#include <QString>

/**
 * @struct MetricasSitio
 * @brief Estructura de datos que almacena los resultados del análisis estadístico de un sitio web.
 * @details Agrupa métricas de rendimiento de red, dimensiones del grafo de adyacencia y
 * hallazgos topológicos específicos como páginas sumidero o nodos de alta conectividad.
 */
struct MetricasSitio {
    /** @brief Tiempo total transcurrido durante la ejecución del rastreo (formato "MM:SS" o similar). */
    QString tiempoEjecucion = "00:00:00";

    /** @brief Cantidad total de páginas web únicas descubiertas (nodos del grafo). */
    int paginasEncontradas = 0;

    /** @brief Cantidad total de hipervínculos válidos detectados (aristas del grafo). */
    int enlacesDetectados = 0;

    /** @brief Densidad promedio de conexiones en la red, calculada como la razón aristas/nodos. */
    double densidadConexiones = 0.0;

    /** @brief Cadena formateada que representa el tamaño acumulado de los datos descargados (ej. "1.20 MB"). */
    QString tamanoTotal = "0 KB";

    /** @brief Cantidad de páginas que no contienen enlaces salientes hacia otros recursos (Dead Ends). */
    int paginasSumidero = 0;

    /** @brief URL de la página web que posee el mayor número de enlaces salientes (mayor Out-degree). */
    QString paginaMasConectada = "Ninguna";

    /** @brief URL de la página web que es más referenciada por otras páginas internas (mayor In-degree). */
    QString paginaMasReferenciada = "Ninguna";

    /** @brief Número máximo de enlaces entrantes que ha recibido la página más referenciada. */
    int maxEnlacesRecibidos = 0;
};

#endif // METRICASITIO_H