#include "GestorArchivos.h"
#include <QDebug>

/**
 * @brief Constructor por defecto de la clase GestorArchivos.
 */
GestorArchivos::GestorArchivos() {}

/**
 * @brief Serializa la topología del grafo web y sus metadatos en un archivo de texto plano.
 * @param rutaArchivo Ubicación del archivo de destino.
 * @param grafo Instancia del GrafoWeb a guardar.
 * @param metadatos Reporte ejecutivo opcional.
 * @return true si se guardó con éxito, false en caso de error.
 */
bool GestorArchivos::guardarGrafo(const QString& rutaArchivo, const GrafoWeb& grafo, const QString& metadatos) {
    QFile archivo(rutaArchivo);
    // Intentar abrir el archivo en modo escritura de texto
    if (!archivo.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream salida(&archivo);
    // Forzar codificación UTF-8 para soportar caracteres especiales
    salida.setEncoding(QStringConverter::Utf8);

    // Escribir bloques decorativos de la cabecera
    salida << "# ==========================================================\n";
    salida << "# REPORTE DE MAPEO WEB Y ESTRUCTURA DE SITIO\n";
    salida << "# ==========================================================\n";
    if (!metadatos.isEmpty()) salida << metadatos << "\n";
    salida << "# ==========================================================\n";
    salida << "# LISTA DE ADYACENCIA (GRAFO ESTRUCTURAL)\n";
    salida << "# Formato: URL_Origen -> URL_Destino1 | URL_Destino2 | ...\n";
    salida << "# ==========================================================\n\n";

    // Obtener la estructura interna del grafo (Lista de adyacencia)
    const QHash<QString, QStringList>& estructura = grafo.obtenerEstructuraCompleta();

    // Recorrer cada nodo origen del grafo
    for (auto it = estructura.constBegin(); it != estructura.constEnd(); ++it) {
        QString urlOrigen = it.key();
        QStringList enlacesDestino = it.value();

        salida << urlOrigen;

        // Si el nodo tiene enlaces salientes, formatearlos separados por " | "
        if (!enlacesDestino.isEmpty()) {
            salida << " -> ";
            for (int i = 0; i < enlacesDestino.size(); ++i) {
                salida << enlacesDestino[i];
                if (i < enlacesDestino.size() - 1) salida << " | ";
            }
        }
        salida << "\n";
    }

    archivo.close(); // Cerrar el flujo del archivo de forma segura
    return true;
}

/**
 * @brief Reconstruye la estructura de un grafo y recupera los metadatos desde un archivo serializado.
 * @param rutaArchivo Trayectoria del archivo a leer.
 * @param grafoDestino Grafo en memoria que será poblado.
 * @param urlRaiz String de salida para la URL inicial.
 * @param tiempo String de salida para el tiempo registrado.
 * @param tamano String de salida para el peso de descarga.
 * @return true si se importó correctamente, false en caso contrario.
 */
bool GestorArchivos::cargarGrafo(const QString& rutaArchivo, GrafoWeb& grafoDestino, QString& urlRaiz, QString& tiempo, QString& tamano) {
    QFile archivo(rutaArchivo);
    // Intentar abrir el archivo en modo lectura de texto
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream entrada(&archivo);
    entrada.setEncoding(QStringConverter::Utf8);

    // Resetear/Limpiar todas las variables antes de la carga
    grafoDestino.limpiar();
    urlRaiz = "";
    tiempo = "00:00";
    tamano = "0 KB";

    // Leer el archivo línea por línea hasta el final
    while (!entrada.atEnd()) {
        QString linea = entrada.readLine().trimmed();

        // Extraer metadatos de la cabecera mediante parsing de cadenas
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

        // Ignorar líneas vacías o comentarios estándar
        if (linea.isEmpty() || linea.startsWith("#")) continue;

        // Descomponer la línea en Origen y Destinos
        QStringList partes = linea.split(" -> ");
        if (partes.isEmpty()) continue;

        // Normalizar URL de origen (remover slash final si existe)
        QString urlOrigen = partes[0].trimmed();
        if (urlOrigen.endsWith("/")) urlOrigen.chop(1);
        grafoDestino.agregarNodo(urlOrigen);

        // Fallback de seguridad por si la cabecera no definió la raíz
        if (urlRaiz.isEmpty()) urlRaiz = urlOrigen;

        // Si la línea contiene nodos de destino, procesarlos
        if (partes.size() > 1) {
            QStringList destinos = partes[1].split(" | ");
            for (const QString& urlDestino : destinos) {
                // Limpiar y normalizar cada URL de destino hijo
                QString destLimpio = urlDestino.trimmed();
                if (destLimpio.endsWith("/")) destLimpio.chop(1);

                // Insertar el enlace directo (Arista) en el grafo
                if (!destLimpio.isEmpty()) grafoDestino.agregarArista(urlOrigen, destLimpio);
            }
        }
    }

    archivo.close(); // Cerrar el archivo al finalizar el bucle
    return true;
}