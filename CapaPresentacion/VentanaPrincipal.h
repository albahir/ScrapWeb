#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QTreeView>
#include <QListWidget>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include "GrafoWeb.h"
#include "RastreadorWeb.h"
#include "IndiceInvertido.h"
#include "AnalizadorMetricas.h" // Enlaza el motor de métricas
#include "MetricasSitio.h"
#include <QStandardItemModel>


class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaPrincipal(QWidget *parent = nullptr);
    ~VentanaPrincipal();
    enum EstadoAplicacion {
        ESTADO_ESPERANDO,
        ESTADO_MAPEANDO,
        ESTADO_FINALIZADO,
        ESTADO_CANCELADO
    };

private slots:

    void iniciarMapeo();
    void detenerMapeo();
    void buscarPalabra();
    void onEnlaceDescubierto(const QString& url);
    void onRastreoFinalizado();
    void onPaginaDescargada(const QString& url, const QString& html);
    void onTiempoTranscurrido(const QString& tiempoStr);
    void actualizarPanelMetricas();
    void guardarHistorial();
    void cargarHistorial();

private:
    // --- Elementos del Panel Izquierdo (Controles) ---
    GrafoWeb *grafo;
    RastreadorWeb *rastreador;
    IndiceInvertido *miIndiceInvertido;
    QStandardItemModel *modeloArbol;
    QLineEdit *txtUrl;
    QSpinBox *spinProfundidad;
    QSpinBox *spinConcurrencia;
    QPushButton *btnIniciar;
    QPushButton *btnDetener;
    QPushButton *btnGuardar;
    QPushButton *btnCargar;
    QListWidget *listaFiltros;
    QProgressBar *barraProgreso;
    QLabel *lblEstado;

    void configurarInterfaz();
    QVBoxLayout* crearPanelIzquierdo();
    QVBoxLayout* crearPanelCentral();
    QVBoxLayout* crearPanelDerecho();
    // --- Elementos del Panel Central (Estructura y Búsqueda) ---
    QTreeView *arbolEstructura;
    QLineEdit *txtBuscar;
    QPushButton *btnBuscar;
    QListWidget *listaResultadosBusqueda;
    QComboBox *cmbFiltroBusqueda;
    void cambiarEstadoUI(EstadoAplicacion estado);
    void aplicarEstilosGlobales();

    // --- Elementos del Panel Derecho (Métricas) ---
    QLabel *lblTiempoEjecucion;
    QLabel *lblPaginasEncontradas;
    QLabel *lblEnlacesDetectados;
    QLabel *lblDensidadConexiones;
    QLabel *lblTamanoDescargado;
    QLabel *lblPaginaMasConectada;
    QLabel *lblPaginaMasReferenciada;
    QLabel *lblPaginasSumidero;
    //  Guarda el string de tiempo final para el cálculo estadístico
    QString ultimoTiempoRastreo;
    qint64 totalBytesContados = 0;


};

#endif // VENTANAPRINCIPAL_H
