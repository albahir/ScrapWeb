#include "VentanaPrincipal.h"
#include "AdaptadorGrafoArbol.h"
#include "gestorarchivos.h"
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
    connect(rastreador, &RastreadorWeb::tiempoTranscurrido, this, [this](const QString& tiempoStr){
        lblTiempoEjecucion->setText("⏱️ Tiempo de ejecución:\n" + tiempoStr);
    });
    connect(btnGuardar, &QPushButton::clicked, this, &VentanaPrincipal::guardarHistorial);
    connect(btnCargar, &QPushButton::clicked, this, &VentanaPrincipal::cargarHistorial);
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
    lblEstado = new QLabel("Estado: Esperando...");
    layout->addWidget(barraProgreso);
    layout->addWidget(lblEstado);

    return layout;
}
QVBoxLayout* VentanaPrincipal::crearPanelCentral() {
    QVBoxLayout *layout = new QVBoxLayout();

    QGroupBox *grupoEstructura = new QGroupBox("Estructura del Sitio (Mismo Dominio)");
    QVBoxLayout *layoutEstructura = new QVBoxLayout(grupoEstructura);
    arbolEstructura = new QTreeView();
    layoutEstructura->addWidget(arbolEstructura);
    layout->addWidget(grupoEstructura, 2);

    QGroupBox *grupoBusqueda = new QGroupBox("Búsqueda por Palabra Clave");
    QVBoxLayout *layoutBusqueda = new QVBoxLayout(grupoBusqueda);
    QHBoxLayout *layoutBuscador = new QHBoxLayout();

    txtBuscar = new QLineEdit();
    txtBuscar->setPlaceholderText("Ingrese palabra clave...");
    btnBuscar = new QPushButton("🔍 Buscar Palabra");

    layoutBuscador->addWidget(txtBuscar);
    layoutBuscador->addWidget(btnBuscar);

    listaResultadosBusqueda = new QListWidget();

    layoutBusqueda->addLayout(layoutBuscador);
    layoutBusqueda->addWidget(listaResultadosBusqueda);
    layout->addWidget(grupoBusqueda, 1);

    return layout;
}

QVBoxLayout* VentanaPrincipal::crearPanelDerecho() {
    QVBoxLayout *layout = new QVBoxLayout();

    // =========================================================
    // 1. Grupo de Métricas Estructurales
    // =========================================================
    QGroupBox *grupoMetricas = new QGroupBox("Métricas Estructurales");
    QVBoxLayout *layoutMetricas = new QVBoxLayout(grupoMetricas);
    layoutMetricas->setAlignment(Qt::AlignTop);

    lblTotalPaginas = new QLabel("📄 Páginas Totales: 0");
    lblProfundidadMax = new QLabel("🔀 Profundidad Máxima: 0");
    lblMasEnlaces = new QLabel("🔗 Página con más Enlaces:\nN/A (0)");
    lblTamanoTotal = new QLabel("💾 Tamaño Estimado:\n0.0 MB");
    lblTiempoEjecucion = new QLabel("⏱️ Tiempo de ejecución:\n0.00s");

    // Estilos
    QString estiloMetrica = "font-size: 12px; margin-bottom: 15px;";
    lblTotalPaginas->setStyleSheet(estiloMetrica);
    lblProfundidadMax->setStyleSheet(estiloMetrica);
    lblMasEnlaces->setStyleSheet(estiloMetrica);
    lblTamanoTotal->setStyleSheet(estiloMetrica);
    lblTiempoEjecucion->setStyleSheet(estiloMetrica);

    // Agregamos las métricas a su grupo
    layoutMetricas->addWidget(lblTotalPaginas);
    layoutMetricas->addWidget(lblProfundidadMax);
    layoutMetricas->addWidget(lblMasEnlaces);
    layoutMetricas->addWidget(lblTamanoTotal);
    layoutMetricas->addWidget(lblTiempoEjecucion);

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

    lblEstado->setText("Estado: Mapeando...");
    lblTiempoEjecucion->setText("⏱️ Tiempo de ejecución:\nCalculando...");
    btnIniciar->setEnabled(false);
    btnDetener->setEnabled(true);
    barraProgreso->setMinimum(0);
    barraProgreso->setMaximum(0); // Hace que la barra se mueva indefinidamente

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
    lblEstado->setText("Estado: Detenido.");
    btnIniciar->setEnabled(true);
    btnDetener->setEnabled(false);
    barraProgreso->setValue(0);

}

