#include "AdaptadorGrafoArbol.h"
#include <QStandardItem>
#include <QQueue>
#include <QSet>
#include <QPair>

void AdaptadorGrafoArbol::poblarModelo(GrafoWeb* grafo, QStandardItemModel* modelo, const QString& urlInicial) {
    modelo->clear();
    modelo->setHorizontalHeaderLabels({"URL"});

    // Limpiamos el slash de la URL inicial para asegurar coincidencia con el grafo
    QString urlLimpia = urlInicial;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    if (!grafo->contieneNodo(urlLimpia)) return;

    // Nodo raíz del QTreeView
    QStandardItem *itemRaiz = new QStandardItem(urlLimpia);
    modelo->appendRow(itemRaiz);

    // Lógica de recorrido BFS (Anchura) adaptada para la interfaz visual
    QSet<QString> visitadosVisulamente;
    visitadosVisulamente.insert(urlLimpia);

    QQueue<QPair<QString, QStandardItem*>> cola;
    cola.enqueue({urlLimpia, itemRaiz});

    while (!cola.isEmpty()) {
        auto actual = cola.dequeue();
        QString urlActual = actual.first;
        QStandardItem *itemPadre = actual.second;

        QStringList adyacentes = grafo->obtenerAdyacentes(urlActual);

        for (const QString& urlHijo : adyacentes) {
            if (!visitadosVisulamente.contains(urlHijo)) {
                visitadosVisulamente.insert(urlHijo);

                QStandardItem *itemHijo = new QStandardItem(urlHijo);
                itemPadre->appendRow(itemHijo);

                cola.enqueue({urlHijo, itemHijo});
            }
        }
    }
}
