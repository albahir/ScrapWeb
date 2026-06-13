#include "RastreadorWeb.h"
#include <QDebug>

/**
 * @brief Constructor de la clase RastreadorWeb.
 * @details Inicializa el administrador de acceso a red (QNetworkAccessManager), configura los flags de
 * control por defecto y conecta de forma segura la señal de finalización de descargas de red con el
 * slot encargado de procesar las respuestas HTTP.
 * @param grafo Puntero al grafo web en memoria donde se inyectará la topología estructurada descubierta.
 * @param parent Componente QObject padre para la gestión automática de memoria en la jerarquía de Qt.
 */
RastreadorWeb::RastreadorWeb(GrafoWeb* grafo, QObject *parent)
    : QObject(parent), grafoEnMemoria(grafo) {
    networkManager = new QNetworkAccessManager(this);
    rastreoActivo = false;
    peticionesActivas = 0;
    limiteConcurrencia = 1;

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &RastreadorWeb::alTerminarDescarga);
}

/**
 * @brief Configura las variables de entorno e inicia el proceso asíncrono de rastreo web.
 * @details Normaliza el protocolo de la URL base, limpia por completo las colas y conjuntos de visitas previas,
 * vacía la estructura del grafo en memoria, extrae el dominio objetivo para el filtrado estricto, encola
 * el nodo raíz en nivel cero e inicia el cronómetro de rendimiento antes de activar el bucle de despacho.
 * @param urlInicial Dirección de la página web desde la cual arrancará el escaneo de red.
 * @param profundidadMax Límite superior de clics o saltos jerárquicos de enlaces a procesar (0 para ilimitado).
 * @param concurrenciaMax Número máximo de descargas simultáneas permitidas en vuelo por la red.
 */
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

/**
 * @brief Solicita la detención segura de la exploración web activa.
 * @details Cambia el estado del flag de control de flujo, bloqueando el procesamiento de nuevos elementos de la cola.
 */
void RastreadorWeb::detenerRastreo() {

    rastreoActivo = false;
}

/**
 * @brief Gestiona el despacho asíncrono de peticiones de red respetando las cuotas de concurrencia y profundidad.
 * @details Desencola elementos descartando duplicados ya visitados de forma visual o lógica. Configura los atributos
 * internos de la petición HTTP (incluyendo tiempos de espera y el nivel de profundidad actual como metadato del socket)
 * y despacha la solicitud GET. Si la cola se vacía y no quedan descargas activas, calcula la métrica temporal de rendimiento
 * y emite las señales de conclusión.
 */
void RastreadorWeb::procesarSiguiente() {
    // Si ya alcanzamos el límite, no lanzamos nada más; esperamos a que termine una descarga
    while (rastreoActivo && peticionesActivas < limiteConcurrencia && !colaPendientes.isEmpty()) {

        NodoRastreo nodoActual = colaPendientes.dequeue();
        if (urlsVisitadas.contains(nodoActual.url)) {
            qDebug() << "    [Escudo Duplicados] 🛑 Atrapada y destruida en cola:" << nodoActual.url;
            continue;
        }

        urlsVisitadas.insert(nodoActual.url);
        emit enlaceDescubierto(nodoActual.url);

        if (limiteProfundidad > 0 && nodoActual.profundidad >= limiteProfundidad) {
            qDebug() << "[Rastreador] 🛑 DESCARTADO (Profundidad límite alcanzada):" << nodoActual.url << "| Nivel:" << nodoActual.profundidad;
            continue;
        }

        QNetworkRequest peticion((QUrl(nodoActual.url)));
        peticion.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        peticion.setAttribute(QNetworkRequest::User, nodoActual.profundidad);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        peticion.setTransferTimeout(10000);
#endif

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

/**
 * @brief Slot de procesamiento de respuestas HTTP que maneja las descargas de red finalizadas.
 * @details Decrementa la concurrencia activa, procesa posibles redirecciones agregando aristas implícitas
 * en el grafo, extrae metadatos y valida que el tipo de contenido corresponda a un documento web "text/html".
 * Si no hay errores, limpia la URL de fragmentos o barras diagonales finales, emite el código fuente para su
 * indexación inversa en la interfaz y delega la extracción de enlaces internos. Si hay errores de red,
 * gestiona excepciones críticas según el nivel jerárquico del nodo.
 * @param reply Puntero al objeto contenedor de la respuesta y estado del socket de red gestionado por Qt.
 */
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
        QString errorString = reply->errorString();
        qDebug() << "[Rastreador] ❌ ERROR de Red:" << urlPeticion.toString() << "->" << errorString;

        if (profundidadActual == 0) {
            // Si la página principal (nivel 0) falla, disparamos una alerta crítica.
            emit error("CRITICO: El sitio web inicial no existe, está caído o sin conexión.\n\nDetalle técnico: " + errorString);
        } else {
            // Si un enlace "hijo" está roto, lo reportamos suavemente y el programa sigue mapeando el resto.
            emit error("⚠️ Omitiendo enlace roto o lento: " + urlPeticion.host());
        }
    }

    reply->deleteLater();
    procesarSiguiente();
}

/**
 * @brief Escanea el código fuente HTML buscando hipervínculos mediante expresiones regulares.
 * @details Utiliza una regex insensible a mayúsculas para capturar los atributos `href` de etiquetas de anclaje.
 * Descarta enlaces locales inertes (`#`), protocolos de correo o instrucciones de JavaScript. Resuelve las URLs
 * relativas con respecto a la página base bajo análisis, inserta las aristas directas en la estructura del grafo
 * y encola de manera jerárquica los nuevos destinos válidos incrementando el nivel de profundidad.
 * @param html Cadena de texto que contiene el código fuente HTML de la página descargada.
 * @param urlActual Objeto URL absoluto que representa la dirección de la página que contiene el HTML.
 * @param profundidadActual Nivel jerárquico actual en el árbol de exploración del rastreador.
 */
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

/**
 * @brief Evalúa si una URL candidata cumple con las políticas de rastreo de dominio y extensión de archivo.
 * @details Valida que el host de destino coincida estrictamente con el dominio objetivo configurado al inicio,
 * evitando fugas accidentales del rastreador hacia internet. Adicionalmente, contrasta la ruta del enlace con una
 * lista negra de extensiones multimedia, hojas de estilo, scripts y documentos binarios para evitar descargas innecesarias.
 * @param urlDestino Objeto QUrl de la dirección web candidata a evaluar.
 * @param urlOrigen Objeto QUrl de la página de procedencia que contenía el hipervínculo.
 * @return true si la dirección web cumple con todos los criterios de validez para el rastreo, false en caso contrario.
 */
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