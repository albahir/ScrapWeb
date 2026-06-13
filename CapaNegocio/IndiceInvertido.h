#ifndef INDICEINVERTIDO_H
#define INDICEINVERTIDO_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QRegularExpression>
#include <QVector> //    Para acceso ultra rápido por ID

/**
 * @class IndiceInvertido
 * @brief Clase encargada de mapear, indexar y recuperar términos extraídos de las páginas web.
 * @details Implementa un motor de búsqueda local basado en la estructura de Índice Invertido.
 * Asocia palabras clave optimizadas con los identificadores únicos de los documentos (URLs)
 * donde aparecen, registrando además su frecuencia para permitir futuras funciones de ordenamiento.
 */
class IndiceInvertido {
public:
    /**
     * @brief Constructor de la clase IndiceInvertido.
     * @details Inicializa las expresiones regulares de filtrado y carga el listado de stop-words.
     */
    IndiceInvertido();

    /**
     * @brief Tokeniza, filtra e indexa el contenido textual de una página web específica.
     * @details Limpia el HTML residual, extrae palabras válidas, remueve términos vacíos (stop-words),
     * normaliza el texto y actualiza la tabla hash de frecuencias asociándola al ID único de la URL.
     * @param url Dirección web del documento que se va a indexar.
     * @param contenidoTexto Código fuente o texto plano extraído de la página.
     */
    void indexarPagina(const QString& url, const QString& contenidoTexto);

    /**
     * @brief Recupera el listado de URLs que contienen la palabra consultada.
     * @details Aplica la normalización al término de búsqueda y consulta el índice mapeando de
     * regreso los IDs internos a cadenas de texto de URLs legibles.
     * @param palabra Término o concepto clave que se desea buscar.
     * @return QStringList Lista con las direcciones URL de las páginas coincidentes.
     */
    QStringList buscar(const QString& palabra) const;

    /**
     * @brief Vacía por completo el índice invertido y restablece los mapas bidireccionales de URLs.
     */
    void limpiar();

private:
    // (ID_URL -> Frecuencia)
    QHash<QString, QHash<int, int>> tablaIndice;

    // DICCIONARIOS INTERNOS PRIVADOS
    QHash<QString, int> mapaUrlAId;
    QVector<QString> mapaIdAUrl;

    QSet<QString> palabrasIgnoradas;
    QRegularExpression regexPalabras;
    QRegularExpression regexEtiquetasHTML;

    /**
     * @brief Normaliza un término transformándolo a minúsculas y removiendo caracteres diacríticos.
     * @param palabra Cadena de texto original sin procesar.
     * @return QString Cadena normalizada limpia de acentos y estandarizada.
     */
    QString normalizarPalabra(const QString& palabra) const;

    /**
     * @brief Carga el conjunto de palabras ignoradas (stop-words) de uso común en el idioma.
     */
    void inicializarPalabrasIgnoradas();
};

#endif // INDICEINVERTIDO_H
