#include "IndiceInvertido.h"
#include <algorithm>

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

QString IndiceInvertido::normalizarPalabra(const QString& palabra) const {
    return palabra.toLower();
}

void IndiceInvertido::limpiar() {
    tablaIndice.clear();
    mapaUrlAId.clear();
    mapaIdAUrl.clear();
}