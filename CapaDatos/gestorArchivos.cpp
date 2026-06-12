#include "GestorArchivos.h"
#include <QDebug>

GestorArchivos::GestorArchivos() {}

bool GestorArchivos::guardarGrafo(const QString& rutaArchivo, const GrafoWeb& grafo, const QString& metadatos) {
    QFile archivo(rutaArchivo);
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream salida(&archivo);
    salida.setEncoding(QStringConverter::Utf8);

    // Escribimos el reporte ejecutivo
    salida << "# ==========================================================\n";
    salida << "# REPORTE DE MAPEO WEB Y ESTRUCTURA DE SITIO\n";
    salida << "# ==========================================================\n";
    if (!metadatos.isEmpty()) salida << metadatos << "\n";
    salida << "# ==========================================================\n";
    salida << "# LISTA DE ADYACENCIA (GRAFO ESTRUCTURAL)\n";
    salida << "# Formato: URL_Origen -> URL_Destino1 | URL_Destino2 | ...\n";
    salida << "# ==========================================================\n\n";

    const QHash<QString, QStringList>& estructura = grafo.obtenerEstructuraCompleta();

    for (auto it = estructura.constBegin(); it != estructura.constEnd(); ++it) {
        QString urlOrigen = it.key();
        QStringList enlacesDestino = it.value();
        salida << urlOrigen;
        if (!enlacesDestino.isEmpty()) {
            salida << " -> ";
            for (int i = 0; i < enlacesDestino.size(); ++i) {
                salida << enlacesDestino[i];
                if (i < enlacesDestino.size() - 1) salida << " | ";
            }
        }
        salida << "\n";
    }
    archivo.close();
    return true;
}

bool GestorArchivos::cargarGrafo(const QString& rutaArchivo, GrafoWeb& grafoDestino, QString& urlRaiz, QString& tiempo, QString& tamano) {
    QFile archivo(rutaArchivo);
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream entrada(&archivo);
    entrada.setEncoding(QStringConverter::Utf8);
    grafoDestino.limpiar();
    urlRaiz = "";
    tiempo = "00:00";
    tamano = "0 KB";

    while (!entrada.atEnd()) {
        QString linea = entrada.readLine().trimmed();

        if (linea.startsWith("# URL Raíz")) {
            urlRaiz = linea.section(':', 1).trimmed();
            continue;
        }
        if (linea.startsWith("# Tamaño del contenido")) {
            tamano = linea.section(':', 1).trimmed();
            continue;
        }
        if (linea.startsWith("# Tiempo de ejecución")) {
            tiempo = linea.section(':', 1).trimmed();
            continue;
        }

        if (linea.isEmpty() || linea.startsWith("#")) continue;

        QStringList partes = linea.split(" -> ");
        if (partes.isEmpty()) continue;

        QString urlOrigen = partes[0].trimmed();
        if (urlOrigen.endsWith("/")) urlOrigen.chop(1);
        grafoDestino.agregarNodo(urlOrigen);

        if (urlRaiz.isEmpty()) urlRaiz = urlOrigen; // Fallback

        if (partes.size() > 1) {
            QStringList destinos = partes[1].split(" | ");
            for (const QString& urlDestino : destinos) {
                QString destLimpio = urlDestino.trimmed();
                if (destLimpio.endsWith("/")) destLimpio.chop(1);
                if (!destLimpio.isEmpty()) grafoDestino.agregarArista(urlOrigen, destLimpio);
            }
        }
    }
    archivo.close();
    return true;
}
