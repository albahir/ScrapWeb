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
#include "GrafoWeb.h"
#include "RastreadorWeb.h"
#include "IndiceInvertido.h"
#include <QStandardItemModel>


class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaPrincipal(QWidget *parent = nullptr);
    ~VentanaPrincipal();

private slots:

    void iniciarMapeo();
    void detenerMapeo();
    void buscarPalabra();
    void onEnlaceDescubierto(const QString& url);
    void onRastreoFinalizado();
    void onPaginaDescargada(const QString& url, const QString& html);
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

    // --- Elementos del Panel Derecho (Métricas) ---
    QLabel *lblTotalPaginas;
    QLabel *lblProfundidadMax;
    QLabel *lblMasEnlaces;
    QLabel *lblTamanoTotal;
    QLabel *lblTiempoEjecucion;


};

#endif // VENTANAPRINCIPAL_H
