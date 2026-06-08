#include "RastreadorWeb.h"
#include <QDebug>

RastreadorWeb::RastreadorWeb(GrafoWeb* grafo, QObject *parent)
    : QObject(parent), grafoEnMemoria(grafo) {
    networkManager = new QNetworkAccessManager(this);
    rastreoActivo = false;
    peticionesActivas = 0;
    limiteConcurrencia = 1;

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &RastreadorWeb::alTerminarDescarga);
}

void RastreadorWeb::iniciarRastreo(const QString& urlInicial, int profundidadMax, int concurrenciaMax) {

    QString urlAProcesar = urlInicial;
    if(!urlAProcesar.startsWith("http://") && !urlAProcesar.startsWith("https://")) {
        urlAProcesar = "https://" + urlAProcesar;
    }

    colaPendientes.clear();
    urlsVisitadas.clear();
    grafoEnMemoria->limpiar();

    limiteProfundidad = profundidadMax;
    limiteConcurrencia = concurrenciaMax;
    rastreoActivo = true;
    peticionesActivas = 0;

    QUrl urlDecodificada(urlAProcesar);
    if (!urlDecodificada.isValid()) {
        emit error("La URL inicial no es válida.");
        return;
    }
    dominioObjetivo = urlDecodificada.host();

    colaPendientes.enqueue({urlAProcesar, 0});
    cronometro.start();
    procesarSiguiente();
}

void RastreadorWeb::detenerRastreo() {

    rastreoActivo = false;
}

void RastreadorWeb::procesarSiguiente() {
    // Si ya alcanzamos el límite, no lanzamos nada más; esperamos a que termine una descarga
    while (rastreoActivo && peticionesActivas < limiteConcurrencia && !colaPendientes.isEmpty()) {

        NodoRastreo nodoActual = colaPendientes.dequeue();
        if (urlsVisitadas.contains(nodoActual.url)) continue;

        urlsVisitadas.insert(nodoActual.url);
        emit enlaceDescubierto(nodoActual.url);

        if (limiteProfundidad > 0 && nodoActual.profundidad >= limiteProfundidad) {
            qDebug() << "[Rastreador] 🛑 DESCARTADO (Profundidad límite alcanzada):" << nodoActual.url << "| Nivel:" << nodoActual.profundidad;
            continue;
        }

        QNetworkRequest peticion((QUrl(nodoActual.url)));
        peticion.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        peticion.setAttribute(QNetworkRequest::User, nodoActual.profundidad);

        peticionesActivas++; // Incrementamos el contador

        qDebug() << "[Rastreador] 🚀 ENVIANDO:" << nodoActual.url
                 << "| Profundidad:" << nodoActual.profundidad
                 << "| Activas:" << peticionesActivas << "/" << limiteConcurrencia;
        networkManager->get(peticion);
    }

    // Si ya no hay nada en cola y no hay descargas activas, terminamos
    if (rastreoActivo && colaPendientes.isEmpty() && peticionesActivas == 0) {
        // 1. Calculamos el tiempo total en milisegundos
        qint64 milisegundos = cronometro.elapsed();
        double segundos = milisegundos / 1000.0;

        // 2. Formateamos el texto (ejemplo: "2.45s")
        QString tiempoTexto = QString::number(segundos, 'f', 2) + "s";

        // 3. LOG DE CONSOLA EXPANDIDO (Como lo pediste)
        qDebug() << "\n=====================================================";
        qDebug() << "[Rastreador] 🏁 MAPEO FINALIZADO EXITOSAMENTE";
        qDebug() << "[Rastreador] ⏱️ Tiempo total de ejecución:" << tiempoTexto;
        qDebug() << "=====================================================\n";

        // 4. Enviamos el valor a la interfaz gráfica
        emit tiempoTranscurrido(tiempoTexto);

        emit rastreoFinalizado();
    }

}

void RastreadorWeb::alTerminarDescarga(QNetworkReply* reply) {
    peticionesActivas--;

    if(peticionesActivas<0)peticionesActivas=0;

    if (!rastreoActivo) {
        reply->deleteLater();
        return;
    }

    QUrl urlPeticion = reply->request().url();
    QUrl urlRespuesta = reply->url();

    if (urlPeticion != urlRespuesta) {
        grafoEnMemoria->agregarArista(urlPeticion.toString(QUrl::RemoveFragment),
                                      urlRespuesta.toString(QUrl::RemoveFragment));
    }

    int profundidadActual = reply->request().attribute(QNetworkRequest::User).toInt();
    qDebug() << "[Rastreador] ✅ RECIBIDO:" << urlRespuesta.toString()
             << "| Nivel:" << profundidadActual
             << "| Cupos ocupados:" << peticionesActivas << "/" << limiteConcurrencia
             << "| Páginas pendientes en cola:" << colaPendientes.size();
    if (reply->error() == QNetworkReply::NoError) {
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

        if(contentType.contains("text/html", Qt::CaseInsensitive)) {
            QString html = QString::fromUtf8(reply->readAll());

            // 1. OBTENER URL Y NORMALIZAR (Quitar el slash final para que coincida con el grafo)
            QString urlLimpia = urlRespuesta.toString(QUrl::RemoveFragment);
            if (urlLimpia.endsWith("/")) {
                urlLimpia.chop(1);
            }

            // 2. Emitimos la señal con la URL ya limpia.
            // VentanaPrincipal la recibirá y la meterá en el Índice Invertido oficial.
            emit paginaDescargada(urlLimpia, html);

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

    int nuevosEncontrados = 0;
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
                 nuevosEncontrados++;
            }
        }
    }
    qDebug() << "    -> Extracción finalizada en:" << urlActual.toString() << "| Nuevos hijos encolados:" << nuevosEncontrados;
}

bool RastreadorWeb::esEnlaceValido(const QUrl& urlDestino, const QUrl& urlOrigen) const {
    if (urlDestino.host() != dominioObjetivo) {
        qDebug() << "    [Filtro Dominio] 🚫 Ignorado por ser enlace externo:" << urlDestino.host();
        return false;
    }

    QString path = urlDestino.path().toLower();
    QStringList ignorar = {".jpg", ".jpeg", ".png", ".gif", ".svg", ".webp",
                           ".mp3", ".mp4", ".avi", ".mov",
                           ".pdf", ".doc", ".docx", ".xls", ".xlsx", ".zip", ".rar",
                           ".css", ".js"};

    for (const QString& ext : ignorar) {
        if (path.endsWith(ext)) {
            qDebug() << "    [Filtro Archivo] 🖼️ Ignorado por extensión prohibida (" << ext << "):" << path;
            return false;
        }
    }
    return true;
}
