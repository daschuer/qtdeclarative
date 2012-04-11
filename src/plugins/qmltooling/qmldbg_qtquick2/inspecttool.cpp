/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "inspecttool.h"

#include "highlight.h"
#include "qquickviewinspector.h"

#include <QtCore/QLineF>

#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QTouchEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QStyleHints>

#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>

namespace QmlJSDebugger {
namespace QtQuick2 {

InspectTool::InspectTool(QQuickViewInspector *inspector, QQuickView *view) :
    AbstractTool(inspector),
    m_originalSmooth(view->rootItem()->smooth()),
    m_dragStarted(false),
    m_pinchStarted(false),
    m_didPressAndHold(false),
    m_tapEvent(false),
    m_rootItem(view->rootItem()),
    m_originalPosition(view->rootItem()->pos()),
    m_currentScale(1.0f),
    m_smoothScaleFactor(Constants::ZoomSnapDelta),
    m_minScale(0.125f),
    m_maxScale(48.0f),
    m_originalScale(view->rootItem()->scale()),
    m_touchTimestamp(0),
    m_hoverHighlight(new HoverHighlight(inspector->overlay())),
    m_lastItem(0),
    m_lastClickedItem(0)
{
    //Press and Hold Timer
    m_pressAndHoldTimer.setSingleShot(true);
    m_pressAndHoldTimer.setInterval(Constants::PressAndHoldTimeout);
    connect(&m_pressAndHoldTimer, SIGNAL(timeout()), SLOT(zoomTo100()));
    enable(true);
    inspector->addToUnselectableItems(m_hoverHighlight);
}

InspectTool::~InspectTool()
{
    enable(false);
}

void InspectTool::enable(bool enable)
{
    if (!enable) {
        inspector()->setSelectedItems(QList<QQuickItem*>());
        // restoring the original states.
        if (m_rootItem) {
            m_rootItem->setScale(m_originalScale);
            m_rootItem->setPos(m_originalPosition);
            m_rootItem->setSmooth(m_originalSmooth);
        }
    } else {
        if (m_rootItem) {
            m_originalSmooth = m_rootItem->smooth();
            m_originalScale = m_rootItem->scale();
            m_originalPosition = m_rootItem->pos();
            m_rootItem->setSmooth(true);
        }
    }
}

void InspectTool::leaveEvent(QEvent *)
{
    m_hoverHighlight->setVisible(false);
}

void InspectTool::mousePressEvent(QMouseEvent *event)
{
    m_mousePosition = event->posF();
    if (event->button() == Qt::LeftButton) {
        m_pressAndHoldTimer.start();
        initializeDrag(event->posF());
    }
}

void InspectTool::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePosition = event->posF();
    m_pressAndHoldTimer.stop();
    if (event->button() == Qt::LeftButton && !m_dragStarted) {
        selectItem();
        m_hoverHighlight->setVisible(false);
    }
}

void InspectTool::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_mousePosition = event->posF();
    m_pressAndHoldTimer.stop();
    if (event->button() == Qt::LeftButton) {
        selectNextItem();
        m_hoverHighlight->setVisible(false);
    }
}

void InspectTool::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePosition = event->posF();
    moveItem(event->buttons() & Qt::LeftButton);
}

void InspectTool::hoverMoveEvent(QMouseEvent *event)
{
    m_mousePosition = event->posF();
    m_pressAndHoldTimer.stop();
    QQuickItem *item = inspector()->topVisibleItemAt(event->pos());
    if (!item || item == m_lastClickedItem) {
        m_hoverHighlight->setVisible(false);
    } else {
        if (item->parentItem())
            m_hoverHighlight->setParentItem(item->parentItem());
        else
            m_hoverHighlight->setParentItem(inspector()->overlay());
        m_hoverHighlight->setItem(item);
        m_hoverHighlight->setVisible(true);
    }
}

void InspectTool::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() != Qt::Vertical)
        return;

    Qt::KeyboardModifier smoothZoomModifier = Qt::ControlModifier;
    if (event->modifiers() & smoothZoomModifier) {
        int numDegrees = event->delta() / 8;
        qreal newScale = m_currentScale + m_smoothScaleFactor * (numDegrees / 15.0f);
        scaleView(newScale / m_currentScale, m_mousePosition, m_mousePosition);
    } else if (!event->modifiers()) {
        if (event->delta() > 0) {
            zoomIn();
        } else if (event->delta() < 0) {
            zoomOut();
        }
    }
}

void InspectTool::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9: {
        qreal newScale = ((event->key() - Qt::Key_0) * 1.0f);
        scaleView(newScale / m_currentScale, m_mousePosition, m_mousePosition);
        break;
    }
    default:
        break;
    }
}

