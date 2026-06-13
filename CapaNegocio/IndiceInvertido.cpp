#include "IndiceInvertido.h"
#include <algorithm>

/**
 * @brief Constructor de la clase IndiceInvertido.
 * @details Configura las expresiones regulares encargadas de la tokenización de palabras aisladas
 * y el filtrado de etiquetas HTML estructurales, e invoca la carga del diccionario de stop-words.
 */
IndiceInvertido::IndiceInvertido() {
    regexPalabras.setPattern("\\b[a-záéíóúñ]+\\b");
    regexPalabras.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    regexEtiquetasHTML.setPattern("<[^>]*>");
    inicializarPalabrasIgnoradas();
}

/**
 * @brief Inicializa el conjunto interno de palabras vacías o stop-words.
 * @details Carga un listado predefinido de artículos, preposiciones y conectores comunes en español
 * e inglés que no aportan peso semántico, previniendo la sobrecarga de la tabla hash de indexación.
 */
void IndiceInvertido::inicializarPalabrasIgnoradas() {
    palabrasIgnoradas = {
        "que", "los", "las", "del", "por", "con", "una", "uno", "unos", "unas",
        "para", "como", "mas", "pero", "este", "esta", "sus", "sin", "sobre", "the", "and", "for"
    };
}

/**
 * @brief Tokeniza, filtra y comprime el contenido textual de una página web en la matriz del índice.
 * @details Remueve las barras diagonales de cierre de la URL, traduce la cadena de texto de la URL a un
 * identificador numérico compacto de tipo entero (ID) para ahorrar memoria, limpia las etiquetas HTML,
 * aísla las palabras válidas mediante expresiones regulares y actualiza las frecuencias de aparición.
 * @param url Dirección URL de la página web procesada.
 * @param contenidoTexto Código fuente o texto plano extraído del documento.
 */
void IndiceInvertido::indexarPagina(const QString& url, const QString& contenidoTexto) {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    // ==========================================
    // 1. TRADUCCIÓN SILENCIOSA (STRING -> ID)
    // ==========================================
    int idUrl;
    if (mapaUrlAId.contains(urlLimpia)) {
        idUrl = mapaUrlAId.value(urlLimpia);
    } else {
        idUrl = mapaIdAUrl.size(); // El índice del QVector (0, 1, 2...) será el ID
        mapaUrlAId.insert(urlLimpia, idUrl);
        mapaIdAUrl.append(urlLimpia);
    }

    QString textoLimpio = contenidoTexto;
    textoLimpio.remove(regexEtiquetasHTML);

    QRegularExpressionMatchIterator it = regexPalabras.globalMatch(textoLimpio);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString palabra = normalizarPalabra(match.captured(0));

        if (palabra.length() > 2 && !palabrasIgnoradas.contains(palabra)) {
            // Guardamos usando el NÚMERO (Ocupa 4 bytes en vez de 100+)
            tablaIndice[palabra][idUrl]++;
        }
    }
}

/**
 * @brief Ejecuta una consulta de búsqueda de término único y devuelve las coincidencias ordenadas por relevancia.
 * @details Normaliza la palabra clave, extrae el mapa interno de IDs y frecuencias, ordena de forma descendente
 * los documentos según el nivel de repetición del término empleando una función lambda y traduce de regreso
 * los IDs numéricos a sus cadenas URL correspondientes con coste de acceso $O(1)$.
 * @param palabra Término o concepto clave que se desea buscar en el corpus indexado.
 * @return QStringList Lista con las direcciones URL de las páginas coincidentes ordenadas por frecuencia.
 */
QStringList IndiceInvertido::buscar(const QString& palabra) const {
    QString palabraNormalizada = normalizarPalabra(palabra);

    if (!tablaIndice.contains(palabraNormalizada)) {
        return QStringList();
    }

    // El mapa interno ahora es de enteros
    QHash<int, int> mapaResultados = tablaIndice.value(palabraNormalizada);
    QList<int> idsOrdenados = mapaResultados.keys();

    std::sort(idsOrdenados.begin(), idsOrdenados.end(), [&mapaResultados](int a, int b) {
        return mapaResultados[a] > mapaResultados[b];
    });

    // ==========================================
    // 2. RETORNO DE TEXTO (ID -> STRING)
    // ==========================================
    QStringList urlsOrdenadas;
    for (int id : idsOrdenados) {
        urlsOrdenadas.append(mapaIdAUrl[id]); // Extracción ultrarrápida O(1) usando el vector
    }

    return urlsOrdenadas;
}

/**
 * @brief Estandariza un término lingüístico para unificar criterios de búsqueda.
 * @details Pasa la cadena de caracteres a minúsculas homogéneas.
 * @param palabra Cadena de texto original sin procesar.
 * @return QString Palabra resultante convertida a minúsculas.
 */
QString IndiceInvertido::normalizarPalabra(const QString& palabra) const {
    return palabra.toLower();
}

/**
 * @brief Vacía por completo las estructuras del índice y las tablas de traducción de IDs.
 * @details Libera la memoria RAM utilizada por los mapas hash y los vectores de direccionamiento inverso.
 */
void IndiceInvertido::limpiar() {
    tablaIndice.clear();
    mapaUrlAId.clear();
    mapaIdAUrl.clear();
}