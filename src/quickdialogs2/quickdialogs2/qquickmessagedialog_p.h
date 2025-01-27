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

#ifndef QQUICKMESSAGEDIALOG_P_H
#define QQUICKMESSAGEDIALOG_P_H

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

#include "qquickabstractdialog_p.h"

QT_BEGIN_NAMESPACE

class Q_QUICKDIALOGS2_PRIVATE_EXPORT QQuickMessageDialog : public QQuickAbstractDialog
{
    Q_OBJECT

private:
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QString informativeText READ informativeText WRITE setInformativeText NOTIFY informativeTextChanged FINAL)
    Q_PROPERTY(QString detailedText READ detailedText WRITE setDetailedText NOTIFY detailedTextChanged FINAL)
    Q_PROPERTY(QPlatformDialogHelper::StandardButtons buttons READ buttons WRITE setButtons NOTIFY buttonsChanged FINAL)
    Q_FLAG(QPlatformDialogHelper::StandardButton)
    QML_NAMED_ELEMENT(MessageDialog)
    QML_ADDED_IN_VERSION(6, 3)

public:
    explicit QQuickMessageDialog(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    QString informativeText() const;
    void setInformativeText(const QString &text);

    QString detailedText() const;
    void setDetailedText(const QString &text);

    QPlatformDialogHelper::StandardButtons buttons() const;
    void setButtons(QPlatformDialogHelper::StandardButtons buttons);

Q_SIGNALS:
    void textChanged();
    void informativeTextChanged();
    void detailedTextChanged();
    void buttonsChanged();

    void buttonClicked(QPlatformDialogHelper::StandardButton button,
                       QPlatformDialogHelper::ButtonRole role);

private Q_SLOTS:
    void handleClick(QPlatformDialogHelper::StandardButton button,
                     QPlatformDialogHelper::ButtonRole role);

protected:
    void onCreate(QPlatformDialogHelper *dialog) override;
    void onShow(QPlatformDialogHelper *dialog) override;

private:
    QSharedPointer<QMessageDialogOptions> m_options;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuickMessageDialog)

#endif // QQUICKMESSAGEDIALOG_P_H
