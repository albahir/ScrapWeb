#include "AdaptadorGrafoArbol.h"
#include <QStandardItem>
#include <QQueue>
#include <QSet>
#include <QPair>

void AdaptadorGrafoArbol::poblarModelo(GrafoWeb* grafo, QStandardItemModel* modelo, const QString& urlInicial) {
    // 1. Limpiamos el modelo visual y apagamos temporalmente sus cálculos
    modelo->clear();
    modelo->setHorizontalHeaderLabels({"URL"});

    QString urlLimpia = urlInicial;
    if (urlLimpia.endsWith("/")) urlLimpia.chop(1);

    if (!grafo->contieneNodo(urlLimpia)) return;

    // 2. Creamos el nodo raíz
    QStandardItem *itemRaiz = new QStandardItem(urlLimpia);

    QSet<QString> visitadosVisulamente;
    visitadosVisulamente.insert(urlLimpia);

    QQueue<QPair<QString, QStandardItem*>> cola;
    cola.enqueue({urlLimpia, itemRaiz});

    // 3. Bucle BFS
    while (!cola.isEmpty()) {
        auto actual = cola.dequeue();
        QString urlActual = actual.first;
        QStandardItem *itemPadre = actual.second;

        QStringList adyacentes = grafo->obtenerAdyacentes(urlActual);

        for (const QString& urlHijo : adyacentes) {
            if (!visitadosVisulamente.contains(urlHijo)) {
                visitadosVisulamente.insert(urlHijo);

                QStandardItem *itemHijo = new QStandardItem(urlHijo);
                itemPadre->appendRow(itemHijo); // Se agrega al padre en memoria, no genera lag

                cola.enqueue({urlHijo, itemHijo});
            }
        }
    }


    modelo->appendRow(itemRaiz);
}
