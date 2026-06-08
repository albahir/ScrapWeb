#ifndef GESTORARCHIVOS_H
#define GESTORARCHIVOS_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include "../CapaNegocio/GrafoWeb.h"

class GestorArchivos {
public:
    GestorArchivos();


    static bool guardarGrafo(const QString& rutaArchivo, const GrafoWeb& grafo, const QString& metadatos = "");


    static bool cargarGrafo(const QString& rutaArchivo, GrafoWeb& grafoDestino, QString& urlRaiz);
};

#endif // GESTORARCHIVOS_H
