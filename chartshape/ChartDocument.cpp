/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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
 * Boston, MA 02110-1301, USA.
*/

// Own
#include "ChartDocument.h"

// Qt
#include <QWidget>
#include <QIODevice>
#include <QDebug>
#include <QPainter>

// KOffice
#include <KoDocument.h>
#include <KXmlWriter.h>
#include <KOdfStoreReader.h>
#include <KOdfWriteStore.h>
#include <KOdfLoadingContext.h>
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>
#include <KOdfXmlNS.h>
#include <KOdfStylesReader.h>
#include <KOdfGenericStyles.h>
#include <KOdfEmbeddedDocumentSaver.h>
#include <KoView.h>
#include <KComponentData>
#include <KDebug>

#include "ChartShape.h"

class ChartDocument::Private
{
public:
    Private();
    ~Private();

    ChartShape *parent;
};

ChartDocument::Private::Private()
{
}

ChartDocument::Private::~Private()
{
}

ChartDocument::ChartDocument(ChartShape *parent)
    : KoDocument(0, 0)
    , d (new Private)
{
    d->parent = parent;
    // Needed by KoDocument::nativeOasisMimeType().
    // KOdfEmbeddedDocumentSaver uses that method to
    // get the mimetype of the embedded document.
    setComponentData(KComponentData("kchart"));
}

ChartDocument::~ChartDocument()
{
    delete d;
}


bool ChartDocument::loadOdf(KOdfStoreReader &odfStore)
{
    KXmlDocument doc = odfStore.contentDoc();
    KXmlNode bodyNode = doc.documentElement().namedItemNS(KOdfXmlNS::office, "body");
    if (bodyNode.isNull()) {
        kError(35001) << "No <office:body> element found.";
        return false;
    }
    KXmlNode chartElementParentNode = bodyNode.namedItemNS(KOdfXmlNS::office, "chart");
    if (chartElementParentNode.isNull()) {
        kError(35001) << "No <office:chart> element found.";
        return false;
    }
    KXmlElement chartElement = chartElementParentNode.namedItemNS(KOdfXmlNS::chart, "chart").toElement();
    if (chartElement.isNull()) {
        kError(35001) << "No <chart:chart> element found.";
        return false;
    }
    KOdfLoadingContext odfLoadingContext(odfStore.styles(), odfStore.store());
    KShapeLoadingContext context(odfLoadingContext, d->parent->resourceManager());

    return d->parent->loadOdfChartElement(chartElement, context);
}

bool ChartDocument::loadXML(const KXmlDocument &doc, KOdfStore *)
{
    Q_UNUSED(doc);

    // We don't support the old XML format any more.
    return false;
}

bool ChartDocument::saveOdf(SavingContext &context)
{
    KOdfWriteStore &odfStore = context.odfStore;
    KOdfStore *store = odfStore.store();
    KXmlWriter *manifestWriter = odfStore.manifestWriter();
    KXmlWriter *contentWriter  = odfStore.contentWriter();
    if (!contentWriter)
        return false;

    KOdfGenericStyles mainStyles;
    KXmlWriter *bodyWriter = odfStore.bodyWriter();
    if (!bodyWriter)
        return false;

    KOdfEmbeddedDocumentSaver& embeddedSaver = context.embeddedSaver;

    KShapeSavingContext savingContext(*bodyWriter, mainStyles, embeddedSaver);

    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:chart");

    d->parent->saveOdf(savingContext);

    bodyWriter->endElement(); // office:chart
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);
    odfStore.closeContentWriter();

    // Add manifest line for content.xml and styles.xml
    manifestWriter->addManifestEntry(url().path() + "/content.xml", "text/xml");
    manifestWriter->addManifestEntry(url().path() + "/styles.xml", "text/xml");

    // save the styles.xml
    if (!mainStyles.saveOdfStylesDotXml(store, manifestWriter))
        return false;

    if (!savingContext.saveDataCenter(store, manifestWriter)) {
        return false;
    }

    return true;
}

KoView *ChartDocument::createViewInstance(QWidget *parent)
{
    Q_UNUSED(parent);

    return 0;
}

void ChartDocument::paintContent(QPainter &painter, const QRect &rect)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
}

