#ifndef INDICEINVERTIDO_H
#define INDICEINVERTIDO_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QRegularExpression>
#include <QVector> //   Para acceso ultra rápido por ID

class IndiceInvertido {
public:
    IndiceInvertido();

    void indexarPagina(const QString& url, const QString& contenidoTexto);
    QStringList buscar(const QString& palabra) const;
    void limpiar();

private:
    // (ID_URL -> Frecuencia)
    QHash<QString, QHash<int, int>> tablaIndice;

    // DICCIONARIOS INTERNOS PRIVADOS
    QHash<QString, int> mapaUrlAId;
    QVector<QString> mapaIdAUrl;

    QSet<QString> palabrasIgnoradas;
    QRegularExpression regexPalabras;
    QRegularExpression regexEtiquetasHTML;

    QString normalizarPalabra(const QString& palabra) const;
    void inicializarPalabrasIgnoradas();
};

#endif // INDICEINVERTIDO_H
