#include "VentanaPrincipal.h"
#include "AdaptadorGrafoArbol.h"
#include "gestorarchivos.h"
#include "AnalizadorMetricas.h"
#include "MetricasSitio.h"
#include <QFormLayout>
#include <QHeaderView>

#include <QStringlist>
#include <qcoreapplication.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>

#include <QDebug>

VentanaPrincipal::VentanaPrincipal(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Analizador de Sitios Web v1.0");
    resize(1050, 600); // Tamaño inicial basado en tu imagen
    //inicializar objetos
    grafo = new GrafoWeb();
    rastreador = new RastreadorWeb(grafo, this);
    modeloArbol = new QStandardItemModel(this);
    miIndiceInvertido = new IndiceInvertido();
    configurarInterfaz();
    //Asignar el modelo al árbol visual
    arbolEstructura->setModel(modeloArbol);

    // Conectar la capa de negocio con la interfaz gráfica
    // Conectar la capa de negocio con la interfaz gráfica (añade esta línea junto a los otros connect)
    connect(rastreador, &RastreadorWeb::enlaceDescubierto, this, &VentanaPrincipal::onEnlaceDescubierto);
    connect(rastreador, &RastreadorWeb::rastreoFinalizado, this, &VentanaPrincipal::onRastreoFinalizado);
    connect(rastreador, &RastreadorWeb::paginaDescargada, this, &VentanaPrincipal::onPaginaDescargada);
    connect(rastreador, &RastreadorWeb::error, this, [this](const QString& msg){

        lblEstado->setText(msg);
    });

    connect(rastreador, &RastreadorWeb::tiempoTranscurrido, this, &VentanaPrincipal::onTiempoTranscurrido);
    connect(btnGuardar, &QPushButton::clicked, this, &VentanaPrincipal::guardarHistorial);
    connect(btnCargar, &QPushButton::clicked, this, &VentanaPrincipal::cargarHistorial);
    connect(rastreador, &RastreadorWeb::error, this, [this](const QString& msg){
        if (msg.startsWith("CRITICO: ")) {
            // 1. Mostramos ventana roja de alerta nativa
            // Usamos mid(9) para borrar la palabra "CRITICO: " del mensaje visual
            QMessageBox::critical(this, "Error de Conexión", msg.mid(9));

            // 2. Abortamos automáticamente el mapeo en la Interfaz y el Motor
            detenerMapeo();
        } else {
            // Para errores de enlaces menores, solo actualizamos el texto de la esquina inferior
            lblEstado->setText(msg);
        }
    });
}

VentanaPrincipal::~VentanaPrincipal() {
    // Qt maneja la destrucción de los widgets hijos automáticamente
}
void VentanaPrincipal::configurarInterfaz() {
    QWidget *widgetCentral = new QWidget(this);
    setCentralWidget(widgetCentral);
    QHBoxLayout *layoutPrincipal = new QHBoxLayout(widgetCentral);

    // ¡Mira qué limpio y fácil de leer es esto ahora!
    layoutPrincipal->addLayout(crearPanelIzquierdo(), 1);
    layoutPrincipal->addLayout(crearPanelCentral(), 3);
    layoutPrincipal->addLayout(crearPanelDerecho(), 1);

    // Conexión de Señales y Slots (Eventos)
    connect(btnIniciar, &QPushButton::clicked, this, &VentanaPrincipal::iniciarMapeo);
    connect(btnDetener, &QPushButton::clicked, this, &VentanaPrincipal::detenerMapeo);
    connect(btnBuscar, &QPushButton::clicked, this, &VentanaPrincipal::buscarPalabra);

    aplicarEstilosGlobales();
    cambiarEstadoUI(ESTADO_ESPERANDO);
}

