#include "VentanaPrincipal.h"
#include <QFormLayout>
#include <QHeaderView>
#include <QStandardItem>
#include <QQueue>
#include <QSet>
#include <QPair>
#include <qcoreapplication.h>
VentanaPrincipal::VentanaPrincipal(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Analizador de Sitios Web v1.0");
    resize(1050, 600); // Tamaño inicial basado en tu imagen
    //inicializar objetos
    grafo = new GrafoWeb();
    rastreador = new RastreadorWeb(grafo, this);
    modeloArbol = new QStandardItemModel(this);

    configurarInterfaz();
    //Asignar el modelo al árbol visual
    arbolEstructura->setModel(modeloArbol);

    // Conectar la capa de negocio con la interfaz gráfica
    connect(rastreador, &RastreadorWeb::enlaceDescubierto, this, &VentanaPrincipal::onEnlaceDescubierto);
    connect(rastreador, &RastreadorWeb::rastreoFinalizado, this, &VentanaPrincipal::onRastreoFinalizado);
    connect(rastreador, &RastreadorWeb::error, this, [this](const QString& msg){
        // Mostramos el enlace roto momentáneamente en la barra de estado
        lblEstado->setText(msg);
    });
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
    grupoControles->setMaximumWidth(300);
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
    grupoFiltros->setMaximumWidth(300);
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
    QString urlInicial = txtUrl->text().trimmed();

    // 2. Validación de vacío
    if(urlInicial.isEmpty()) {
        lblEstado->setText("Error: La URL no puede estar vacía.");
        return;
    }

    // 3. Validación de esquema (http/https)
    if(!urlInicial.startsWith("http://") && !urlInicial.startsWith("https://")) {
        urlInicial = "https://" + urlInicial; // Autocorrección para mejor UX
        txtUrl->setText(urlInicial); // Actualizamos la vista
    }

    lblEstado->setText("Estado: Mapeando...");
    btnIniciar->setEnabled(false);
    btnDetener->setEnabled(true);
    barraProgreso->setMinimum(0);
    barraProgreso->setMaximum(0); // Hace que la barra se mueva indefinidamente

    modeloArbol->clear(); // Limpiar árbol anterior

    //  le damos la orden a la Capa de Negocio
    int profundidad = spinProfundidad->value();
    rastreador->iniciarRastreo(urlInicial, profundidad);
}

void VentanaPrincipal::detenerMapeo() {
    lblEstado->setText("Estado: Detenido.");
    btnIniciar->setEnabled(true);
    btnDetener->setEnabled(false);
    barraProgreso->setValue(0);

}

void VentanaPrincipal::buscarPalabra() {
    listaResultadosBusqueda->clear();

    // Sanitización básica para la búsqueda
    QString palabra = txtBuscar->text().trimmed();

    if(palabra.isEmpty()) {
        listaResultadosBusqueda->addItem("⚠️ Por favor, ingrese una palabra válida.");
        return;
    }

    listaResultadosBusqueda->addItem("Buscando ruta para: " + palabra + "...");
    // Aquí irá la lógica de búsqueda en el grafo más adelante
}
void VentanaPrincipal::onEnlaceDescubierto(const QString& url) {
    // 1. Tomamos el tamaño actual que tiene el texto del estado
    QFontMetrics metricas(lblEstado->font());

    // 2. Si la URL es más ancha que el espacio disponible, le pone "..." al final
    // Le restamos unos 20 pixeles por margen de seguridad
    QString textoCortado = metricas.elidedText("Encontrado: " + url, Qt::ElideRight, lblEstado->width() - 20);

    // 3. Mostramos el texto cortado en la interfaz
    lblEstado->setText(textoCortado);

    QCoreApplication::processEvents();
}

void VentanaPrincipal::onRastreoFinalizado() {
    lblEstado->setText("Estado: Mapeo completado");
    btnIniciar->setEnabled(true);
    btnDetener->setEnabled(false);
    barraProgreso->setMaximum(100);
    barraProgreso->setValue(100); // Llenar la barra

    poblarArbolVisual(); // Dibujar el árbol

    // Aquí actualizarías tus métricas leyendo del grafo
    lblTotalPaginas->setText("📄 Páginas Totales: " + QString::number(grafo->cantidadNodos()));
}

void VentanaPrincipal::poblarArbolVisual() {
    modeloArbol->clear();
    modeloArbol->setHorizontalHeaderLabels({"URL"});

    QString urlInicial = txtUrl->text();
    if (!grafo->contieneNodo(urlInicial)) return;

    // Nodo raíz del QTreeView
    QStandardItem *itemRaiz = new QStandardItem(urlInicial);
    modeloArbol->appendRow(itemRaiz);

    // Usamos recursividad o cola para poblar el árbol.
    // IMPORTANTE: Usamos un QSet para evitar bucles infinitos (Página A apunta a B, y B apunta a A)
    QSet<QString> visitadosVisulamente;
    visitadosVisulamente.insert(urlInicial);

    QQueue<QPair<QString, QStandardItem*>> cola;
    cola.enqueue({urlInicial, itemRaiz});

    while (!cola.isEmpty()) {
        auto actual = cola.dequeue();
        QString urlActual = actual.first;
        QStandardItem *itemPadre = actual.second;

        QStringList adyacentes = grafo->obtenerAdyacentes(urlActual);

        for (const QString& urlHijo : adyacentes) {
            if (!visitadosVisulamente.contains(urlHijo)) {
                visitadosVisulamente.insert(urlHijo);

                QStandardItem *itemHijo = new QStandardItem(urlHijo);
                itemPadre->appendRow(itemHijo); // Añadimos la rama

                cola.enqueue({urlHijo, itemHijo});
            }
        }
    }

    arbolEstructura->expandAll(); // Expandir todo el árbol para que el usuario lo vea
}


