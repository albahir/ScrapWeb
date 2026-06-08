#include "IndiceInvertido.h"
#include <algorithm> // Requerido para std::sort

IndiceInvertido::IndiceInvertido() {
    regexPalabras.setPattern("\\b[a-záéíóúñ]+\\b");
    regexPalabras.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    regexEtiquetasHTML.setPattern("<[^>]*>");
    inicializarPalabrasIgnoradas();
}

void IndiceInvertido::inicializarPalabrasIgnoradas() {
    palabrasIgnoradas = {
        "que", "los", "las", "del", "por", "con", "una", "uno", "unos", "unas",
        "para", "como", "mas", "pero", "este", "esta", "sus", "sin", "sobre", "the", "and", "for"
    };
}

void IndiceInvertido::indexarPagina(const QString& url, const QString& contenidoTexto) {
    // 1. Blindaje de URL
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    // 2. Limpieza de HTML
    QString textoLimpio = contenidoTexto;
    textoLimpio.remove(regexEtiquetasHTML);

    // 3. Extracción de palabras
    QRegularExpressionMatchIterator it = regexPalabras.globalMatch(textoLimpio);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString palabra = normalizarPalabra(match.captured(0));

        if (palabra.length() > 2 && !palabrasIgnoradas.contains(palabra)) {
            // Guardamos usando siempre la URL limpia
            tablaIndice[palabra][urlLimpia]++;
        }
    }
}

QStringList IndiceInvertido::buscar(const QString& palabra) const {
    QString palabraNormalizada = normalizarPalabra(palabra);

    // Si la palabra no está en el índice global, devolvemos una lista vacía
    if (!tablaIndice.contains(palabraNormalizada)) {
        return QStringList();
    }

    // Obtenemos el diccionario interno de: URL -> Frecuencia
    QHash<QString, int> mapaResultados = tablaIndice.value(palabraNormalizada);

    // Extraemos solo las URLs para poder ordenarlas
    QStringList urlsOrdenadas = mapaResultados.keys();

    // Ordenamiento por relevancia (Frecuencia descendente) usando una función Lambda
    std::sort(urlsOrdenadas.begin(), urlsOrdenadas.end(), [&mapaResultados](const QString& a, const QString& b) {
        return mapaResultados[a] > mapaResultados[b];
    });

    return urlsOrdenadas;
}

QString IndiceInvertido::normalizarPalabra(const QString& palabra) const {
    return palabra.toLower();
}

void IndiceInvertido::limpiar() {
    tablaIndice.clear();
}