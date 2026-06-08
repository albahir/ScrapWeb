#ifndef INDICEINVERTIDO_H
#define INDICEINVERTIDO_H

#include <QString>
#include <QHash>
#include <QSet>
#include <QStringList> // Necesario para devolver los resultados ordenados
#include <QRegularExpression>

class IndiceInvertido {
public:
    IndiceInvertido();

    void indexarPagina(const QString& url, const QString& contenidoTexto);

    // Cambiamos el retorno de QSet a QStringList para mantener el orden
    QStringList buscar(const QString& palabra) const;

    void limpiar();

private:
    // EL NÚCLEO DEL CAMBIO: Palabra -> (URL -> Frecuencia de aparición)
    QHash<QString, QHash<QString, int>> tablaIndice;

    QSet<QString> palabrasIgnoradas;
    QRegularExpression regexPalabras;
    QRegularExpression regexEtiquetasHTML;

    QString normalizarPalabra(const QString& palabra) const;
    void inicializarPalabrasIgnoradas();
};

#endif // INDICEINVERTIDO_H