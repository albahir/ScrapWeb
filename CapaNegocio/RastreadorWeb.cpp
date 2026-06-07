#include "RastreadorWeb.h"
#include <QDebug>

RastreadorWeb::RastreadorWeb(GrafoWeb* grafo, QObject *parent)
    : QObject(parent), grafoEnMemoria(grafo) {
    networkManager = new QNetworkAccessManager(this);
    rastreoActivo = false;
    peticionesActivas = 0;

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &RastreadorWeb::alTerminarDescarga);
}

void RastreadorWeb::iniciarRastreo(const QString& urlInicial, int profundidadMax) {
    colaPendientes.clear();
    urlsVisitadas.clear();
    grafoEnMemoria->limpiar();

    limiteProfundidad = profundidadMax;
    rastreoActivo = true;

    QUrl urlDecodificada(urlInicial);
    if (!urlDecodificada.isValid()) {
        emit error("La URL inicial no es válida.");
        return;
    }
    dominioObjetivo = urlDecodificada.host();

    colaPendientes.enqueue({urlInicial, 0});
    procesarSiguiente();
}

void RastreadorWeb::detenerRastreo() {
    rastreoActivo = false;
}

void RastreadorWeb::procesarSiguiente() {
    // Si ya alcanzamos el límite, no lanzamos nada más; esperamos a que termine una descarga
    while (rastreoActivo && peticionesActivas < MAX_PETICIONES_SIMULTANEAS && !colaPendientes.isEmpty()) {

        NodoRastreo nodoActual = colaPendientes.dequeue();
        if (urlsVisitadas.contains(nodoActual.url)) continue;

        urlsVisitadas.insert(nodoActual.url);
        emit enlaceDescubierto(nodoActual.url);

        if (limiteProfundidad > 0 && nodoActual.profundidad >= limiteProfundidad) continue;

        QNetworkRequest peticion((QUrl(nodoActual.url)));
        peticion.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        peticion.setAttribute(QNetworkRequest::User, nodoActual.profundidad);

        peticionesActivas++; // Incrementamos el contador
        networkManager->get(peticion);
    }

    // Si ya no hay nada en cola y no hay descargas activas, terminamos
    if (rastreoActivo && colaPendientes.isEmpty() && peticionesActivas == 0) {
        emit rastreoFinalizado();
    }
}

void RastreadorWeb::alTerminarDescarga(QNetworkReply* reply) {
    peticionesActivas--;
    if (!rastreoActivo) {
        reply->deleteLater();
        return;
    }

    QUrl urlPeticion = reply->request().url(); // La que escribiste en la interfaz
    QUrl urlRespuesta = reply->url();          // La real (por si la página redirigió a 'www')


    if (urlPeticion != urlRespuesta) {
        grafoEnMemoria->agregarArista(urlPeticion.toString(QUrl::RemoveFragment),
                                      urlRespuesta.toString(QUrl::RemoveFragment));
    }

    int profundidadActual = reply->request().attribute(QNetworkRequest::User).toInt();

    if (reply->error() == QNetworkReply::NoError) {
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

        if(contentType.contains("text/html", Qt::CaseInsensitive)) {
            QString html = QString::fromUtf8(reply->readAll());

            // IMPORTANTE: Le pasamos la url de Respuesta (la real), no la de petición
            extraerEnlaces(html, urlRespuesta, profundidadActual);
        }
    } else {
        QString mensajeError = "Omitido (Error HTTP): " + urlPeticion.toString();
        emit error(mensajeError);
    }

    reply->deleteLater();
    procesarSiguiente();
}


void RastreadorWeb::extraerEnlaces(const QString& html, const QUrl& urlActual, int profundidadActual) {
    QRegularExpression regex("<a\\s+(?:[^>]*?\\s+)?href=([\"'])(.*?)\\1", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator iterador = regex.globalMatch(html);

    while (iterador.hasNext()) {
        QRegularExpressionMatch match = iterador.next();
        QString enlaceCrudo = match.captured(2);

        if (enlaceCrudo.isEmpty() || enlaceCrudo.startsWith("#") || enlaceCrudo.startsWith("mailto:") || enlaceCrudo.startsWith("javascript:")) {
            continue;
        }

        QUrl urlDestino = urlActual.resolved(QUrl(enlaceCrudo));

        if (esEnlaceValido(urlDestino, urlActual)) {
            QString strOrigen = urlActual.toString(QUrl::RemoveFragment);
            QString strDestino = urlDestino.toString(QUrl::RemoveFragment);

            grafoEnMemoria->agregarArista(strOrigen, strDestino);

            if (!urlsVisitadas.contains(strDestino)) {
                colaPendientes.enqueue({strDestino, profundidadActual + 1});
            }
        }
    }
}

bool RastreadorWeb::esEnlaceValido(const QUrl& urlDestino, const QUrl& urlOrigen) const {
    if (urlDestino.host() != dominioObjetivo) {
        return false;
    }

    QString path = urlDestino.path().toLower();
    QStringList ignorar = {".jpg", ".jpeg", ".png", ".gif", ".svg", ".webp",
                           ".mp3", ".mp4", ".avi", ".mov",
                           ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".zip", ".rar",
                           ".css", ".js"};

    for (const QString& ext : ignorar) {
        if (path.endsWith(ext)) {
            return false;
        }
    }
    return true;
}
