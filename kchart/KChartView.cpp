/* This file is part of the KDE project

   Copyright 1999-2007  Kalle Dalheimer <kalle@kde.org>
   Copyright 2005-2007  Inge Wallin <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


// Local
#include "KChartView.h"

// Qt
#include <QFile>
#include <qpainter.h>
#include <qcursor.h>
#include <QMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QPaintEvent>
#include <QGridLayout>

// KDE
#include <kicon.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kprinter.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kcomponentdata.h>
#include <kxmlguifactory.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

// KOffice
#include <KoCsvImportDialog.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShapeCreateCommand.h>
#include <KoTemplateCreateDia.h>
#include <KoToolBoxFactory.h>
#include <KoToolDockerFactory.h>
#include <KoToolManager.h>
#include <KoViewAdaptor.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoCanvasController.h>

// KChart
#include "KChartFactory.h"
#include "KChartPart.h"
#include "KChartViewAdaptor.h"
#include "KCPageLayout.h"
#include "KCPrinterDialog.h"
#include "ChartShape.h"
#include "KChartCanvas.h"

#include "prefs.h"

#include "commands/ChartTypeCommand.h"

using namespace std;


namespace KChart
{


KChartView::KChartView( KChartPart* part, QWidget* parent )
    : KoView( part, parent )
{
    setComponentData( KChartFactory::global() );
    if ( koDocument()->isReadWrite() )
        setXMLFile( "kchart.rc" );
    else
        setXMLFile( "kchart_readonly.rc" );

    m_dbus = new ViewAdaptor(this);

    m_importData  = new KAction(i18n("Import Data..."), this);
    actionCollection()->addAction("import_data", m_importData );
    connect(m_importData, SIGNAL(triggered(bool)), SLOT( importData() ));
    KAction *actionExtraCreateTemplate  = new KAction(i18n("&Create Template From Document..."), this);
    actionCollection()->addAction("extra_template", actionExtraCreateTemplate );
    connect(actionExtraCreateTemplate, SIGNAL(triggered(bool)), SLOT( extraCreateTemplate() ));

    m_edit  = new KAction(KIcon("edit"), i18n("Edit &Data..."), this);
    actionCollection()->addAction("editdata", m_edit );
    connect(m_edit, SIGNAL(triggered(bool) ), SLOT( editData() ));

    /*
    // One KToggleAction per chart type
    m_chartbars  = new KToggleAction(KIcon("chart_bar_3d"), i18n("&Bar"), this);
    actionCollection()->addAction("barschart", m_chartbars );
    connect(m_chartbars, SIGNAL(triggered(bool)), SLOT( barsChart() ));
    QActionGroup *charttypes = new QActionGroup( this );
    charttypes->addAction(m_chartbars);

    m_chartline  = new KToggleAction(KIcon("chart_line"), i18n("&Line"), this);
    actionCollection()->addAction("linechart", m_chartline );
    connect(m_chartline, SIGNAL(triggered(bool)), SLOT( lineChart() ));
    charttypes->addAction(m_chartline);

    m_chartareas  = new KToggleAction(KIcon("chart_area"), i18n("&Area"), this);
    actionCollection()->addAction("areaschart", m_chartareas );
    connect(m_chartareas, SIGNAL(triggered(bool)), SLOT( areasChart() ));
    charttypes->addAction(m_chartareas);
    m_chartareas->setEnabled(false); // TODO not supported yet

    m_charthilo  = new KToggleAction(KIcon("chart_hilo"), i18n("&HiLo"), this);
    actionCollection()->addAction("hilochart", m_charthilo );
    connect(m_charthilo, SIGNAL(triggered(bool)), SLOT( hiLoChart() ));
    charttypes->addAction(m_charthilo);
    m_charthilo->setEnabled(false); // TODO not supported yet

    m_chartbw  = new KToggleAction(KIcon("chart_boxwhisker"), i18n("Bo&x && Whiskers"), this);
    actionCollection()->addAction("bwchart", m_chartbw );
    connect(m_chartbw, SIGNAL(triggered(bool)), SLOT( bwChart() ));
    charttypes->addAction(m_chartbw);
    m_chartbw->setEnabled(false); // TODO not supported yet

    m_chartpie  = new KToggleAction(KIcon("chart_pie"), i18n("&Pie"), this);
    actionCollection()->addAction("piechart", m_chartpie );
    connect(m_chartpie, SIGNAL(triggered(bool)), SLOT( pieChart() ));
    charttypes->addAction(m_chartpie);
    m_chartring  = new KToggleAction(KIcon("chart_ring"), i18n("&Ring"), this);
    actionCollection()->addAction("ringchart", m_chartring );
    connect(m_chartring, SIGNAL(triggered(bool)), SLOT( ringChart() ));
    charttypes->addAction(m_chartring);
    m_chartpolar  = new KToggleAction(KIcon("chart_polar"), i18n("&Polar"), this);
    actionCollection()->addAction("polarchart", m_chartpolar );
    connect(m_chartpolar, SIGNAL(triggered(bool)), SLOT( polarChart() ));
    charttypes->addAction(m_chartpolar);
    */

    // the KChartCanvas class requires an instanciated KoZoomHandler, so we'll do that first
    m_zoomHandler = new KoZoomHandler;

    m_canvas = new KChartCanvas( this, part );

    connect( m_canvas, SIGNAL( documentViewRectChanged( const QRectF& ) ),
        this, SLOT( documentViewRectChanged( const QRectF& ) ) );

    m_canvasController = new KoCanvasController( this );
    m_canvasController->setCanvas( m_canvas );
    m_canvasController->setCanvasMode( KoCanvasController::Centered );
    m_canvasController->show();

    m_zoomController = new KoZoomController( m_canvasController, m_zoomHandler, actionCollection(), true );

    connect( m_canvasController, SIGNAL( moveDocumentOffset( const QPoint& ) ),
             m_canvas, SLOT( setDocumentOffset( const QPoint& ) ) );

    QGridLayout *layout = new QGridLayout;
    layout->setMargin( 0 );
    layout->addWidget( m_canvasController, 0, 0 );

    setLayout( layout );

    KoZoomAction *zoomAction = m_zoomController->zoomAction();

    // Initially, zoom is at 100%
    m_zoomController->setZoom( KoZoomMode::ZOOM_CONSTANT, 100 );
    m_zoomHandler->setZoom( 100 );

    //kDebug() << "----------------------------------------------------------------";
    //kDebug() << "Size is: " << m_zoomHandler->viewToDocument( size() );
    //kDebug() << "size() is: " << size();
    //part->shape()->setSize( m_zoomHandler->viewToDocument( size() ) );

    // This is probably a good default size for a chart.
    part->shape()->setSize( QSizeF( CM_TO_POINT( 12 ), CM_TO_POINT( 8 ) ) );
    //m_zoomController->setZoomMode( KoZoomMode::ZOOM_PAGE );

    m_zoomController->setPageSize( m_canvas->documentViewRect().size() );
    m_zoomController->setDocumentSize( m_canvas->documentViewRect().size() );

    addStatusBarItem( zoomAction->createWidget( ( QWidget* )statusBar() ), 0 );
    connect( m_zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode, double)),
             this, SLOT(zoomChanged(KoZoomMode::Mode, double)));
    m_zoomController->setZoomMode( KoZoomMode::ZOOM_PAGE );

    // initialize the configuration
    //    loadConfig();

    //KChartPart *part = (KChartPart*)koDocument();
    
    KoToolManager::instance()->addController( m_canvasController );
    KoToolManager::instance()->registerTools( actionCollection(), m_canvasController );

    KoToolDockerFactory toolDockerFactory;
    KoToolDocker *td =  dynamic_cast<KoToolDocker*>( createDockWidget( &toolDockerFactory ) );
    connect( m_canvasController, SIGNAL( toolOptionWidgetChanged(QWidget* ) ), td, SLOT( newOptionWidget( QWidget* ) ) );

    KoToolBoxFactory toolBoxFactory( m_canvasController, "KChart" );
    createDockWidget( &toolBoxFactory );
	
    // Disable some things if we can't change the data, e.g. because
    // we are inside another application that provides the data for us.
    if ( !part->canChangeValue() )
    {
        m_edit->setEnabled( false );
        m_importData->setEnabled( false );
    }

    m_canvas->shapeManager()->add( part->shape() );
    connect( m_canvas->shapeManager()->selection(), SIGNAL( selectionChanged() ), this, SLOT( selectionChanged() ) );
    
    //updateGuiTypeOfChart();
}


