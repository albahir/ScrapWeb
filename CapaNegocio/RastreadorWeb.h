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

class RastreadorWeb : public QObject {
    Q_OBJECT

public:
    explicit RastreadorWeb(GrafoWeb* grafo, QObject *parent = nullptr);

    void iniciarRastreo(const QString& urlInicial, int profundidadMax);
    void detenerRastreo();

signals:
    void enlaceDescubierto(const QString& url);
    void rastreoFinalizado();
    void error(const QString& mensaje);

private slots:
    void alTerminarDescarga(QNetworkReply* reply);

private:
    int peticionesActivas;
    const int MAX_PETICIONES_SIMULTANEAS = 4;
    GrafoWeb* grafoEnMemoria;
    QNetworkAccessManager* networkManager;

    struct NodoRastreo {
        QString url;
        int profundidad;
    };
    QQueue<NodoRastreo> colaPendientes;
    QSet<QString> urlsVisitadas;

    QString dominioObjetivo;
    int limiteProfundidad;
    bool rastreoActivo;

    void procesarSiguiente();
    bool esEnlaceValido(const QUrl& urlDestino, const QUrl& urlOrigen) const;
    void extraerEnlaces(const QString& html, const QUrl& urlActual, int profundidadActual);
};

#endif // RASTREADORWEB_H