QVBoxLayout* VentanaPrincipal::crearPanelIzquierdo() {
    QVBoxLayout *layout = new QVBoxLayout();

    QGroupBox *grupoControles = new QGroupBox("Controles de Mapeo");
    grupoControles->setMaximumWidth(300);
    QVBoxLayout *layoutControles = new QVBoxLayout(grupoControles);

    layoutControles->addWidget(new QLabel("URL Inicial:"));
    txtUrl = new QLineEdit();
    txtUrl->setPlaceholderText("https://ejemplo.com");
    layoutControles->addWidget(txtUrl);

    layoutControles->addWidget(new QLabel("Profundidad Máxima (0 = ilimitada):"));
    spinProfundidad = new QSpinBox();
    spinProfundidad->setRange(0, 100);
    spinProfundidad->setValue(3);
    layoutControles->addWidget(spinProfundidad);

    layoutControles->addWidget(new QLabel("Peticiones Simultáneas (1 a 5):"));
    spinConcurrencia = new QSpinBox();
    spinConcurrencia->setRange(1, 5); // 1 para probar la lentitud, máximo 5 por seguridad
    spinConcurrencia->setValue(4);    // Valor inicial de 4, como pediste
    layoutControles->addWidget(spinConcurrencia);

    btnIniciar = new QPushButton("▶ Iniciar Mapeo");
    btnIniciar->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    btnDetener = new QPushButton("■ Detener Mapeo");
    btnDetener->setEnabled(false);

    layoutControles->addWidget(btnIniciar);
    layoutControles->addWidget(btnDetener);
    layout->addWidget(grupoControles);

    QGroupBox *grupoFiltros = new QGroupBox("Filtros de Contenido (Ignorar)");
    grupoFiltros->setMaximumWidth(300);
    QVBoxLayout *layoutFiltros = new QVBoxLayout(grupoFiltros);
    listaFiltros = new QListWidget();
    listaFiltros->addItems({"Imágenes", "Videos", "Audio", "Documentos", "CSS", "Scripts", "Archivos Comprimidos"});
    listaFiltros->setEnabled(false);
    layoutFiltros->addWidget(listaFiltros);
    layout->addWidget(grupoFiltros);

    barraProgreso = new QProgressBar();
    barraProgreso->setValue(0);
    barraProgreso->setMaximumWidth(300);
    lblEstado = new QLabel("Estado: Esperando...");
    lblEstado->setMaximumWidth(300);
    lblEstado->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    layout->addWidget(barraProgreso);
    layout->addWidget(lblEstado);

    return layout;
}
QVBoxLayout* VentanaPrincipal::crearPanelCentral() {
    QVBoxLayout *layout = new QVBoxLayout();

    QGroupBox *grupoEstructura = new QGroupBox("Estructura del Sitio (Mismo Dominio)");
    QVBoxLayout *layoutEstructura = new QVBoxLayout(grupoEstructura);
    grupoEstructura->setFixedWidth(650);
     grupoEstructura->setFixedHeight(400);
    arbolEstructura = new QTreeView();
     arbolEstructura->setUniformRowHeights(true);

     arbolEstructura->setAnimated(false);
    arbolEstructura->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    arbolEstructura->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    arbolEstructura->header()->setStretchLastSection(false);
    layoutEstructura->addWidget(arbolEstructura);
    layout->addWidget(grupoEstructura, 2);

    QGroupBox *grupoBusqueda = new QGroupBox("Búsqueda por Palabra Clave");
    QVBoxLayout *layoutBusqueda = new QVBoxLayout(grupoBusqueda);
    QHBoxLayout *layoutBuscador = new QHBoxLayout();

    txtBuscar = new QLineEdit();
    txtBuscar->setPlaceholderText("Ingrese palabra clave a buscar...");

    cmbFiltroBusqueda = new QComboBox();
    cmbFiltroBusqueda->addItem("Global", "CONTENIDO");
    cmbFiltroBusqueda->addItem("Solo en la URL", "URL");

    btnBuscar = new QPushButton("🔍 Buscar");


    // =========================================================
    // 2. LAYOUT
    // =========================================================


    layoutBuscador->addWidget(txtBuscar);
    layoutBuscador->addWidget(cmbFiltroBusqueda);
    layoutBuscador->addWidget(btnBuscar);
    layoutBuscador->setSpacing(8);

    listaResultadosBusqueda = new QListWidget();
    listaResultadosBusqueda->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layoutBusqueda->addLayout(layoutBuscador);
    layoutBusqueda->addWidget(listaResultadosBusqueda);
    layout->addWidget(grupoBusqueda, 1);

    return layout;
}