KChartView::~KChartView()
{
//     delete m_dcop;
}


ViewAdaptor* KChartView::dbusObject()
{
    return m_dbus;
}


void KChartView::paintEvent( QPaintEvent* /*ev*/ )
{
    //QPainter painter( this );
    //QRect clipRect = 

    // ### TODO: Scaling

    // Let the document do the drawing
    // This calls KChartPart::paintContent, basically.
    //koDocument()->paintEverything( painter, rect(), this );
    //m_canvas->shapeManager()->paint( painter, KoZoomHandler(), true );
}


void KChartView::updateReadWrite( bool /*readwrite*/ )
{
#ifdef __GNUC__
#warning TODO
#endif
}



void KChartView::updateGuiTypeOfChart()
{
    // FIXME
    return;
    KChartPart *part = qobject_cast<KChartPart*>( koDocument() );

    switch(part->chartType()) {
    case BarChartType:
	m_chartbars->setChecked(true);
	break;
    case LineChartType:
	m_chartline->setChecked(true);
	break;
    case AreaChartType:
	m_chartareas->setChecked(true);
	break;
    case CircleChartType:
	m_chartpie->setChecked(true);
	break;
#if 0 //Temporarily disabled because this is a stock chart and so is BoxWhiskers
    case HiLoChartType:
	m_charthilo->setChecked(true);
	break;
#endif
    case RingChartType:
	m_chartring->setChecked(true);
	break;
    case RadarChartType:
        m_chartpolar->setChecked(true);
        break;
    case StockChartType:
        m_chartbw->setChecked( true );
        break;
    default:
	//todo
	break;
    }

    // Disable subtype configuration button if appropriate.
    updateButton();
}



