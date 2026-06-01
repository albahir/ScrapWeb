#include "VentanaPrincipal.h"
#include <QFormLayout>
#include <QHeaderView>

VentanaPrincipal::VentanaPrincipal(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Analizador de Sitios Web v1.0");
    resize(1050, 600); // Tamaño inicial basado en tu imagen

    configurarInterfaz();
}

VentanaPrincipal::~VentanaPrincipal() {
    // Qt maneja la destrucción de los widgets hijos automáticamente
}

void VentanaPrincipal::configurarInterfaz() {
    // Contenedor principal
    QWidget *widgetCentral = new QWidget(this);
    setCentralWidget(widgetCentral);
    QHBoxLayout *layoutPrincipal = new QHBoxLayout(widgetCentral);

    // =========================================================
    // 1. PANEL IZQUIERDO: Controles de Análisis
    // =========================================================
    QVBoxLayout *layoutIzquierdo = new QVBoxLayout();

    QGroupBox *grupoControles = new QGroupBox("Controles de Mapeo");
    QVBoxLayout *layoutGrupoControles = new QVBoxLayout(grupoControles);

    layoutGrupoControles->addWidget(new QLabel("URL Inicial:"));
    txtUrl = new QLineEdit();
    txtUrl->setPlaceholderText("https://ejemplo.com");
    layoutGrupoControles->addWidget(txtUrl);

    layoutGrupoControles->addWidget(new QLabel("Profundidad Máxima (0 = ilimitada):"));
    spinProfundidad = new QSpinBox();
    spinProfundidad->setRange(0, 100);
    spinProfundidad->setValue(3);
    layoutGrupoControles->addWidget(spinProfundidad);

    btnIniciar = new QPushButton("▶ Iniciar Mapeo");
    btnIniciar->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    btnDetener = new QPushButton("■ Detener Mapeo");
    btnDetener->setEnabled(false); // Deshabilitado al inicio
    layoutGrupoControles->addWidget(btnIniciar);
    layoutGrupoControles->addWidget(btnDetener);

    layoutIzquierdo->addWidget(grupoControles);

    // Filtros
    QGroupBox *grupoFiltros = new QGroupBox("Filtros de Contenido (Ignorar)");
    QVBoxLayout *layoutFiltros = new QVBoxLayout(grupoFiltros);
    listaFiltros = new QListWidget();
    listaFiltros->addItems({"Imágenes", "Videos", "Audio", "Documentos", "CSS", "Scripts", "Archivos Comprimidos"});
    listaFiltros->setEnabled(false); // Solo lectura, como exige el proyecto
    layoutFiltros->addWidget(listaFiltros);
    layoutIzquierdo->addWidget(grupoFiltros);

    // Progreso
    barraProgreso = new QProgressBar();
    barraProgreso->setValue(0);
    lblEstado = new QLabel("Estado: Esperando...");
    layoutIzquierdo->addWidget(barraProgreso);
    layoutIzquierdo->addWidget(lblEstado);

    layoutPrincipal->addLayout(layoutIzquierdo, 1); // El '1' es la proporción de estiramiento

    // =========================================================
    // 2. PANEL CENTRAL: Estructura y Búsqueda
    // =========================================================
    QVBoxLayout *layoutCentral = new QVBoxLayout();

    QGroupBox *grupoEstructura = new QGroupBox("Estructura del Sitio (Mismo Dominio)");
    QVBoxLayout *layoutGrupoEstructura = new QVBoxLayout(grupoEstructura);
    arbolEstructura = new QTreeView();
    layoutGrupoEstructura->addWidget(arbolEstructura);
    layoutCentral->addWidget(grupoEstructura, 2); // Ocupa más espacio vertical

    QGroupBox *grupoBusqueda = new QGroupBox("Búsqueda por Palabra Clave");
    QVBoxLayout *layoutGrupoBusqueda = new QVBoxLayout(grupoBusqueda);
    QHBoxLayout *layoutBuscador = new QHBoxLayout();

    txtBuscar = new QLineEdit();
    txtBuscar->setPlaceholderText("Ingrese palabra clave...");
    btnBuscar = new QPushButton("🔍 Buscar Palabra");

    layoutBuscador->addWidget(txtBuscar);
    layoutBuscador->addWidget(btnBuscar);

    listaResultadosBusqueda = new QListWidget();

    layoutGrupoBusqueda->addLayout(layoutBuscador);
    layoutGrupoBusqueda->addWidget(listaResultadosBusqueda);
    layoutCentral->addWidget(grupoBusqueda, 1);

    layoutPrincipal->addLayout(layoutCentral, 3); // Ocupa el triple de ancho que los laterales

    // =========================================================
    // 3. PANEL DERECHO: Métricas Estructurales
    // =========================================================
    QVBoxLayout *layoutDerecho = new QVBoxLayout();
    QGroupBox *grupoMetricas = new QGroupBox("Métricas Estructurales");
    QVBoxLayout *layoutGrupoMetricas = new QVBoxLayout(grupoMetricas);
    layoutGrupoMetricas->setAlignment(Qt::AlignTop);

    lblTotalPaginas = new QLabel("📄 Páginas Totales: 0");
    lblProfundidadMax = new QLabel("🔀 Profundidad Máxima: 0");
    lblMasEnlaces = new QLabel("🔗 Página con más Enlaces:\nN/A (0)");
    lblTamanoTotal = new QLabel("💾 Tamaño Estimado:\n0.0 MB");

    // Damos un poco de margen y estilo a las métricas
    QString estiloMetrica = "font-size: 12px; margin-bottom: 15px;";
    lblTotalPaginas->setStyleSheet(estiloMetrica);
    lblProfundidadMax->setStyleSheet(estiloMetrica);
    lblMasEnlaces->setStyleSheet(estiloMetrica);
    lblTamanoTotal->setStyleSheet(estiloMetrica);

    layoutGrupoMetricas->addWidget(lblTotalPaginas);
    layoutGrupoMetricas->addWidget(lblProfundidadMax);
    layoutGrupoMetricas->addWidget(lblMasEnlaces);
    layoutGrupoMetricas->addWidget(lblTamanoTotal);

    layoutDerecho->addWidget(grupoMetricas);
    layoutPrincipal->addLayout(layoutDerecho, 1);

    // =========================================================
    // Conexión de Señales y Slots (Eventos)
    // =========================================================
    connect(btnIniciar, &QPushButton::clicked, this, &VentanaPrincipal::iniciarMapeo);
    connect(btnDetener, &QPushButton::clicked, this, &VentanaPrincipal::detenerMapeo);
    connect(btnBuscar, &QPushButton::clicked, this, &VentanaPrincipal::buscarPalabra);
}

// Implementación vacía de los slots (Aquí llamarán a su Capa de Negocio luego)
void VentanaPrincipal::iniciarMapeo() {
    lblEstado->setText("Estado: Mapeando...");
    btnIniciar->setEnabled(false);
    btnDetener->setEnabled(true);
    barraProgreso->setValue(10); // Ejemplo visual
}

void VentanaPrincipal::detenerMapeo() {
    lblEstado->setText("Estado: Detenido.");
    btnIniciar->setEnabled(true);
    btnDetener->setEnabled(false);
    barraProgreso->setValue(0);
}

void VentanaPrincipal::buscarPalabra() {
    listaResultadosBusqueda->clear();
    QString palabra = txtBuscar->text();
    if(palabra.isEmpty()) return;

    listaResultadosBusqueda->addItem("Buscando ruta para: " + palabra + "...");
}
