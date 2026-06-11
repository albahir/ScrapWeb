// Archivo: CapaNegocio/MetricaSitio.h
#ifndef METRICASSITIO_H
#define METRICASSITIO_H

#include <QString>

struct MetricasSitio {
    QString tiempoEjecucion = "00:00:00";
    int paginasEncontradas = 0;
    int enlacesDetectados = 0;
    double densidadConexiones = 0.0;
    QString tamanoTotal = "0 KB";

    int paginasSumidero = 0;


    QString paginaMasConectada = "Ninguna";


    QString paginaMasReferenciada = "Ninguna";
    int maxEnlacesRecibidos = 0;
};

#endif // METRICASITIO_H