void KChartView::slotRepaint()
{
    ((KChartPart*)koDocument())->setModified(true);
    update();
}


void KChartView::saveConfig()
{
    kDebug(35001) <<"Save config...";
    //((KChartPart*)koDocument())->saveConfig( KGlobal::config().data() );
}


void KChartView::loadConfig()
{
    kDebug(35001) <<"Load config...";

    //KGlobal::config()->reparseConfiguration();
    //((KChartPart*)koDocument())->loadConfig( KGlobal::config().data() );

    //updateGuiTypeOfChart();
    //refresh chart when you load config
    update();
}


void KChartView::defaultConfig()
{
    //((KChartPart*)koDocument())->defaultConfig(  );
    //updateGuiTypeOfChart();
    update();
}


void KChartView::pieChart()
{
#if 0
    if ( m_chartpie->isChecked() ) {
	forceAxisParams(false);
	KChartParams  *params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Pie );
	params->setThreeDPies(params->threeDBars());
	params->setExplodeFactor( 0 );
	params->setExplode( true );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartpie->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(CircleChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}

void KChartView::forceAxisParams(bool lineMode)
{
    Q_UNUSED( lineMode );
#if 0
    KChartParams  *params = ((KChartPart*)koDocument())->params();
    KDChartAxisParams  axisParams;
    axisParams = params->axisParams( KDChartAxisParams::AxisPosLeft );
    if(params->chartType() == KChartParams::Line)
        m_logarithmicScale = axisParams.axisCalcMode();
    if(lineMode) {
        if(m_logarithmicScale)
            axisParams.setAxisCalcMode(KDChartAxisParams::AxisCalcLogarithmic);
    } else
        axisParams.setAxisCalcMode(KDChartAxisParams::AxisCalcLinear);
    params->setAxisParams( KDChartAxisParams::AxisPosLeft, axisParams );
#endif
}

void KChartView::lineChart()
{
#if 0
    if ( m_chartline->isChecked() ) {
	forceAxisParams(true);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Line );
	params->setLineChartSubType( KDChartParams::LineNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartline->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(LineChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::barsChart()
{
#if 0
    if ( m_chartbars->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Bar );
	params->setBarChartSubType( KDChartParams::BarNormal );

	updateButton();
    params->setThreeDBars( params->threeDPies() );
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartbars->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(BarChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::areasChart()
{
#if 0
    if ( m_chartareas->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Area );
	params->setAreaChartSubType( KDChartParams::AreaNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartareas->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(AreaChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


#if 0  // Temporarily disabled because there is no type HiLo in ODF
void KChartView::hiLoChart()
{
#if 0
    if ( m_charthilo->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::HiLo );
	params->setHiLoChartSubType( KDChartParams::HiLoNormal );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_charthilo->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->chart());
//     command->setChartType(HiLoChartType);
    koDocument()->addCommand(command);
    update();
#endif
}
#endif

void KChartView::ringChart()
{
#if 0
    if ( m_chartring->isChecked() ) {
	forceAxisParams(false);
	KChartParams* params = ((KChartPart*)koDocument())->params();

	params->setChartType( KChartParams::Ring );

	updateButton();
	update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
	m_chartring->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(RingChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::radarChart()
{
#if 0
    if ( m_chartpolar->isChecked() ) {
	forceAxisParams(false);
        KDChartParams* params = ((KChartPart*)koDocument())->params();

        params->setChartType( KDChartParams::Polar );
        params->setPolarChartSubType( KDChartParams::PolarNormal );

        update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartpolar->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    command->setChartType(RadarChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::stockChart()
{
#if 0
    if ( m_chartbw->isChecked() ) {
	forceAxisParams(false);
        KDChartParams* params = ((KChartPart*)koDocument())->params();

        params->setChartType( KDChartParams::BoxWhisker );
        params->setBWChartSubType( KDChartParams::BWNormal );

        update();
	((KChartPart*)koDocument())->setModified(true);
    }
    else
        m_chartbw->setChecked( true ); // always one has to be checked !
#else
    ChartTypeCommand* command = new ChartTypeCommand(qobject_cast<KChartPart*>(koDocument())->shape());
    // FIXME: Use a subtype suitable for stock charts when those are created
    command->setChartType(StockChartType, NormalChartSubtype);
    koDocument()->addCommand(command);
    update();
#endif
}


void KChartView::mousePressEvent ( QMouseEvent *e )
{
    if (!koDocument()->isReadWrite() || !factory())
        return;
    if ( e->button() == Qt::RightButton )
        ((QMenu*)factory()->container("action_popup",this))->popup(QCursor::pos());
}


// FIXME: Rename into something suitable.
void KChartView::updateButton()
{
#if 0
    // Disable sub chart config item.
    KChartPart* part = qobject_cast<KChartPart*>(koDocument());

    bool state=(part->chartType() == BarChartType ||
                part->chartType() == AreaChartType ||
                part->chartType() == LineChartType ||
                part->chartType() == HiLoChartType ||
                part->chartType() == PolarChartType);
    m_subTypeChartConfig->setEnabled(state);
#endif
}


void KChartView::slotConfigPageLayout()
{
    KChartPart    *part = qobject_cast<KChartPart*>( koDocument() );
    KCPageLayout  *dialog = new KCPageLayout(part, this);

    connect( dialog, SIGNAL( dataChanged() ),
             this,   SLOT( slotRepaint() ) );

    dialog->exec();
    delete dialog;
}


void KChartView::setupPrinter( KPrinter &printer )
{
  if ( !printer.previewOnly() )
    printer.addDialogPage( new KCPrinterDialog( 0, "KChart page" ) );
}


void KChartView::print(KPrinter &printer)
{
    Q_UNUSED( printer );
#if 0                           // Disable printing for now.
    printer.setFullPage( false );

    QPainter painter;
    painter.begin(&printer);

    int  height;
    int  width;
    if ( !printer.previewOnly() ) {
	int const scalex = printer.option("kde-kchart-printsizex").toInt();
	int const scaley = printer.option("kde-kchart-printsizey").toInt();

	width  = printer.width()  * scalex / 100;
	height = printer.height() * scaley / 100;
    }
    else {
	// Fill the whole page.
	width  = printer.width();
	height = printer.height();
    }

    QRect  rect(0, 0, width, height);
    KDChart::print(&painter,
		   ((KChartPart*)koDocument())->params(),
		   ((KChartPart*)koDocument())->data(),
		   0, 		// regions
		   &rect);
    painter.end();
#endif
}


// Import data from a Comma Separated Values file.
//

void KChartView::importData()
{
    // Get the name of the file to open.
    QString filename = KFileDialog::getOpenFileName(KUrl(QString()),// startDir
						    QString(),// filter
						    0,
						    i18n("Import Data"));
    kDebug(35001) <<"Filename = <" << filename <<">";
    if (filename.isEmpty())
      return;

    // Check to see if we can read the file.
    QFile  inFile(filename);
    if (!inFile.open(QIODevice::ReadOnly)) {
	KMessageBox::sorry( 0, i18n("The file %1 could not be read.",
                                    filename) );
	inFile.close();
	return;
    }

    // Let the CSV dialog structure the data in the file.
    QByteArray  inData( inFile.readAll() );
    inFile.close();
    KoCsvImportDialog *dialog = new KoCsvImportDialog(0L);
    dialog->setData(inData);

    if ( !dialog->exec() ) {
	// kDebug(35001) <<"Cancel was pressed";
	return;
    }

    //kDebug(35001) <<"OK was pressed";

#if 0
    uint  rows = dialog->rows();
    uint  cols = dialog->cols();

    //kDebug(35001) <<"Rows:" << rows <<"  Cols:" << cols;

    bool  hasRowHeaders = ( rows > 1 && dialog->firstRowContainHeaders() );
    bool  hasColHeaders = ( cols > 1 && dialog->firstColContainHeaders() );

    KDChartTableData  data( rows, cols );
    data.setUsedRows( rows );
    data.setUsedCols( cols );
    for (uint row = 0; row < rows; row++) {
	for (uint col = 0; col < cols; col++) {
	    bool     ok;
	    QString  tmp;
	    double   val;

	    // Get the text and convert to double unless in the headers.
	    tmp = dialog->text( row, col );
	    if ( ( row == 0 && hasRowHeaders )
		 || ( col == 0 && hasColHeaders ) ) {
		kDebug(35001) <<"Setting header (" << row <<"," << col
			       << ") to value " << tmp << endl;
		data.setCell( row, col, tmp );
	    }
	    else {
		val = tmp.toDouble(&ok);
		if (!ok)
		    val = 0.0;

		kDebug(35001) <<"Setting (" << row <<"," << col
			       << ") to value " << val << endl;

		// and do the actual setting.
		data.setCell( row, col, val );
	    }
	}
    }

    ((KChartPart*)koDocument())->doSetData( data,
					    hasRowHeaders, hasColHeaders );
#else
    QStandardItemModel  data();
#endif
}


void KChartView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    QPixmap pix = koDocument()->generatePreview(QSize(width, height));

    KTemporaryFile tempFile;
    tempFile.setSuffix(".chrt");
    tempFile.open();

    koDocument()->saveNativeFormat( tempFile.fileName() );

    KoTemplateCreateDia::createTemplate( "kchart_template", KChartFactory::global(),
                                         tempFile.fileName(), pix, this );

    KChartFactory::global().dirs()->addResourceType( "kchart_template", "data", "kchart/templates/" );
}

void KChartView::selectionChanged()
{
    KoSelection *selection = m_canvas->shapeManager()->selection();
    emit selectionChange();
}

void KChartView::documentViewRectChanged( const QRectF &viewRect )
{
    QSizeF size = viewRect.size();
    m_zoomController->setDocumentSize( size );
    m_zoomController->setPageSize( size );
    m_canvas->update();
    m_canvasController->ensureVisible( m_canvas->shapeManager()->selection()->boundingRect() );
}

void KChartView::zoomChanged( KoZoomMode::Mode mode, double zoom )
{
    QRectF documentViewRect = m_canvas->documentViewRect();
    m_zoomController->setDocumentSize( documentViewRect.size() );
    m_canvasController->setPreferredCenter( m_zoomHandler->documentToView( documentViewRect ).center().toPoint() );
}

}  //KChart namespace

#include "KChartView.moc"