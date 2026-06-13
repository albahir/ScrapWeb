#ifndef ANALIZADORMETRICAS_H
#define ANALIZADORMETRICAS_H

#include "GrafoWeb.h"
#include "MetricasSitio.h"

/**
 * @class AnalizadorMetricas
 * @brief Clase de utilidad encargada del análisis estadístico y cálculo de métricas del grafo de red.
 * @details Procesa de forma estática la topología estructurada del grafo web junto con los parámetros
 * globales de rendimiento de red para consolidar un reporte unificado del sitio rastreador.
 */
class AnalizadorMetricas {
public:
    //analiza la estructura del grafo en un instante de tiempo dado

    /**
     * @brief Analiza la estructura del grafo en un instante de tiempo dado y genera un objeto de métricas unificado.
     * @details Evalúa la densidad de conexiones, el volumen de datos transferidos y el tiempo acumulado de ejecución
     * para empaquetar un diagnóstico de rendimiento estructural del sitio web explorado.
     * @param grafo Puntero constante al objeto GrafoWeb que almacena la topología de la red de URLs.
     * @param tiempo Cadena de texto que representa el tiempo total transcurrido en el rastreo.
     * @param bytesTotales Número entero de 64 bits con el conteo acumulado de bytes descargados de la red.
     * @return MetricasSitio Objeto estructurado que contiene todos los índices de rendimiento y métricas calculadas.
     */
    static MetricasSitio generarReporte(const GrafoWeb* grafo, const QString& tiempo, qint64 bytesTotales);



};


#endif // ANALIZADORMETRICAS_H