void InspectTool::touchEvent(QTouchEvent *event)
{
    QList<QTouchEvent::TouchPoint> touchPoints = event->touchPoints();

    switch (event->type()) {
    case QEvent::TouchBegin:
        if (touchPoints.count() == 1 && (event->touchPointStates() & Qt::TouchPointPressed)) {
            if (!m_pressAndHoldTimer.isActive())
                m_pressAndHoldTimer.start();
            m_mousePosition = touchPoints.first().pos();
            initializeDrag(touchPoints.first().pos());
            m_tapEvent = true;
        } else {
            m_tapEvent = false;
        }
        break;
    case QEvent::TouchUpdate: {
        if (touchPoints.count() > 1)
            m_tapEvent = false;
        if ((touchPoints.count() == 1)
                && (event->touchPointStates() & Qt::TouchPointMoved)) {
            m_mousePosition = touchPoints.first().pos();
            moveItem(true);
        } else if ((touchPoints.count() == 2)
                   && (!(event->touchPointStates() & Qt::TouchPointReleased))) {
            // determine scale factor
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

            qreal touchScaleFactor =
                    QLineF(touchPoint0.pos(), touchPoint1.pos()).length()
                    / QLineF(touchPoint0.lastPos(), touchPoint1.lastPos()).length();

            QPointF oldcenter = (touchPoint0.lastPos() + touchPoint1.lastPos()) / 2;
            QPointF newcenter = (touchPoint0.pos() + touchPoint1.pos()) / 2;

            m_pinchStarted = true;
            scaleView(touchScaleFactor, newcenter, oldcenter);
        }
        break;
    }
    case QEvent::TouchEnd: {
        m_pressAndHoldTimer.stop();
        if (m_pinchStarted) {
            m_pinchStarted = false;
        }
        if (touchPoints.count() == 1 && !m_dragStarted &&
                !m_didPressAndHold && m_tapEvent) {
            m_tapEvent = false;
            bool doubleTap = event->timestamp() - m_touchTimestamp
                    < static_cast<ulong>(qApp->styleHints()->mouseDoubleClickInterval());
            if (doubleTap)
                selectNextItem();
            else
                selectItem();
            m_touchTimestamp = event->timestamp();
        }
        m_didPressAndHold = false;
        break;
    }
    default:
        break;
    }
}

void InspectTool::scaleView(const qreal &factor, const QPointF &newcenter, const QPointF &oldcenter)
{
    m_pressAndHoldTimer.stop();
    if (((m_currentScale * factor) > m_maxScale)
            || ((m_currentScale * factor) < m_minScale)) {
        return;
    }
    //New position = new center + scalefactor * (oldposition - oldcenter)
    m_adjustedOrigin = newcenter + (factor * (m_adjustedOrigin - oldcenter));
    m_currentScale *= factor;

    m_rootItem->setScale(m_currentScale);
    m_rootItem->setPos(m_adjustedOrigin);
}

void InspectTool::zoomIn()
{
    qreal newScale = nextZoomScale(ZoomIn);
    scaleView(newScale / m_currentScale, m_mousePosition, m_mousePosition);
}

void InspectTool::zoomOut()
{
    qreal newScale = nextZoomScale(ZoomOut);
    scaleView(newScale / m_currentScale, m_mousePosition, m_mousePosition);
}

void InspectTool::zoomTo100()
{
    m_didPressAndHold = true;
    m_currentScale = 1.0;
    m_adjustedOrigin = QPointF(0, 0);

    m_rootItem->setPos(m_adjustedOrigin);
    m_rootItem->setScale(m_currentScale);
}

qreal InspectTool::nextZoomScale(ZoomDirection direction)
{
    static QList<qreal> zoomScales =
            QList<qreal>()
            << 0.125f
            << 1.0f / 6.0f
            << 0.25f
            << 1.0f / 3.0f
            << 0.5f
            << 2.0f / 3.0f
            << 1.0f
            << 2.0f
            << 3.0f
            << 4.0f
            << 5.0f
            << 6.0f
            << 7.0f
            << 8.0f
            << 12.0f
            << 16.0f
            << 32.0f
            << 48.0f;

    if (direction == ZoomIn) {
        for (int i = 0; i < zoomScales.length(); ++i) {
            if (zoomScales[i] > m_currentScale)
                return zoomScales[i];
        }
        return zoomScales.last();
    } else {
        for (int i = zoomScales.length() - 1; i >= 0; --i) {
            if (zoomScales[i] < m_currentScale)
                return zoomScales[i];
        }
        return zoomScales.first();
    }

    return 1.0f;
}

void InspectTool::initializeDrag(const QPointF &pos)
{
    m_dragStartPosition = pos;
    m_dragStarted = false;
}

void InspectTool::dragItemToPosition()
{
    m_adjustedOrigin += m_mousePosition - m_dragStartPosition;
    m_dragStartPosition = m_mousePosition;
    m_rootItem->setPos(m_adjustedOrigin);
}

void InspectTool::moveItem(bool valid)
{
    if (m_pinchStarted)
        return;

    if (!m_dragStarted
            && valid
            && ((m_dragStartPosition - m_mousePosition).manhattanLength()
                > qApp->styleHints()->startDragDistance())) {
        m_pressAndHoldTimer.stop();
        m_dragStarted = true;
    }
    if (m_dragStarted)
        dragItemToPosition();
}

void InspectTool::selectNextItem()
{
    if (m_lastClickedItem != inspector()->topVisibleItemAt(m_mousePosition))
        return;
    QList<QQuickItem*> items = inspector()->itemsAt(m_mousePosition);
    for (int i = 0; i < items.count(); i++) {
        if (m_lastItem == items[i]) {
            if (i + 1 < items.count())
                m_lastItem = items[i+1];
            else
                m_lastItem = items[0];
            inspector()->setSelectedItems(QList<QQuickItem*>() << m_lastItem);
            break;
        }
    }
}

void InspectTool::selectItem()
{
    if (!inspector()->topVisibleItemAt(m_mousePosition))
        return;
    if (m_lastClickedItem == inspector()->topVisibleItemAt(m_mousePosition))
        return;
    m_lastClickedItem = inspector()->topVisibleItemAt(m_mousePosition);
    m_lastItem = m_lastClickedItem;
    inspector()->setSelectedItems(QList<QQuickItem*>() << m_lastClickedItem);
}

QQuickViewInspector *InspectTool::inspector() const
{
    return static_cast<QQuickViewInspector*>(AbstractTool::inspector());
}

} // namespace QtQuick2
} // namespace QmlJSDebugger
