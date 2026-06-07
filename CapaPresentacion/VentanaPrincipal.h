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
#include <QStandardItemModel>


class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaPrincipal(QWidget *parent = nullptr);
    ~VentanaPrincipal();

private slots:
    // Aquí conectaremos la interfaz con la Capa de Negocio
    void iniciarMapeo();
    void detenerMapeo();
    void buscarPalabra();
    void onEnlaceDescubierto(const QString& url);
    void onRastreoFinalizado();
    void poblarArbolVisual();

private:
    // --- Elementos del Panel Izquierdo (Controles) ---
    GrafoWeb *grafo;
    RastreadorWeb *rastreador;
    QStandardItemModel *modeloArbol;
    QLineEdit *txtUrl;
    QSpinBox *spinProfundidad;
    QPushButton *btnIniciar;
    QPushButton *btnDetener;
    QListWidget *listaFiltros;
    QProgressBar *barraProgreso;
    QLabel *lblEstado;

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

    // Función auxiliar para armar la interfaz
    void configurarInterfaz();
};

#endif // VENTANAPRINCIPAL_H
