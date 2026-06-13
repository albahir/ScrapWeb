#ifndef GESTORARCHIVOS_H
#define GESTORARCHIVOS_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include "../CapaNegocio/GrafoWeb.h"

/**
 * @class GestorArchivos
 * @brief Clase de la capa de persistencia encargada de la exportación e importación del grafo web.
 * @details Proporciona funciones estáticas para serializar la topología del grafo y sus metadatos asociados
 * hacia archivos de texto plano, así como para reconstruir el estado de la aplicación desde un archivo guardado.
 */
class GestorArchivos {
public:
    /**
     * @brief Constructor por defecto de la clase GestorArchivos.
     */
    GestorArchivos();

    /**
     * @brief Serializa y guarda la estructura completa del grafo web en un archivo local.
     * @details Escribe de forma formateada las relaciones de adyacencia (nodos y aristas) junto con
     * una cabecera opcional de metadatos de rendimiento del rastreo.
     * @param rutaArchivo Ruta absoluta o relativa del disco donde se creará el archivo.
     * @param grafo Referencia constante al objeto GrafoWeb que se desea respaldar.
     * @param metadatos Cadena opcional que contiene información estadística para adjuntar al archivo (ej. tiempo, tamaño).
     * @return true si el archivo se abrió y escribió correctamente, false en caso de fallo de E/S.
     */
    static bool guardarGrafo(const QString& rutaArchivo, const GrafoWeb& grafo, const QString& metadatos = "");

    /**
     * @brief Carga y reconstruye un grafo web previamente serializado desde un archivo local.
     * @details Limpia el grafo de destino, lee la estructura de adyacencia guardada línea por línea
     * e intercepta los metadatos de la cabecera para devolver el estado original del rastreo.
     * @param rutaArchivo Ruta del archivo de texto que se va a leer.
     * @param grafoDestino Referencia al objeto GrafoWeb donde se inyectará la topología reconstruida.
     * @param urlRaiz Parámetro de salida que recibirá la URL inicial del rastreo recuperada.
     * @param tiempo Parámetro de salida que recibirá el tiempo de ejecución histórico registrado.
     * @param tamano Parámetro de salida que recibirá el peso total de descarga histórico registrado.
     * @return true si el archivo fue interpretado y procesado con éxito, false si el archivo no existe o está corrupto.
     */
    static bool cargarGrafo(const QString& rutaArchivo, GrafoWeb& grafoDestino, QString& urlRaiz, QString& tiempo, QString& tamano);
};

#endif // GESTORARCHIVOS_H