QVBoxLayout* VentanaPrincipal::crearPanelDerecho() {
    QVBoxLayout *layout = new QVBoxLayout();

    // =========================================================
    // 1. Grupo de Métricas Estructurales (Usando QFormLayout)
    // =========================================================
    QGroupBox *grupoMetricas = new QGroupBox("Métricas Estructurales");
    QFormLayout *layoutMetricas = new QFormLayout(grupoMetricas);
    grupoMetricas->setFixedWidth(300);

    // Inicialización de labels numéricos comunes
    lblTiempoEjecucion = new QLabel("00:00", this);
    lblPaginasEncontradas = new QLabel("0", this);
    lblEnlacesDetectados = new QLabel("0", this);
    lblDensidadConexiones = new QLabel("0.00", this);
    lblPaginasSumidero = new QLabel("0", this);
    lblTamanoDescargado = new QLabel("0 KB", this);

    // Configuración compacta de la Métrica 1 (Enlaces Salientes)
    lblPaginaMasConectada = new QLabel(this);
    lblPaginaMasConectada->setWordWrap(true);
    lblPaginaMasConectada->setTextFormat(Qt::RichText);
    lblPaginaMasConectada->setText(
        "<span style='color: #2d3748; font-weight: bold;'>Página con más salidas:</span><br>"
        "<small style='color: #718096; font-size: 10px;'>Ninguna</small>"
        );

    // Configuración compacta de la Métrica 2 (Enlaces Entrantes) - NUEVA
    lblPaginaMasReferenciada = new QLabel(this);
    lblPaginaMasReferenciada->setWordWrap(true);
    lblPaginaMasReferenciada->setTextFormat(Qt::RichText);
    lblPaginaMasReferenciada->setText(
        "<span style='color: #2d3748; font-weight: bold;'>Página más referenciada:</span><br>"
        "<small style='color: #718096; font-size: 10px;'>Ninguna (0 enlaces)</small>"
        );

    // Columnas estándar del formulario
    layoutMetricas->addRow("Tiempo de ejecución:", lblTiempoEjecucion);
    layoutMetricas->addRow("Páginas encontradas:", lblPaginasEncontradas);
    layoutMetricas->addRow("Enlaces detectados:", lblEnlacesDetectados);
    layoutMetricas->addRow("Páginas sumidero:", lblPaginasSumidero);
    layoutMetricas->addRow("Densidad de conexiones:", lblDensidadConexiones);
    layoutMetricas->addRow("Tamaño total:", lblTamanoDescargado);

    // Filas de ancho completo en la parte inferior para los enlaces
    layoutMetricas->addRow(lblPaginaMasConectada);
    layoutMetricas->addRow(lblPaginaMasReferenciada);

    grupoMetricas->setMaximumWidth(270); // Blindaje visual definitivo
    layout->addWidget(grupoMetricas);

    // =========================================================
    // 2. Grupo de Archivo (Cargar y Guardar de la Capa de Datos)
    // =========================================================
    QGroupBox *grupoArchivo = new QGroupBox("Capa de Datos");
    QVBoxLayout *layoutControles = new QVBoxLayout(grupoArchivo);

    btnGuardar = new QPushButton("💾 Guardar Grafo");
    btnCargar = new QPushButton("📂 Cargar Grafo");

    btnGuardar->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    btnCargar->setStyleSheet("background-color: #3498db; color: white; font-weight: bold;");

    // Agregamos los botones a su grupo
    layoutControles->addWidget(btnGuardar);
    layoutControles->addWidget(btnCargar);

    layout->addWidget(grupoArchivo);

    // El "Stretch" empuja todo hacia arriba para que no quede flotando en el medio
    layout->addStretch();

    return layout; // Retornamos el layout limpio a la ventana principal
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
    ultimoTiempoRastreo = "00:00";
    totalBytesContados = 0;
    lblTiempoEjecucion->setText(" 00:00");
    lblPaginasEncontradas->setText("0");
    lblEnlacesDetectados->setText("0");
    lblDensidadConexiones->setText("0.00");
    lblTamanoDescargado->setText("0 KB");
    lblPaginaMasConectada->setText("Calculando...");
    lblPaginaMasReferenciada->setText("Calculando...");
    lblEstado->setText("Estado: Mapeando...");
    lblTiempoEjecucion->setText("Calculando...");
    cambiarEstadoUI(ESTADO_MAPEANDO);

    miIndiceInvertido->limpiar();
    modeloArbol->clear(); // Limpiar árbol anterior

    qDebug() << "\n=====================================================";
    qDebug() << "[UI] INICIANDO MAPEO";
    qDebug() << "[UI] URL Inicial:" << urlInicial;
    qDebug() << "[UI] Límite de Profundidad:" << spinProfundidad->value();
    qDebug() << "[UI] Concurrencia (Hilos):" << spinConcurrencia->value();
    qDebug() << "=====================================================\n";

    //  le damos la orden a la Capa de Negocio
    rastreador->iniciarRastreo(urlInicial, spinProfundidad->value(), spinConcurrencia->value());
}

