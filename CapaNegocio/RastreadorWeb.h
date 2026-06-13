#ifndef RASTREADORWEB_H
#define RASTREADORWEB_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrl>
#include <QSet>
#include <QQueue>
#include "GrafoWeb.h"
#include <QElapsedTimer>

/**
 * @class RastreadorWeb
 * @brief Clase encargada del rastreo asíncrono y la extracción de enlaces de sitios web (Web Crawler).
 * @details Hereda de QObject para aprovechar el sistema de señales y slots de Qt. Realiza peticiones
 * HTTP concurrentes controladas, filtra URLs por dominio, y alimenta de forma progresiva el GrafoWeb
 * en memoria a medida que descubre y descarga páginas.
 */
class RastreadorWeb : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor de la clase RastreadorWeb.
     * @param grafo Puntero al objeto GrafoWeb donde se registrarán los nodos y aristas descubiertos.
     * @param parent Puntero al objeto QObject padre (por defecto nullptr).
     */
    explicit RastreadorWeb(GrafoWeb* grafo, QObject *parent = nullptr);

    /**
     * @brief Inicializa y arranca el proceso de rastreo web con los límites configurados.
     * @param urlInicial Dirección URL base desde la cual comenzará la exploración.
     * @param profundidadMax Límite de niveles jerárquicos a explorar (0 para ilimitado).
     * @param concurrenciaMax Número máximo de peticiones HTTP simultáneas permitidas.
     */
    void iniciarRastreo(const QString& urlInicial, int profundidadMax, int concurrenciaMax);

    /**
     * @brief Detiene de forma segura el proceso de rastreo activo.
     * @details Cancela la recepción de nuevas URLs de la cola y cambia los flags de control,
     * permitiendo que las peticiones en vuelo terminen limpiamente.
     */
    void detenerRastreo();

signals:
    /**
     * @brief Señal emitida inmediatamente al descubrir una nueva dirección URL en el HTML.
     * @param url Cadena de texto con la URL descubierta.
     */
    void enlaceDescubierto(const QString& url);

    /**
     * @brief Señal emitida cuando la cola de pendientes se vacía o el usuario aborta el proceso.
     */
    void rastreoFinalizado();

    /**
     * @brief Señal emitida cuando ocurre una anomalía de red o un error crítico de conexión.
     * @param mensaje Descripción textual del error producido.
     */
    void error(const QString& mensaje);

    /**
     * @brief Señal emitida tras descargar con éxito el código fuente de una página.
     * @param url URL de procedencia del documento.
     * @param html Cadena con el código fuente web plano (HTML).
     */
    void paginaDescargada(const QString& url, const QString& html);

    /**
     * @brief Señal periódica que transmite el tiempo transcurrido desde el inicio del rastreo.
     * @param tiempoStr Cadena formateada que representa el tiempo (MM:SS).
     */
    void tiempoTranscurrido(const QString& tiempoStr);

private slots:
    /**
     * @brief Slot asíncrono invocado automáticamente cuando una petición HTTP de red finaliza.
     * @details Procesa la respuesta de red, maneja códigos de estado, emite los contenidos descargados
     * e inicia la extracción de enlaces internos antes de liberar el hilo de red.
     * @param reply Puntero al objeto de respuesta de red gestionado por Qt.
     */
    void alTerminarDescarga(QNetworkReply* reply);

private:
    int peticionesActivas;
    GrafoWeb* grafoEnMemoria;
    QNetworkAccessManager* networkManager;
    QElapsedTimer cronometro;

    /**
     * @struct NodoRastreo
     * @brief Estructura interna para almacenar el estado de exploración de una URL en la cola.
     */
    struct NodoRastreo {
        QString url;     ///< Dirección URL de la página.
        int profundidad; ///< Nivel jerárquico/profundidad en el que fue encontrada.
    };
    QQueue<NodoRastreo> colaPendientes;
    QSet<QString> urlsVisitadas;

    QString dominioObjetivo;
    int limiteProfundidad;
    int limiteConcurrencia;
    bool rastreoActivo;

    /**
     * @brief Extrae el siguiente elemento de la cola de pendientes y lanza su petición HTTP si no excede la concurrencia.
     */
    void procesarSiguiente();

    /**
     * @brief Valida si una URL de destino pertenece al mismo dominio que la URL de origen y cumple criterios estéticos.
     * @param urlDestino Objeto QUrl de la dirección encontrada.
     * @param urlOrigen Objeto QUrl de la página contenedora.
     * @return true si el enlace es válido para ser rastreado, false en caso contrario.
     */
    bool esEnlaceValido(const QUrl& urlDestino, const QUrl& urlOrigen) const;

    /**
     * @brief Analiza el código HTML mediante expresiones regulares para extraer hipervínculos válidos.
     * @details Normaliza los enlaces relativos a absolutos, inyecta las aristas resultantes en el grafo
     * y encola los nuevos nodos si no exceden el límite de profundidad establecido.
     * @param html Código fuente de la página web actual.
     * @param urlActual Objeto QUrl de la página que se está analizando.
     * @param profundidadActual Nivel de profundidad del nodo actual.
     */
    void extraerEnlaces(const QString& html, const QUrl& urlActual, int profundidadActual);
};

#endif // RASTREADORWEB_H
