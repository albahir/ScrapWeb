#include "GrafoWeb.h"
#include "QQueue"
#include "QSet"

/**
 * @brief Constructor de la clase GrafoWeb.
 * @details Inicializa los contadores de la estructura de red web, estableciendo el número
 * de aristas o conexiones acumuladas en cero.
 */
GrafoWeb::GrafoWeb() {
    totalAristas = 0;
}

/**
 * @brief Agrega un nuevo nodo (página web) a la lista de adyacencia.
 * @details Normaliza la dirección URL removiendo la barra diagonal de cierre si existe.
 * Si el nodo no se encuentra previamente registrado, inicializa su lista de enlaces salientes vacía.
 * @param url Dirección URL que actuará como identificador único del nodo.
 */
void GrafoWeb::agregarNodo(const QString& url) {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    // Si la URL limpia no existe, la insertamos
    if (!listaAdyacencia.contains(urlLimpia)) {
        listaAdyacencia.insert(urlLimpia, QStringList());
    }


}

/**
 * @brief Añade una arista dirigida que conecta un nodo origen con un nodo destino.
 * @details Asegura la existencia de ambos nodos en el grafo, normaliza sus URLs para evitar
 * duplicados estéticos y asocia el destino a la lista de adyacencia del origen si no estaba ya presente.
 * @param urlOrigen URL de la página web de procedencia.
 * @param urlDestino URL de la página web vinculada.
 */
void GrafoWeb::agregarArista(const QString& urlOrigen, const QString& urlDestino) {
    // Nos aseguramos de que el nodo de origen exista
    agregarNodo(urlOrigen);
    // Asegurarnos de que el destino también sea un nodo reconocido en el grafo
    agregarNodo(urlDestino);

    QString urlLimpia = urlOrigen;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }
    QString urlLimpia2 = urlDestino;
    if (urlLimpia2.endsWith("/")) {
        urlLimpia2.chop(1);
    }

    // Evitamos agregar enlaces duplicados desde la misma página
    if (!listaAdyacencia[urlLimpia].contains(urlLimpia2)) {
        listaAdyacencia[urlLimpia].append(urlLimpia2);
        totalAristas++;
    }
}

/**
 * @brief Verifica si una URL específica se encuentra registrada en la estructura del grafo.
 * @details Aplica una normalización previa a la cadena y realiza una consulta de existencia en el hash.
 * @param url URL que se desea comprobar.
 * @return true si el nodo existe en la lista de adyacencia, false en caso contrario.
 */
bool GrafoWeb::contieneNodo(const QString& url) const {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    return listaAdyacencia.contains(urlLimpia);
}

/**
 * @brief Recupera los enlaces de salida o páginas adyacentes a un nodo dado.
 * @details Limpia la URL de barras finales y extrae la lista de strings asociada en el hash.
 * @param url URL del nodo origen a consultar.
 * @return QStringList Lista de URLs a las que el nodo origen apunta directamente.
 */
QStringList GrafoWeb::obtenerAdyacentes(const QString& url) const {
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    if (listaAdyacencia.contains(urlLimpia)) {
        return listaAdyacencia.value(urlLimpia);
    }
    return QStringList();
}

/**
 * @brief Obtiene el listado completo de todos los nodos (claves del hash) indexados en el grafo.
 * @return QList<QString> Lista lineal con los identificadores únicos de todas las páginas web registradas.
 */
QList<QString> GrafoWeb::obtenerTodosLosNodos() const {
    return listaAdyacencia.keys();
}

/**
 * @brief Restablece el grafo por completo eliminando todos sus nodos y aristas.
 * @details Vacía la tabla hash y pone a cero el contador total de conexiones.
 */
void GrafoWeb::limpiar() {
    listaAdyacencia.clear();
    totalAristas = 0;
}

/**
 * @brief Devuelve la cantidad actual de nodos únicos registrados en la red.
 * @return int Tamaño o número de entradas en la lista de adyacencia.
 */
int GrafoWeb::cantidadNodos() const {
    return listaAdyacencia.size();
}

/**
 * @brief Devuelve el conteo acumulado de aristas o enlaces hipertextuales válidos.
 * @return int Número total de conexiones dirigidas detectadas.
 */
int GrafoWeb::cantidadAristas() const {
    return totalAristas;
}

/**
 * @brief Permite el acceso de solo lectura a la estructura hash de adyacencia subyacente.
 * @details Utilizado principalmente por componentes de persistencia externos (capa de datos).
 * @return const QHash<QString, QStringList>& Referencia constante a la tabla hash interna.
 */
const QHash<QString, QStringList>& GrafoWeb::obtenerEstructuraCompleta() const {
    return listaAdyacencia;
}

/**
 * @brief Ejecuta el algoritmo de exploración BFS para calcular distancias óptimas y predecesores desde la raíz.
 * @details Limpia los contenedores de salida e inicia un recorrido en anchura desde la URL origen.
 * Determina el número de clics mínimos para alcanzar cualquier otro nodo y guarda la relación de padres.
 * @param urlOrigen URL raíz o punto de partida del recorrido.
 * @param distancias Hash de salida que mapeará cada URL con su distancia en clics desde la raíz.
 * @param padres Hash de salida que asociará cada URL con su nodo predecesor directo en el camino mínimo.
 */
void GrafoWeb::calcularRutasDesdeRaiz(const QString& urlOrigen,
                                      QHash<QString, int>& distancias,
                                      QHash<QString, QString>& padres) const {
    distancias.clear();
    padres.clear();

    QString origenLimpio = urlOrigen;
    if (origenLimpio.endsWith("/")) origenLimpio.chop(1);

    if (!listaAdyacencia.contains(origenLimpio)) return;

    QQueue<QString> cola;
    cola.enqueue(origenLimpio);
    distancias.insert(origenLimpio, 0);

    while (!cola.isEmpty()) {
        QString actual = cola.dequeue();
        int distActual = distancias.value(actual);

        QStringList adyacentes = listaAdyacencia.value(actual);
        for (const QString& vecino : adyacentes) {
            if (!distancias.contains(vecino)) {
                distancias.insert(vecino, distActual + 1);
                padres.insert(vecino, actual); // Guardamos quién es el predecesor directo
                cola.enqueue(vecino);
            }
        }
    }
}

/**
 * @brief Traza y reconstruye la secuencia de nodos ordenados que forman la ruta óptima.
 * @details Realiza un recorrido inverso desde el destino hacia el origen utilizando el mapa de precedencias.
 * Si no se consigue un camino conectado válido hasta la raíz, devuelve una lista vacía.
 * @param urlOrigen URL inicial o nodo raíz de la búsqueda.
 * @param urlDestino URL de destino a la que se desea llegar.
 * @param padres Hash de consulta que contiene la estructura de relaciones de procedencia.
 * @return QStringList Lista ordenada secuencialmente desde el origen hasta el destino si la ruta es válida.
 */
QStringList GrafoWeb::reconstruirRuta(const QString& urlOrigen, const QString& urlDestino, const QHash<QString, QString>& padres) const {
    QStringList ruta;
    QString pasoActual = urlDestino;

    // Navegamos hacia atrás usando el diccionario de padres
    while (pasoActual != urlOrigen && padres.contains(pasoActual)) {
        ruta.prepend(pasoActual);
        pasoActual = padres.value(pasoActual);
    }

    // Validamos que realmente hayamos llegado a la raíz
    if (pasoActual == urlOrigen) {
        ruta.prepend(urlOrigen);
    } else {
        ruta.clear(); // Ruta inválida o inalcanzable
    }

    return ruta;
}