void VentanaPrincipal::detenerMapeo() {
    rastreador->detenerRastreo();


    lblEstado->setText("Estado: Detenido por el usuario. Rescatando datos parciales...");
    cambiarEstadoUI(ESTADO_CANCELADO);


    QString urlBase = txtUrl->text().trimmed();
    if(!urlBase.startsWith("http")) urlBase = "https://" + urlBase;

    arbolEstructura->setUpdatesEnabled(false);
    AdaptadorGrafoArbol::poblarModelo(grafo, modeloArbol, urlBase);
    arbolEstructura->setUpdatesEnabled(true);
    arbolEstructura->expandToDepth(0);

    ultimoTiempoRastreo = "Cancelado";
    actualizarPanelMetricas();

    lblEstado->setText("Estado: Detenido. Datos parciales mostrados.");
}

void VentanaPrincipal::buscarPalabra() {
    listaResultadosBusqueda->clear();
    QString palabraObjetivo = txtBuscar->text().trimmed();
    QString urlInicial = txtUrl->text().trimmed();

    if (palabraObjetivo.isEmpty() || urlInicial.isEmpty()) return;
    if (urlInicial.endsWith("/")) urlInicial.chop(1);

    // =========================================================
    // 1. EL BFS ENRUTA PRIMERO
    // Esto hace el recorrido y nos regala todas las URLs conocidas
    // =========================================================
    QHash<QString, int> mapaDistancias;
    QHash<QString, QString> mapaPadres;
    grafo->calcularRutasDesdeRaiz(urlInicial, mapaDistancias, mapaPadres);



    // =========================================================
    // 2. EL CONDICIONAL DE BÚSQUEDA (Reutilizando funciones)
    // =========================================================
    // === USAMOS UN QSET PARA HACERLO ULTRA VELOZ Y EVITAR DUPLICADOS NATIVAMENTE ===
    QSet<QString> conjuntoResultados;
    QString modoBusqueda = cmbFiltroBusqueda->currentData().toString();

    // PASO A: Si es "Sin Filtro", cargamos el contenido primero
    if (modoBusqueda == "CONTENIDO") {
        QStringList contenidoEncontrado = miIndiceInvertido->buscar(palabraObjetivo);
        for(const QString& url : contenidoEncontrado) {
            conjuntoResultados.insert(url);
        }
    }

    //  FOR LIBRE Se ejecuta de forma lineal para ambos casos
    for (const QString& url : mapaDistancias.keys()) {
        if (url.contains(palabraObjetivo, Qt::CaseInsensitive)) {
            // QSet inserta instantáneamente. Si ya existía por el PASO A,
            // simplemente lo ignora de forma segura. ¡Cero duplicados, cero lag!
            conjuntoResultados.insert(url);
        }
    }

    // Convertimos el conjunto final a una lista
    QList<QString> urlsDestino = conjuntoResultados.values();

    if (urlsDestino.isEmpty()) {
        listaResultadosBusqueda->addItem("❌ No se encontraron coincidencias para la búsqueda.");
        return;
    }

    // =========================================================
    //  FUSIÓN Y TRAZADO JERÁRQUICO
    // =========================================================
    bool algunaRutaValida = false;

    for (const QString& urlDestino : urlsDestino) {
        if (!mapaDistancias.contains(urlDestino) && urlDestino != urlInicial) {
            continue;
        }

        // DELEGACIÓN PURA: El Grafo calcula la ruta matemática, nosotros solo la dibujamos
        QStringList ruta = grafo->reconstruirRuta(urlInicial, urlDestino, mapaPadres);
        if (ruta.isEmpty()) continue;

        algunaRutaValida = true;
        int clics = mapaDistancias.value(urlDestino, 0);

        listaResultadosBusqueda->addItem(QString("📌 [Destino Encontrado | Clics mínimos: %1]").arg(clics));

        for (int i = 0; i < ruta.size(); ++i) {
            QString espaciado = QString("   ").repeated(i);
            QString vineta = (i == 0) ? "🌐 " : "↳ ";
            listaResultadosBusqueda->addItem(espaciado + vineta + ruta[i]);
        }
        listaResultadosBusqueda->addItem("--------------------------------------------------------------------------------");
    }

}
void VentanaPrincipal::onEnlaceDescubierto(const QString& url) {
    // 1. Tomamos el tamaño actual que tiene el texto del estado
    QFontMetrics metricas(lblEstado->font());

    // 2. Si la URL es más ancha que el espacio disponible, le pone "..." al final
    // Le restamos unos 20 pixeles por margen de seguridad
    QString textoCortado = metricas.elidedText("Encontrado: " + url, Qt::ElideRight, 290);

    // 3. Mostramos el texto cortado en la interfaz
    lblEstado->setText(textoCortado);

    QCoreApplication::processEvents();
}

