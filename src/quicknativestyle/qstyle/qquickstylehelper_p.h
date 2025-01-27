/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qpaintdevice.h>
#include <QtGui/qpolygon.h>
#include <QtCore/qstringbuilder.h>
#if QT_CONFIG(accessibility)
#include <QtGui/qaccessible.h>
#endif

#ifndef QSTYLEHELPER_P_H
#define QSTYLEHELPER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QObject;
class QPainter;
class QPixmap;

namespace QQC2 {

class QStyleOptionSlider;
class QStyleOption;

namespace QStyleHelper
{
    QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size);

    qreal dpi(const QStyleOption *option);
    qreal dpiScaled(qreal value, qreal dpi);
    qreal dpiScaled(qreal value, const QPaintDevice *device);
    qreal dpiScaled(qreal value, const QStyleOption *option);

    qreal angle(const QPointF &p1, const QPointF &p2);
    QPolygonF calcLines(const QStyleOptionSlider *dial);
    int calcBigLineSize(int radius);
    void drawDial(const QStyleOptionSlider *dial, QPainter *painter);

    void drawBorderPixmap(const QPixmap &pixmap, QPainter *painter, const QRect &rect,
                     int left = 0, int top = 0, int right = 0,
                     int bottom = 0);

#if QT_CONFIG(accessibility)
    bool isInstanceOf(QObject *obj, QAccessible::Role role);
    bool hasAncestor(QObject *obj, QAccessible::Role role);
#endif
    QColor backgroundColor(const QPalette &pal);

    enum WidgetSizePolicy { SizeLarge = 0, SizeSmall = 1, SizeMini = 2, SizeDefault = -1 };
    WidgetSizePolicy widgetSizePolicy(const QStyleOption *opt);
}

} // namespace QQC2

QT_END_NAMESPACE

#endif // QSTYLEHELPER_P_H
