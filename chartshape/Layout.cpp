/* This file is part of the KDE project

   Copyright 2010 Johannes Simon <johannes.simon@gmail.com>

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

// Qt
#include <QSizeF>
#include <QPointF>

// KChart
#include "Layout.h"

// KOffice
#include <KShapeContainer.h>


// Static helper methods (defined at end of file)
static QPointF itemPosition(KShape *shape);
static QSizeF itemSize(KShape *shape);
static void setItemPosition(KShape *shape, const QPointF& pos);

class Layout::LayoutData
{
public:
    Position pos;
    int weight;
    bool clipped;
    bool inheritsTransform;

    LayoutData(Position _pos, int _weight)
        : pos(_pos),
          weight(_weight),
          clipped(true),
          inheritsTransform(true) {}
};

Layout::Layout()
    : m_doingLayout(false)
    , m_relayoutScheduled(false)
{
}

Layout::~Layout()
{
    foreach(LayoutData *data, m_layoutItems.values())
        delete data;
}

void Layout::add(KShape *shape)
{
    Q_ASSERT(!m_layoutItems.contains(shape));
    m_layoutItems.insert(shape, new LayoutData(FloatingPosition, 0));
    scheduleRelayout();
}

void Layout::add(KShape *shape, Position pos, int weight)
{
    Q_ASSERT(!m_layoutItems.contains(shape));
    m_layoutItems.insert(shape, new LayoutData(pos, weight));
    scheduleRelayout();
}

void Layout::remove(KShape *shape)
{
    if (m_layoutItems.contains(shape)) {
        // delete LayoutData
        delete m_layoutItems.value(shape);
        m_layoutItems.remove(shape);
        scheduleRelayout();
    }
}

void Layout::setClipped(const KShape *shape, bool clipping)
{
    Q_ASSERT(m_layoutItems.contains(const_cast<KShape*>(shape)));
    m_layoutItems.value(const_cast<KShape*>(shape))->clipped = clipping;
}

bool Layout::isClipped(const KShape *shape) const
{
    Q_ASSERT(m_layoutItems.contains(const_cast<KShape*>(shape)));
    return m_layoutItems.value(const_cast<KShape*>(shape))->clipped;
}

void Layout::setInheritsTransform(const KShape *shape, bool inherit)
{
    m_layoutItems.value(const_cast<KShape*>(shape))->inheritsTransform = inherit;
}

bool Layout::inheritsTransform(const KShape *shape) const
{
    return m_layoutItems.value(const_cast<KShape*>(shape))->inheritsTransform;
}

int Layout::count() const
{
    return m_layoutItems.size();
}

QList<KShape*> Layout::shapes() const
{
    return m_layoutItems.keys();
}

void Layout::containerChanged(KShapeContainer *container, KShape::ChangeType type)
{
    switch(type) {
    case KShape::SizeChanged:
        m_containerSize = container->size();
        scheduleRelayout();
        break;
    default:
        break;
    }
}

bool Layout::isChildLocked(const KShape *shape) const
{
    return shape->isGeometryProtected();
}

void Layout::setPosition(const KShape *shape, Position pos, int weight)
{
    Q_ASSERT(m_layoutItems.contains(const_cast<KShape*>(shape)));
    LayoutData *data = m_layoutItems.value(const_cast<KShape*>(shape));
    data->pos = pos;
    data->weight = weight;
    scheduleRelayout();
}

void Layout::childChanged(KShape *shape, KShape::ChangeType type)
{
    Q_UNUSED(shape);

    // Do not relayout again if we're currently in the process of a relayout.
    // Repositioning a layout item or resizing it will result in a cull of this method.
    if (m_doingLayout)
        return;

    // This can be fine-tuned, but right now, simply everything will be re-layouted.
    switch (type) {
    case KShape::PositionChanged:
    case KShape::SizeChanged:
        scheduleRelayout();
    // FIXME: There's some cases that would require relayouting but that don't make sense
    // for chart items, e.g. ShearChanged. How should these changes be avoided or handled?
    default:
        break;
    }
}

void Layout::scheduleRelayout()
{
    m_relayoutScheduled = true;
}

void Layout::layout()
{
    Q_ASSERT(!m_doingLayout);

    if (!m_relayoutScheduled)
        return;

    m_doingLayout = true;

    QMap<int, KShape*> top, bottom, start, end;
    KShape *topStart    = 0,
            *bottomStart = 0,
            *topEnd      = 0,
            *bottomEnd   = 0,
            *p      = 0;

    QMapIterator<KShape*, LayoutData*> it(m_layoutItems);
    while (it.hasNext()) {
        it.next();
        KShape *shape = it.key();
        if (!shape->isVisible())
            continue;
        LayoutData *data = it.value();
        switch (data->pos) {
        case TopPosition:
            top.insert(data->weight, shape);
            break;
        case BottomPosition:
            bottom.insert(data->weight, shape);
            break;
        case StartPosition:
            start.insert(data->weight, shape);
            break;
        case EndPosition:
            end.insert(data->weight, shape);
            break;
        case TopStartPosition:
            topStart = shape;
            break;
        case BottomStartPosition:
            bottomStart = shape;
            break;
        case TopEndPosition:
            topEnd = shape;
            break;
        case BottomEndPosition:
            bottomEnd = shape;
            break;
        case CenterPosition:
            p = shape;
            break;
        case FloatingPosition:
            // Nothing to do
            break;
        }
    }

    qreal topY = layoutTop(top);
    qreal bottomY = layoutBottom(bottom);
    qreal startX = layoutStart(start);
    qreal endX = layoutEnd(end);
    if (p) {
        setItemPosition(p, QPointF(startX, topY));
        p->setSize(QSizeF(endX - startX, bottomY - topY));
    }

    layoutTopStart(topStart);
    layoutBottomStart(bottomStart);
    layoutTopEnd(topEnd);
    layoutBottomEnd(bottomEnd);

    m_doingLayout = false;
    m_relayoutScheduled = false;
}



/// Private Methods



qreal Layout::layoutTop(const QMap<int, KShape*>& shapes)
{
    qreal top = 0.0;
    qreal pX = m_containerSize.width() / 2.0;
    foreach(KShape *shape, shapes) {
        QSizeF size = itemSize(shape);
        setItemPosition(shape, QPointF(pX - size.width() / 2.0, top));
        top += size.height();
    }
    return top;
}

qreal Layout::layoutBottom(const QMap<int, KShape*>& shapes)
{
    qreal bottom = m_containerSize.height();
    qreal pX = m_containerSize.width() / 2.0;
    foreach(KShape *shape, shapes) {
        QSizeF size = itemSize(shape);
        bottom -= size.height();
        setItemPosition(shape, QPointF(pX - size.width() / 2.0, bottom));
    }
    return bottom;
}

qreal Layout::layoutStart(const QMap<int, KShape*>& shapes)
{
    qreal start = 0.0;
    qreal pY = m_containerSize.height() / 2.0;
    foreach(KShape *shape, shapes) {
        QSizeF size = itemSize(shape);
        setItemPosition(shape, QPointF(start, pY - size.height() / 2.0));
        start += size.width();
    }
    return start;
}

qreal Layout::layoutEnd(const QMap<int, KShape*>& shapes)
{
    qreal end = m_containerSize.width();
    qreal pY = m_containerSize.height() / 2.0;
    foreach(KShape *shape, shapes) {
        QSizeF size = itemSize(shape);
        end -= size.width();
        setItemPosition(shape, QPointF(end, pY - size.height() / 2.0));
    }
    return end;
}

void Layout::layoutTopStart(KShape *shape)
{
    if (!shape)
        return;
    setItemPosition(shape, QPointF(0, 0));
}

void Layout::layoutBottomStart(KShape *shape)
{
    if (!shape)
        return;
    setItemPosition(shape, QPointF(0, m_containerSize.height() - itemSize(shape).height()));
}

void Layout::layoutTopEnd(KShape *shape)
{
    if (!shape)
        return;
    setItemPosition(shape, QPointF(m_containerSize.width() - itemSize(shape).width(), 0));
}

void Layout::layoutBottomEnd(KShape *shape)
{
    if (!shape)
        return;
    const QSizeF size = itemSize(shape);
    setItemPosition(shape, QPointF(m_containerSize.width()  - size.width(),
                                     m_containerSize.height() - size.height()));
}


/// Static Helper Methods

static QPointF itemPosition(KShape *shape)
{
    const QRectF boundingRect = QRectF(QPointF(0, 0), shape->size());
    return shape->transformation().mapRect(boundingRect).topLeft();
}

static QSizeF itemSize(KShape *shape)
{
    const QRectF boundingRect = QRectF(QPointF(0, 0), shape->size());
    return shape->transformation().mapRect(boundingRect).size();
}

static void setItemPosition(KShape *shape, const QPointF& pos)
{
    const QPointF offset =  shape->position() - itemPosition(shape);
    shape->setPosition(pos + offset);
}