void VentanaPrincipal::onRastreoFinalizado() {
    lblEstado->setText("Estado: Mapeo completado. Analizando métricas...");
   cambiarEstadoUI(ESTADO_FINALIZADO);
    // DELEGACIÓN: El adaptador se encarga de convertir el Grafo al Árbol
    QString urlBase = txtUrl->text().trimmed();
    if(!urlBase.startsWith("http")) urlBase = "https://" + urlBase;
    arbolEstructura->setUpdatesEnabled(false);
    AdaptadorGrafoArbol::poblarModelo(grafo, modeloArbol, urlBase);
    arbolEstructura->setUpdatesEnabled(true);
   arbolEstructura->expandToDepth(0);

    actualizarPanelMetricas();

    lblEstado->setText("Sistema listo. En espera.");

}

void VentanaPrincipal::onPaginaDescargada(const QString& url, const QString& html) {
    // 1. Normalizamos la URL (garantizamos que NO tenga slash final)
    QString urlLimpia = url;
    if (urlLimpia.endsWith("/")) {
        urlLimpia.chop(1);
    }

    // 2. Alimentamos el índice invertido con la URL limpia
    // y el HTML que el rastreador acaba de descargar.
    if (miIndiceInvertido) {
        miIndiceInvertido->indexarPagina(urlLimpia, html);
    }
    totalBytesContados += html.toUtf8().size();
    actualizarPanelMetricas();
}

void VentanaPrincipal::onTiempoTranscurrido(const QString& tiempoStr) {
    ultimoTiempoRastreo = tiempoStr;
    actualizarPanelMetricas();
}

