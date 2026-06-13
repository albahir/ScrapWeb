#include "AdaptadorGrafoArbol.h"
#include <QStandardItem>
#include <QQueue>
#include <QSet>
#include <QPair>

/**
 * @brief Transfiere de forma jerárquica la estructura del grafo web hacia el modelo de árbol visual.
 * @details Limpia el modelo visual existente y realiza un recorrido en anchura (BFS) sobre el grafo web
 * para mapear la red de páginas descubiertas como nodos jerárquicos de tipo QStandardItem. Utiliza un
 * conjunto de visitados para evitar redundancias y ciclos infinitos durante el emparejamiento.
 * @param grafo Puntero al objeto GrafoWeb que contiene la red de páginas y conexiones rastreadas.
 * @param modelo Puntero al modelo estándar de Qt (QStandardItemModel) que será repoblado.
 * @param urlInicial Dirección URL raíz que servirá como el nodo base principal en la vista de árbol.
 */
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
