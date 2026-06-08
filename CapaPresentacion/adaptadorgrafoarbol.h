#ifndef ADAPTADORGRAFOARBOL_H
#define ADAPTADORGRAFOARBOL_H

#include "GrafoWeb.h"
#include <QStandardItemModel>
#include <QString>

class AdaptadorGrafoArbol {
public:

    static void poblarModelo(GrafoWeb* grafo, QStandardItemModel* modelo, const QString& urlInicial);
};

#endif // ADAPTADORGRAFOARBOL_H