void VentanaPrincipal::actualizarPanelMetricas() {
    // Evita calcular si el grafo está vacío


    // === 2. BARRERA DE SEGURIDAD PARA EL GRAFO ===
    // Si el grafo está vacío (segundos iniciales), salimos AQUÍ de forma segura.
    // El reloj ya se pintó, así que el usuario verá el segundero moverse en vivo.
    if (!grafo || grafo->cantidadNodos() == 0) {
        lblTiempoEjecucion->setText(QString("00:00"));
        lblPaginasEncontradas->setText("Páginas encontradas: 0");
        lblEnlacesDetectados->setText("Enlaces detectados: 0");
        lblPaginasSumidero->setText("0");
        lblDensidadConexiones->setText("0.00");
        lblTamanoDescargado->setText("Tamaño descargado: 0 Bytes");

    }else{

    // Llamamos a la Capa de Negocio (AnalizadorMetricas)
    MetricasSitio reporte = AnalizadorMetricas::generarReporte(grafo, ultimoTiempoRastreo, totalBytesContados);
    // Volcamos a la UI
    lblTiempoEjecucion->setText(QString(" %1").arg(reporte.tiempoEjecucion));
    lblPaginasEncontradas->setText(QString::number(reporte.paginasEncontradas));
    lblEnlacesDetectados->setText(QString::number(reporte.enlacesDetectados));

    // Inyectamos los sumideros
    lblPaginasSumidero->setText(QString::number(reporte.paginasSumidero));

    lblDensidadConexiones->setText(QString::number(reporte.densidadConexiones, 'f', 2));
    lblTamanoDescargado->setText(reporte.tamanoTotal);

    // Inyectamos HTML para la página con más Salidas (Métrica 1)
    QString formatoSalidas = QString(
                                 "<span style='color: #2d3748; font-weight: bold;'>Página con más salidas:</span><br>"
                                 "<span style='color: #4a5568; font-size: 10px; font-weight: normal;'>%1</span>"
                                 ).arg(reporte.paginaMasConectada);
    lblPaginaMasConectada->setText(formatoSalidas);

    // Inyectamos HTML para la página más Referenciada (Métrica 2)
    QString formatoEntradas = QString(
                                  "<span style='color: #2d3748; font-weight: bold;'>Página más referenciada:</span><br>"
                                  "<span style='color: #4a5568; font-size: 10px; font-weight: normal;'>%1 <b>(%2 enlaces)</b></span>"
                                  ).arg(reporte.paginaMasReferenciada).arg(reporte.maxEnlacesRecibidos);
    lblPaginaMasReferenciada->setText(formatoEntradas);

    }
}


