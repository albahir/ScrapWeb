#ifndef ANALIZADORMETRICAS_H
#define ANALIZADORMETRICAS_H

#include "GrafoWeb.h"
#include "MetricasSitio.h"

class AnalizadorMetricas {
public:
    //analiza la estructura del grafo en un instante de tiempo dado

    static MetricasSitio generarReporte(const GrafoWeb* grafo, const QString& tiempo, qint64 bytesTotales);



};


#endif // ANALIZADORMETRICAS_H
