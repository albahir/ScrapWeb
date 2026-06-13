#ifndef ADAPTADORGRAFOARBOL_H
#define ADAPTADORGRAFOARBOL_H

#include "GrafoWeb.h"
#include <QStandardItemModel>
#include <QString>

/**
 * @class AdaptadorGrafoArbol
 * @brief Clase utilitaria que implementa el patrón Adapter entre la capa lógica del Grafo y la UI de Qt.
 * @details Se encarga de transformar la estructura de red del GrafoWeb en un formato jerárquica
 * compatible con QStandardItemModel para su posterior despliegue visual en un QTreeView.
 */
class AdaptadorGrafoArbol {
public:

    /**
     * @brief Transfiere de forma jerárquica la estructura del grafo web hacia el modelo de árbol visual.
     * @details Limpia el modelo visual existente y realiza un recorrido sistemático de los nodos y
     * aristas del GrafoWeb comenzando desde la URL raíz, organizando los enlaces por su relación de dependencia.
     * @param grafo Puntero al objeto GrafoWeb que contiene la red de páginas y conexiones rastreadas.
     * @param modelo Puntero al modelo estándar de Qt (QStandardItemModel) que será repoblado.
     * @param urlInicial Dirección URL raíz que servirá como el nodo base principal en la vista de árbol.
     */
    static void poblarModelo(GrafoWeb* grafo, QStandardItemModel* modelo, const QString& urlInicial);
};

#endif // ADAPTADORGRAFOARBOL_H