void VentanaPrincipal::guardarHistorial() {
    if (grafo->cantidadNodos() == 0) {
        QMessageBox::warning(this, "Guardar Grafo", "El grafo está vacío. Mapea un sitio web primero.");
        return;
    }

    // 1. GENERAR NOMBRE SUGERIDO (Abreviatura + Fecha + Hora)
    // Ejemplo de salida: "MapaWeb_07-06-2026_21-30-15.txt"
    QString fechaHoraArchivo = QDateTime::currentDateTime().toString("dd-MM-yyyy_HH-mm-ss");
    QString nombreSugerido = "MapaWeb_" + fechaHoraArchivo + ".txt";

    // 2. VENTANA DE WINDOWS PARA GUARDAR
    // Al pasar 'nombreSugerido' en el 3er parámetro, Windows lo escribirá automáticamente
    // en la barra de nombre de archivo, pero permitiéndote elegir la carpeta.
    QString ruta = QFileDialog::getSaveFileName(
        this,
        "Guardar Estructura de Grafo",
        nombreSugerido, // <-- AQUÍ SUCEDE LA MAGIA
        "Archivos de texto (*.txt);;Todos los archivos (*)"
        );

    if (ruta.isEmpty()) return; // El usuario canceló o cerró la ventana

    QString fechaHoraReporte = QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss AP");

    // Recopilar la información usando los labels vigentes
    QString paginasEnc = lblPaginasEncontradas->text();
    QString enlacesDet = lblEnlacesDetectados->text();
    QString sumiderosTxt = lblPaginasSumidero->text();
    QString densidadCon = lblDensidadConexiones->text();
    QString tamanoDesc = lblTamanoDescargado->text();
    QString masConectada = lblPaginaMasConectada->text();

    QString metadatos =
        "# Fecha de Mapeo         : " + fechaHoraReporte + "\n" +
        "# URL Raíz               : " + txtUrl->text().trimmed() + "\n" +
        "# Límite Profundidad     : " + QString::number(spinProfundidad->value()) + " niveles\n" +
        "# Hilos Concurrentes     : " + QString::number(spinConcurrencia->value()) + "\n" +
        "#\n" +
        "# --- RENDIMIENTO Y MÉTRICAS ---\n" +
        "# Páginas encontradas    : " + paginasEnc + "\n" +
        "# Enlaces detectados     : " + enlacesDet + "\n" +
        "# Páginas sumidero       : " + sumiderosTxt + "\n" +
        "# Densidad de conexiones : " + densidadCon + "\n" +
        "# Tamaño del contenido   : " + tamanoDesc + "\n" +
        "# Página más conectada   : " + masConectada + "\n" +
        "# Tiempo de ejecución    : " + ultimoTiempoRastreo;

    // 4. GUARDAR Y NOTIFICAR
    if (GestorArchivos::guardarGrafo(ruta, *grafo, metadatos)) {
        QFileInfo infoArchivo(ruta);
        QString nombreFinal = infoArchivo.fileName();

        QString mensajeExito = QString(
                                   "¡El mapa del sitio web se ha guardado exitosamente!\n\n"
                                   "📄 Archivo: %1\n"
                                   "🕒 Fecha y Hora: %2\n"
                                   "🌐 Nodos guardados: %3"
                                   ).arg(nombreFinal, fechaHoraReporte, QString::number(grafo->cantidadNodos()));

        QMessageBox::information(this, "Guardado Exitoso", mensajeExito);
    } else {
        QMessageBox::critical(this, "Error", "No se pudo escribir en el archivo seleccionado.\nVerifique permisos o si el archivo está en uso.");
    }
}
void VentanaPrincipal::cargarHistorial() {

    if (grafo->cantidadNodos() > 0) {
        QMessageBox::StandardButton respuesta;
        respuesta = QMessageBox::question(this, "Datos existentes",
                                          "Ya tienes un sitio mapeado en memoria.\n"
                                          "¿Estás seguro de que deseas sobrescribirlo con un nuevo archivo? "
                                          "Cualquier progreso no guardado se perderá.",
                                          QMessageBox::Yes | QMessageBox::No);
        if (respuesta == QMessageBox::No) {
            return; // El usuario se arrepintió, cancelamos la carga.
        }
    }
    QString ruta = QFileDialog::getOpenFileName(
        this, "Cargar Estructura de Grafo", "", "Archivos de texto (*.txt);;Todos los archivos (*)"
        );

    if (ruta.isEmpty()) return;

    QString urlRaizRecuperada; // Aquí guardaremos la URL que el archivo nos dicte

    // Llamamos a la capa de datos pasando nuestra nueva variable por referencia
    if (GestorArchivos::cargarGrafo(ruta, *grafo, urlRaizRecuperada)) {

        // 1. ACTUALIZAMOS LA INTERFAZ CON LA URL DEL ARCHIVO
        if (!urlRaizRecuperada.isEmpty()) {
            txtUrl->setText(urlRaizRecuperada);
        } else {
            urlRaizRecuperada = txtUrl->text().trimmed(); // Fallback de seguridad
        }
        arbolEstructura->setUpdatesEnabled(false);
        // 2. DIBUJAMOS EL ÁRBOL
        AdaptadorGrafoArbol::poblarModelo(grafo, modeloArbol, urlRaizRecuperada);
        arbolEstructura->setUpdatesEnabled(true);
        arbolEstructura->expandToDepth(0);

        // 3. LIMPIEZA TOTAL DE UI (Evitar datos basura del escaneo anterior)
        miIndiceInvertido->limpiar();
        listaResultadosBusqueda->clear();
        listaResultadosBusqueda->addItem("📂 Grafo recuperado desde archivo plano.");
        listaResultadosBusqueda->addItem("⚠️ Nota: Las búsquedas de palabras requieren un rastreo en vivo.");

        lblTiempoEjecucion->setText("⏱️ Tiempo de ejecución:\nCargado de archivo");
        barraProgreso->setValue(100);
        lblEstado->setText("Estado: Archivo cargado.");

        // Actualizar métricas básicas recuperables en los  labels
        lblPaginasEncontradas->setText(QString::number(grafo->cantidadNodos()));
        lblEnlacesDetectados->setText(QString::number(grafo->cantidadAristas()));

        double dens = (grafo->cantidadNodos() > 0) ? (double)grafo->cantidadAristas() / grafo->cantidadNodos() : 0.0;
        lblDensidadConexiones->setText(QString::number(dens, 'f', 2));

        lblTamanoDescargado->setText("N/A (Historial)");
        lblPaginaMasConectada->setText("N/A (Historial)");
        QMessageBox::information(this, "Carga Exitosa",
                                 QString("Se ha restaurado el grafo desde el archivo.\n\nRaíz: %1\nNodos: %2\nEnlaces: %3")
                                     .arg(urlRaizRecuperada)
                                     .arg(grafo->cantidadNodos())
                                     .arg(grafo->cantidadAristas())
                                 );

    } else {
        QMessageBox::critical(this, "Error", "El archivo seleccionado no tiene un formato válido o está corrupto.");
    }
}
// =========================================================
// MÁQUINA DE ESTADOS (Controlador centralizado de UI)
// =========================================================
void VentanaPrincipal::cambiarEstadoUI(EstadoAplicacion estado) {
    bool hayDatosGuardables = (grafo && grafo->cantidadNodos() > 0);

    switch (estado) {
    case ESTADO_ESPERANDO:
        btnIniciar->setEnabled(true);
        btnDetener->setEnabled(false);
        btnGuardar->setEnabled(hayDatosGuardables);
        btnCargar->setEnabled(true);
        barraProgreso->setValue(0);
        break;

    case ESTADO_MAPEANDO:
        btnIniciar->setEnabled(false);
        btnDetener->setEnabled(true);
        btnGuardar->setEnabled(false); // Bloqueo de seguridad
        btnCargar->setEnabled(false);  // Bloqueo de seguridad
        barraProgreso->setMinimum(0);
        barraProgreso->setMaximum(0);  // Modo cargando (infinito)
        break;

    case ESTADO_FINALIZADO:
        btnIniciar->setEnabled(true);
        btnDetener->setEnabled(false);
        btnGuardar->setEnabled(true);
        btnCargar->setEnabled(true);
        barraProgreso->setMaximum(100);
        barraProgreso->setValue(100);
        break;

    case ESTADO_CANCELADO:
        btnIniciar->setEnabled(true);
        btnDetener->setEnabled(false);
        btnGuardar->setEnabled(true);
        btnCargar->setEnabled(true);
        barraProgreso->setMaximum(100);
        barraProgreso->setValue(0);
        break;
    }
}

// =========================================================
// HOJA DE ESTILOS CENTRALIZADA (Limpia los paneles visuales)
// =========================================================
void VentanaPrincipal::aplicarEstilosGlobales() {
    // Botones Izquierdos
    btnIniciar->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");

    // Botones Archivo
    btnGuardar->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    btnCargar->setStyleSheet("background-color: #3498db; color: white; font-weight: bold;");

    // Buscador y Filtros
    txtBuscar->setStyleSheet(
        "QLineEdit { border: 1px solid #B0BEC5; border-radius: 4px; padding: 6px 10px; font-size: 13px; background-color: #111111; }"
        "QLineEdit:focus { border: 2px solid #2196F3; }"
        );

    cmbFiltroBusqueda->setStyleSheet(
        "QComboBox { border: 1px solid #B0BEC5; border-radius: 4px; padding: 5px 10px; font-size: 13px; background-color: #111111; min-width: 160px; }"
        "QComboBox:focus { border: 2px solid #2196F3; }"
        );

    btnBuscar->setStyleSheet(
        "QPushButton { background-color: #2196F3; color: white; border: none; border-radius: 4px; padding: 6px 16px; font-weight: bold; font-size: 13px; }"
        "QPushButton:hover { background-color: #1976D2; }"
        );
}