void VentanaPrincipal::buscarPalabra() {
    listaResultadosBusqueda->clear();
    QString palabraObjetivo = txtBuscar->text().trimmed();
    QString urlInicial = txtUrl->text().trimmed();

    if (palabraObjetivo.isEmpty() || urlInicial.isEmpty()) return;
    if (urlInicial.endsWith("/")) urlInicial.chop(1);

    // 1. EL ÍNDICE INVERTIDO FILTRA: Páginas con la palabra en su contenido HTML
    QStringList urlsConPalabra = miIndiceInvertido->buscar(palabraObjetivo);

    if (urlsConPalabra.isEmpty()) {
        listaResultadosBusqueda->addItem("❌ No se encontraron páginas con esa palabra clave.");
        return;
    }

    // 2. EL BFS ENRUTA: Calculamos distancias y padres desde la raíz una sola vez
    QHash<QString, int> mapaDistancias;
    QHash<QString, QString> mapaPadres;
    grafo->calcularRutasDesdeRaiz(urlInicial, mapaDistancias, mapaPadres);

    bool algunaRutaValida = false;

    // 3. FUSIÓN Y TRAZADO JERÁRQUICO
    for (const QString& urlDestino : urlsConPalabra) {
        // Ignorar si la URL no es alcanzable desde la raíz seleccionada
        if (!mapaDistancias.contains(urlDestino) && urlDestino != urlInicial) {
            continue;
        }

        algunaRutaValida = true;
        QStringList ruta;
        QString pasoActual = urlDestino;

        // Trazamos el camino hacia atrás usando el diccionario de padres del BFS
        while (pasoActual != urlInicial && mapaPadres.contains(pasoActual)) {
            ruta.prepend(pasoActual); // Insertamos al inicio para ordenar de raíz a destino
            pasoActual = mapaPadres.value(pasoActual);
        }
        ruta.prepend(urlInicial); // Añadimos el nodo raíz al inicio

        int clics = mapaDistancias.value(urlDestino, 0);

        // --- ENCABEZADO DEL RESULTADO ---
        QString infoMetricas = QString("📌 [Destino Encontrado | Clics mínimos: %1]").arg(clics);
        listaResultadosBusqueda->addItem(infoMetricas);

        // --- IMPRESIÓN JERÁRQUICA (Tu lógica adaptada) ---
        for (int i = 0; i < ruta.size(); ++i) {
            QString espaciado = QString("   ").repeated(i); // 3 espacios por nivel de profundidad
            QString vineta = (i == 0) ? "🌐 " : "↳ ";

            // Añadimos cada eslabón de la ruta como un elemento individual abajo del otro
            listaResultadosBusqueda->addItem(espaciado + vineta + ruta[i]);
        }

        // Añadimos una línea sutil de separación para no amontonar si hay más de una ruta
        listaResultadosBusqueda->addItem("--------------------------------------------------------------------------------");
    }

    if (!algunaRutaValida) {
        listaResultadosBusqueda->addItem("⚠ La palabra existe en el índice, pero ninguna página es alcanzable desde la raíz.");
    }
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
    lblEstado->setText("Estado: Mapeo completado.");
    btnIniciar->setEnabled(true);
    btnDetener->setEnabled(false);
    barraProgreso->setMaximum(100);
    barraProgreso->setValue(100);

    // DELEGACIÓN: El adaptador se encarga de convertir el Grafo al Árbol
    QString urlBase = txtUrl->text().trimmed();
    if(!urlBase.startsWith("http")) urlBase = "https://" + urlBase;

    AdaptadorGrafoArbol::poblarModelo(grafo, modeloArbol, urlBase);
    arbolEstructura->expandAll();

    // Actualización de métricas
    lblTotalPaginas->setText("📄 Páginas Totales: " + QString::number(grafo->cantidadNodos()));
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

    // 3. RECOPILACIÓN DE METADATOS (Para el contenido del archivo)
    QString fechaHoraReporte = QDateTime::currentDateTime().toString("dd/MM/yyyy - hh:mm:ss AP");
    QString masEnlaces = lblMasEnlaces->text().replace("\n", " ");
    QString tiempoEjec = lblTiempoEjecucion->text().replace("\n", " ");
    QString tamanoEst = lblTamanoTotal->text().replace("\n", " ");

    QString metadatos =
        "# Fecha de Mapeo       : " + fechaHoraReporte + "\n" +
        "# URL Raíz             : " + txtUrl->text().trimmed() + "\n" +
        "# Límite Profundidad   : " + QString::number(spinProfundidad->value()) + " niveles\n" +
        "# Hilos Concurrentes   : " + QString::number(spinConcurrencia->value()) + "\n" +
        "#\n" +
        "# --- RENDIMIENTO Y MÉTRICAS ---\n" +
        "# " + lblTotalPaginas->text() + "\n" +
        "# " + lblProfundidadMax->text() + "\n" +
        "# " + masEnlaces + "\n" +
        "# " + tamanoEst + "\n" +
        "# " + tiempoEjec;

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

        // 2. DIBUJAMOS EL ÁRBOL
        AdaptadorGrafoArbol::poblarModelo(grafo, modeloArbol, urlRaizRecuperada);
        arbolEstructura->expandAll();

        // 3. LIMPIEZA TOTAL DE UI (Evitar datos basura del escaneo anterior)
        miIndiceInvertido->limpiar();
        listaResultadosBusqueda->clear();
        listaResultadosBusqueda->addItem("📂 Grafo recuperado desde archivo plano.");
        listaResultadosBusqueda->addItem("⚠️ Nota: Las búsquedas de palabras requieren un rastreo en vivo.");

        lblTiempoEjecucion->setText("⏱️ Tiempo de ejecución:\nCargado de archivo");
        barraProgreso->setValue(100);
        lblEstado->setText("Estado: Archivo cargado.");

        // 4. ACTUALIZAR MÉTRICAS
        lblTotalPaginas->setText("📄 Páginas Totales: " + QString::number(grafo->cantidadNodos()));

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
