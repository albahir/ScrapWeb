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

class RastreadorWeb : public QObject {
    Q_OBJECT

public:
    explicit RastreadorWeb(GrafoWeb* grafo, QObject *parent = nullptr);

    void iniciarRastreo(const QString& urlInicial, int profundidadMax, int concurrenciaMax);
    void detenerRastreo();

signals:
    void enlaceDescubierto(const QString& url);
    void rastreoFinalizado();
    void error(const QString& mensaje);
    void paginaDescargada(const QString& url, const QString& html);
    void tiempoTranscurrido(const QString& tiempoStr);

private slots:
    void alTerminarDescarga(QNetworkReply* reply);

private:
    int peticionesActivas;
    GrafoWeb* grafoEnMemoria;
    QNetworkAccessManager* networkManager;
    QElapsedTimer cronometro;



    struct NodoRastreo {
        QString url;
        int profundidad;
    };
    QQueue<NodoRastreo> colaPendientes;
    QSet<QString> urlsVisitadas;

    QString dominioObjetivo;
    int limiteProfundidad;
    int limiteConcurrencia;
    bool rastreoActivo;

    void procesarSiguiente();
    bool esEnlaceValido(const QUrl& urlDestino, const QUrl& urlOrigen) const;
    void extraerEnlaces(const QString& html, const QUrl& urlActual, int profundidadActual);
};

#endif // RASTREADORWEB_H
