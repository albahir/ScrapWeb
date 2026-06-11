#include "GrafoWeb.h"
#include "QQueue"
#include "QSet"
GrafoWeb::GrafoWeb() {
    totalAristas = 0;
}

void GrafoWeb::agregarNodo(const QString& url) {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    // Si la URL limpia no existe, la insertamos
    if (!listaAdyacencia.contains(urlLimpia)) {
        listaAdyacencia.insert(urlLimpia, QStringList());
    }


}

void GrafoWeb::agregarArista(const QString& urlOrigen, const QString& urlDestino) {
    // Nos aseguramos de que el nodo de origen exista
    agregarNodo(urlOrigen);
    // Asegurarnos de que el destino también sea un nodo reconocido en el grafo
    agregarNodo(urlDestino);

    QString urlLimpia = urlOrigen;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }
    QString urlLimpia2 = urlDestino;
    if (urlLimpia2.endsWith("/")) {
        urlLimpia2.chop(1);
    }

    // Evitamos agregar enlaces duplicados desde la misma página
    if (!listaAdyacencia[urlLimpia].contains(urlLimpia2)) {
        listaAdyacencia[urlLimpia].append(urlLimpia2);
        totalAristas++;
    }
}

bool GrafoWeb::contieneNodo(const QString& url) const {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    return listaAdyacencia.contains(urlLimpia);
}

QStringList GrafoWeb::obtenerAdyacentes(const QString& url) const {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    if (listaAdyacencia.contains(urlLimpia)) {
        return listaAdyacencia.value(urlLimpia);
    }
    return QStringList();
}

QList<QString> GrafoWeb::obtenerTodosLosNodos() const {
    return listaAdyacencia.keys();
}

void GrafoWeb::limpiar() {
    listaAdyacencia.clear();
    totalAristas = 0;
}

int GrafoWeb::cantidadNodos() const {
    return listaAdyacencia.size();
}

int GrafoWeb::cantidadAristas() const {
    return totalAristas;
}

const QHash<QString, QStringList>& GrafoWeb::obtenerEstructuraCompleta() const {
    return listaAdyacencia;
}

void GrafoWeb::calcularRutasDesdeRaiz(const QString& urlOrigen,
                                      QHash<QString, int>& distancias,
                                      QHash<QString, QString>& padres) const {
    distancias.clear();
    padres.clear();

    QString origenLimpio = urlOrigen;
    if (origenLimpio.endsWith("/")) origenLimpio.chop(1);

    if (!listaAdyacencia.contains(origenLimpio)) return;

    QQueue<QString> cola;
    cola.enqueue(origenLimpio);
    distancias.insert(origenLimpio, 0);

    while (!cola.isEmpty()) {
        QString actual = cola.dequeue();
        int distActual = distancias.value(actual);

        QStringList adyacentes = listaAdyacencia.value(actual);
        for (const QString& vecino : adyacentes) {
            if (!distancias.contains(vecino)) {
                distancias.insert(vecino, distActual + 1);
                padres.insert(vecino, actual); // Guardamos quién es el predecesor directo
                cola.enqueue(vecino);
            }
        }
    }
}
QStringList GrafoWeb::reconstruirRuta(const QString& urlOrigen, const QString& urlDestino, const QHash<QString, QString>& padres) const {
    QStringList ruta;
    QString pasoActual = urlDestino;

    // Navegamos hacia atrás usando el diccionario de padres
    while (pasoActual != urlOrigen && padres.contains(pasoActual)) {
        ruta.prepend(pasoActual);
        pasoActual = padres.value(pasoActual);
    }

    // Validamos que realmente hayamos llegado a la raíz
    if (pasoActual == urlOrigen) {
        ruta.prepend(urlOrigen);
    } else {
        ruta.clear(); // Ruta inválida o inalcanzable
    }

    return ruta;
}
