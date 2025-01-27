/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "prototype/visitor.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>

namespace Qmltc {
Visitor::Visitor(QQmlJSImporter *importer, QQmlJSLogger *logger,
                 const QString &implicitImportDirectory, const QStringList &qmltypesFiles)
    : QQmlJSImportVisitor(importer, logger, implicitImportDirectory, qmltypesFiles)
{
}

bool Visitor::visit(QQmlJS::AST::UiInlineComponent *component)
{
    if (!QQmlJSImportVisitor::visit(component))
        return false;
    m_logger->logCritical(u"Inline components are not supported"_qs, Log_Compiler,
                          component->firstSourceLocation());
    // despite the failure, return true here so that we do not assert in
    // QQmlJSImportVisitor::endVisit(UiInlineComponent)
    return true;
}
}
