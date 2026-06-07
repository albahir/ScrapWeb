#ifndef GRAFOWEB_H
#define GRAFOWEB_H

#include <QString>
#include <QHash>
#include <QStringList>
#include <QList>

class GrafoWeb {
public:
    GrafoWeb();

    // Métodos para construir el grafo
    void agregarNodo(const QString& url);
    void agregarArista(const QString& urlOrigen, const QString& urlDestino);

    // Métodos de consulta
    bool contieneNodo(const QString& url) const;
    QStringList obtenerAdyacentes(const QString& url) const;
    QList<QString> obtenerTodosLosNodos() const;

    // Utilidades
    void limpiar();
    int cantidadNodos() const;
    int cantidadAristas() const;

    // Exponer el grafo completo por si la capa de datos necesita guardarlo en un archivo
    const QHash<QString, QStringList>& obtenerEstructuraCompleta() const;

private:
    // La Lista de Adyacencia:
    // La llave (Key) es la URL de origen.
    // El valor (Value) es una lista de URLs a las que apunta la llave.
    QHash<QString, QStringList> listaAdyacencia;

    // Contador interno para las métricas
    int totalAristas;
};

#endif // GRAFOWEB_H