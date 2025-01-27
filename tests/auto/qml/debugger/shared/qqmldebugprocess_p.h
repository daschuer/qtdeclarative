/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
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

#ifndef QQMLDEBUGPROCESS_P_H
#define QQMLDEBUGPROCESS_P_H

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

#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>
#include <QtCore/qeventloop.h>
#include <QtCore/qmutex.h>

class QQmlDebugProcess : public QObject
{
    Q_OBJECT
public:
    QQmlDebugProcess(const QString &executable, QObject *parent = nullptr);
    ~QQmlDebugProcess();

    QString stateString() const;

    void addEnvironment(const QString &environment);

    void start(const QStringList &arguments);
    bool waitForSessionStart();
    int debugPort() const;

    bool waitForFinished();
    QProcess::ProcessState state() const;
    QProcess::ExitStatus exitStatus() const;

    QString output() const;
    void stop();
    void setMaximumBindErrors(int numErrors);

signals:
    void readyReadStandardOutput();
    void finished();

private slots:
    void timeout();
    void processAppOutput();
    void processError(QProcess::ProcessError error);

private:
    enum SessionState {
        SessionUnknown,
        SessionStarted,
        SessionFailed
    };

    QString m_executable;
    QProcess m_process;
    QString m_outputBuffer;
    QString m_output;
    QTimer m_timer;
    QEventLoop m_eventLoop;
    QMutex m_mutex;
    SessionState m_state;
    QStringList m_environment;
    int m_port;
    int m_maximumBindErrors;
    int m_receivedBindErrors;
};

#endif // QQMLDEBUGPROCESS_P_H
