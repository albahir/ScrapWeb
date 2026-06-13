#ifndef GRAFOWEB_H
#define GRAFOWEB_H

#include <QString>
#include <QHash>
#include <QStringList>
#include <QList>

/**
 * @class GrafoWeb
 * @brief Clase encargada de gestionar la estructura de un sitio web modelado como un grafo dirigido.
 * @details Utiliza una lista de adyacencia implementada sobre tablas de dispersión (QHash) para almacenar
 * de forma eficiente los nodos (URLs) y sus aristas orientadas (enlaces salientes), sirviendo como base
 * para el cálculo de métricas de red y rutas de navegación.
 */
class GrafoWeb {
public:
    /**
     * @brief Constructor de la clase GrafoWeb.
     * @details Inicializa las variables miembro y los contadores del grafo a su estado por defecto.
     */
    GrafoWeb();

    // Métodos para construir el grafo

    /**
     * @brief Agrega un nuevo nodo al grafo si este no se encuentra registrado en el sistema.
     * @param url Dirección URL que actuará como identificador único del nodo.
     */
    void agregarNodo(const QString& url);

    /**
     * @brief Añade una arista dirigida que conecta una URL de origen con una URL de destino.
     * @details Representa un enlace hipertextual detectado. Si el nodo de origen o el de destino
     * no existen previamente en la estructura de datos, se insertan automáticamente de forma segura.
     * @param urlOrigen URL de la página web que contiene el enlace.
     * @param urlDestino URL de la página de destino a la que apunta el enlace.
     */
    void agregarArista(const QString& urlOrigen, const QString& urlDestino);

    // Métodos de consulta

    /**
     * @brief Verifica de manera constante si una URL ya existe en el grafo.
     * @param url URL que se desea validar.
     * @return true si el nodo está presente en el grafo, false en caso contrario.
     */
    bool contieneNodo(const QString& url) const;

    /**
     * @brief Obtiene los enlaces salientes directos asociados a una URL específica.
     * @param url URL del nodo que se desea consultar.
     * @return QStringList Lista con todas las direcciones URL adyacentes (enlaces de salida).
     */
    QStringList obtenerAdyacentes(const QString& url) const;

    /**
     * @brief Recupera un listado lineal que contiene a todos los nodos registrados en el grafo.
     * @return QList<QString> Lista con los nombres de todas las URLs indexadas hasta el momento.
     */
    QList<QString> obtenerTodosLosNodos() const;

    // Utilidades

    /**
     * @brief Limpia por completo la estructura del grafo restableciendo todos sus componentes y contadores.
     */
    void limpiar();

    /**
     * @brief Devuelve el número total de nodos (páginas web únicas) almacenados en el grafo.
     * @return int Conteo total de nodos existentes en la lista de adyacencia.
     */
    int cantidadNodos() const;

    /**
     * @brief Devuelve la cantidad acumulada de aristas dirigidas válidas que posee la red web.
     * @return int Conteo total de conexiones de red registradas.
     */
    int cantidadAristas() const;

    /**
     * @brief Expone de forma directa la tabla hash subyacente que representa la lista de adyacencia completa.
     * @note Este método se utiliza principalmente para que la capa de datos pueda serializar y guardar el grafo en disco.
     * @return const QHash<QString, QStringList>& Referencia constante a la estructura de adyacencia interna.
     */
    const QHash<QString, QStringList>& obtenerEstructuraCompleta() const;

    /**
     * @brief Ejecuta un algoritmo de búsqueda en anchura (BFS) para calcular las distancias mínimas y la procedencia de los nodos.
     * @details Determina de manera exacta el número de clics mínimos requeridos desde la raíz hacia cualquier página accesible,
     * almacenando la ruta inversa para poder realizar trazados jerárquicos o de enrutamiento en la UI.
     * @param urlOrigen URL base o nodo raíz desde donde comienza la exploración matemática.
     * @param distancias Contenedor hash que se poblará con el conteo de niveles/clics desde la raíz hacia cada URL destino.
     * @param padres Contenedor hash que asociará cada URL con su nodo predecesor directo en el camino óptimo.
     */
    void calcularRutasDesdeRaiz(const QString& urlOrigen,
                                QHash<QString, int>& distancias,
                                QHash<QString, QString>& padres) const;

    /**
     * @brief Reconstruye de manera ordenada la ruta de navegación secuencial que une un origen con un destino.
     * @details Utiliza el mapa de precedencias previamente computado por el BFS para hilar los nodos intermedios del camino.
     * @param urlOrigen URL desde la cual se originó la búsqueda global.
     * @param urlDestino URL final de la cual se desea conocer el camino de clics.
     * @param padres Contenedor hash con la estructura jerárquica de nodos predecesores.
     * @return QStringList Lista con la secuencia ordenada de URLs que representa el camino mínimo de acceso.
     */
    QStringList reconstruirRuta(const QString& urlOrigen, const QString& urlDestino, const QHash<QString, QString>& padres) const;

private:
    // La Lista de Adyacencia:
    // La llave (Key) es la URL de origen.
    // El valor (Value) es una lista de URLs a las que apunta la llave.
    QHash<QString, QStringList> listaAdyacencia;

    // Contador interno para las métricas
    int totalAristas;
};

#endif // GRAFOWEB_H