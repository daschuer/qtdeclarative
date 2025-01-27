/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Dialogs module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickplatformfolderdialog_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtGui/qwindow.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlinfo.h>
#include <QtQuick/qquickwindow.h>
#include <QtQuickTemplates2/private/qquickdialog_p.h>
#include <QtQuickTemplates2/private/qquickpopup_p_p.h>
#include <QtQuickTemplates2/private/qquickpopupanchors_p.h>

#include "qquickfolderdialogimpl_p.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcQuickPlatformFolderDialog, "qt.quick.dialogs.quickplatformfolderdialog")

/*!
    \class QQuickPlatformFolderDialog
    \internal

    An interface that QQuickFolderDialog can use to access the non-native Qt Quick FolderDialog.

    Both this and the native implementations are created in QQuickAbstractDialog::create().
*/
QQuickPlatformFolderDialog::QQuickPlatformFolderDialog(QObject *parent)
{
    qCDebug(lcQuickPlatformFolderDialog) << "creating non-native Qt Quick FolderDialog with parent" << parent;

    // Set a parent so that we get deleted if we can't be shown for whatever reason.
    // Our eventual parent should be the window, though.
    setParent(parent);

    auto qmlContext = ::qmlContext(parent);
    if (!qmlContext) {
        qmlWarning(parent) << "No QQmlContext for QQuickPlatformFolderDialog; can't create non-native FolderDialog implementation";
        return;
    }

    const auto dialogQmlUrl = QUrl(QStringLiteral("qrc:/qt-project.org/imports/QtQuick/Dialogs/quickimpl/qml/FolderDialog.qml"));
    QQmlComponent folderDialogComponent(qmlContext->engine(), dialogQmlUrl, parent);
    if (!folderDialogComponent.isReady()) {
        qmlWarning(parent) << "Failed to load non-native FolderDialog implementation:\n" << folderDialogComponent.errorString();
        return;
    }
    m_dialog = qobject_cast<QQuickFolderDialogImpl*>(folderDialogComponent.create());
    if (!m_dialog) {
        qmlWarning(parent) << "Failed to create an instance of the non-native FolderDialog:\n" << folderDialogComponent.errorString();
        return;
    }
    // Give it a parent until it's parented to the window in show().
    m_dialog->setParent(this);

    connect(m_dialog, &QQuickDialog::accepted, this, &QPlatformDialogHelper::accept);
    connect(m_dialog, &QQuickDialog::rejected, this, &QPlatformDialogHelper::reject);

    connect(m_dialog, &QQuickFolderDialogImpl::selectedFolderChanged, this, &QQuickPlatformFolderDialog::currentChanged);
    connect(m_dialog, &QQuickFolderDialogImpl::currentFolderChanged, this, &QQuickPlatformFolderDialog::directoryEntered);
    connect(m_dialog, &QQuickFolderDialogImpl::selectedFolderChanged, this, &QQuickPlatformFolderDialog::fileSelected);

    // We would do this in QQuickFolderDialogImpl, but we need to ensure that folderChanged()
    // is connected to directoryEntered() before setting it to ensure that the QQuickFolderDialog is notified.
    if (m_dialog->currentFolder().isEmpty())
        m_dialog->setCurrentFolder(QUrl::fromLocalFile(QDir().absolutePath()));
}

bool QQuickPlatformFolderDialog::isValid() const
{
    return m_dialog;
}

bool QQuickPlatformFolderDialog::defaultNameFilterDisables() const
{
    return false;
}

void QQuickPlatformFolderDialog::setDirectory(const QUrl &directory)
{
    if (!m_dialog)
        return;

    m_dialog->setCurrentFolder(directory);
}

QUrl QQuickPlatformFolderDialog::directory() const
{
    if (!m_dialog)
        return {};

    return m_dialog->currentFolder();
}

void QQuickPlatformFolderDialog::selectFile(const QUrl &file)
{
    if (!m_dialog)
        return;

    m_dialog->setSelectedFolder(file);
}

QList<QUrl> QQuickPlatformFolderDialog::selectedFiles() const
{
    // FolderDialog doesn't support multiple selected folders.
    return { m_dialog->selectedFolder() };
}

void QQuickPlatformFolderDialog::setFilter()
{
}

void QQuickPlatformFolderDialog::selectNameFilter(const QString &/*filter*/)
{
}

QString QQuickPlatformFolderDialog::selectedNameFilter() const
{
    return QStringLiteral("*.*");
}

void QQuickPlatformFolderDialog::exec()
{
    qCWarning(lcQuickPlatformFolderDialog) << "exec() is not supported for the Qt Quick FolderDialog fallback";
}

bool QQuickPlatformFolderDialog::show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent)
{
    qCDebug(lcQuickPlatformFolderDialog) << "show called with flags" << flags <<
        "modality" << modality << "parent" << parent;
    if (!m_dialog)
        return false;

    if (!parent)
        return false;

    auto quickWindow = qobject_cast<QQuickWindow*>(parent);
    if (!quickWindow) {
        qmlInfo(this->parent()) << "Parent window (" << parent << ") of non-native dialog is not a QQuickWindow";
        return false;
    }
    m_dialog->setParent(parent);
    m_dialog->resetParentItem();

    auto popupPrivate = QQuickPopupPrivate::get(m_dialog);
    popupPrivate->getAnchors()->setCenterIn(m_dialog->parentItem());

    QSharedPointer<QFileDialogOptions> options = QPlatformFileDialogHelper::options();
    m_dialog->setTitle(options->windowTitle());
    m_dialog->setOptions(options);
    m_dialog->setAcceptLabel(options->isLabelExplicitlySet(QFileDialogOptions::Accept)
        ? options->labelText(QFileDialogOptions::Accept) : QString());
    m_dialog->setRejectLabel(options->isLabelExplicitlySet(QFileDialogOptions::Reject)
        ? options->labelText(QFileDialogOptions::Reject) : QString());

    m_dialog->open();
    return true;
}

void QQuickPlatformFolderDialog::hide()
{
    if (!m_dialog)
        return;

    m_dialog->close();
}

QQuickFolderDialogImpl *QQuickPlatformFolderDialog::dialog() const
{
    return m_dialog;
}

QT_END_NAMESPACE
