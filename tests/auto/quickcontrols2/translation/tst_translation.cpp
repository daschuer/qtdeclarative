/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#include <QtTest/qsignalspy.h>
#include <QtTest/qtest.h>
#include <QtCore/qtranslator.h>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformtheme.h>
#include <QtQuick/qquickview.h>
#include <QtQuickTestUtils/private/qmlutils_p.h>
#include <QtQuickTestUtils/private/visualtestutils_p.h>
#include <QtQuickControlsTestUtils/private/controlstestutils_p.h>
#include <QtQuickTemplates2/private/qquickabstractbutton_p.h>
#include <QtQuickTemplates2/private/qquickcombobox_p.h>
#include <QtQuickTemplates2/private/qquickdialog_p.h>
#include <QtQuickTemplates2/private/qquickdialogbuttonbox_p.h>
#include <QtQuickTemplates2/private/qquicktextfield_p.h>
#include <QtQuickControls2/qquickstyle.h>

using namespace QQuickControlsTestUtils;
using namespace QQuickVisualTestUtils;

class tst_translation : public QQmlDataTest
{
    Q_OBJECT

public:
    tst_translation();

private slots:
    void dialogButtonBox();
    void dialogButtonBoxWithCustomButtons();
    void comboBox();
    void stackView();
};

tst_translation::tst_translation()
    : QQmlDataTest(QT_QMLTEST_DATADIR)
{
    QQuickStyle::setStyle("Basic");
}

void tst_translation::dialogButtonBox()
{
    QQuickView view(testFileUrl("dialogButtonBox.qml"));
    if (view.status() != QQuickView::Ready)
        QFAIL("Failed to load QML file");
    view.show();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickDialog *dialog = view.rootObject()->property("dialog").value<QQuickDialog*>();
    QVERIFY(dialog);

    QQuickDialogButtonBox *dialogButtonBox = qobject_cast<QQuickDialogButtonBox*>(dialog->footer());
    QVERIFY(dialogButtonBox);

    QQuickAbstractButton *saveButton = dialogButtonBox->standardButton(QPlatformDialogHelper::Save);
    QVERIFY(saveButton);
    QString defaultSaveText = QGuiApplicationPrivate::platformTheme()->standardButtonText(QPlatformDialogHelper::Save);
    defaultSaveText = QPlatformTheme::removeMnemonics(defaultSaveText);
    QCOMPARE(saveButton->text(), defaultSaveText);

    QQuickAbstractButton *discardButton = dialogButtonBox->standardButton(QPlatformDialogHelper::Discard);
    QVERIFY(discardButton);
    QString defaultDiscardText = QGuiApplicationPrivate::platformTheme()->standardButtonText(QPlatformDialogHelper::Discard);
    defaultDiscardText = QPlatformTheme::removeMnemonics(defaultDiscardText);
    QCOMPARE(discardButton->text(), defaultDiscardText);

    QTranslator translator;
    QVERIFY(translator.load("qtbase_fr.qm", ":/"));
    QVERIFY(qApp->installTranslator(&translator));
    qApp->sendPostedEvents();
    view.engine()->retranslate();

    QString translatedSaveText = QGuiApplicationPrivate::platformTheme()->standardButtonText(QPlatformDialogHelper::Save);
    translatedSaveText = QPlatformTheme::removeMnemonics(translatedSaveText);
    QCOMPARE(saveButton->text(), translatedSaveText);

    QString translatedDiscardText = QGuiApplicationPrivate::platformTheme()->standardButtonText(QPlatformDialogHelper::Discard);
    translatedDiscardText = QPlatformTheme::removeMnemonics(translatedDiscardText);
    QCOMPARE(discardButton->text(), translatedDiscardText);
}

// Test that custom buttons with explicitly specified text
// do not have that text overwritten on language changes.
void tst_translation::dialogButtonBoxWithCustomButtons()
{
    // This is just a way of simulating the translator going out of scope
    // after the QML has been loaded.
    QScopedPointer<QTranslator> translator(new QTranslator);
    // Doesn't matter which language it is, as we won't be using it anyway.
    QVERIFY(translator->load("qtbase_fr.qm", ":/"));
    QVERIFY(qApp->installTranslator(translator.data()));

    QQuickView view(testFileUrl("dialogButtonBoxWithCustomButtons.qml"));
    if (view.status() != QQuickView::Ready)
        QFAIL("Failed to load QML file");
    view.show();
    QVERIFY(QTest::qWaitForWindowActive(&view));

    QQuickDialog *dialog = view.rootObject()->property("dialog").value<QQuickDialog*>();
    QVERIFY(dialog);

    QQuickDialogButtonBox *dialogButtonBox = qobject_cast<QQuickDialogButtonBox*>(dialog->footer());
    QVERIFY(dialogButtonBox);

    auto okButton = dialogButtonBox->findChild<QQuickAbstractButton*>("okButton");
    QVERIFY(okButton);
    QCOMPARE(okButton->text(), QLatin1String("OK"));

    QQuickAbstractButton *cancelButton = dialogButtonBox->findChild<QQuickAbstractButton*>("cancelButton");
    QVERIFY(cancelButton);
    QCOMPARE(cancelButton->text(), QLatin1String("Cancel"));

    // Delete the translator and hence cause a LanguageChange event,
    // but _without_ calling QQmlEngine::retranslate(), which would
    // restore the original bindings and hence not reproduce the issue.
    translator.reset();
    QCOMPARE(okButton->text(), QLatin1String("OK"));
    QCOMPARE(cancelButton->text(), QLatin1String("Cancel"));
}

void tst_translation::comboBox()
{
    QQuickView view(testFileUrl("comboBox.qml"));

    QQuickComboBox *comboBox = qobject_cast<QQuickComboBox*>(view.rootObject());
    QVERIFY(comboBox);
    QCOMPARE(comboBox->displayText(), QLatin1String("Hello"));

    QQuickTextField *contentItem = qobject_cast<QQuickTextField*>(comboBox->contentItem());
    QVERIFY(contentItem);
    QCOMPARE(contentItem->text(), QLatin1String("Hello"));

    QTranslator translator;
    QVERIFY(translator.load("qml_jp.qm", ":/"));
    QVERIFY(qApp->installTranslator(&translator));
    view.engine()->retranslate();
    QTRY_COMPARE(comboBox->displayText(), QString::fromUtf8("こんにちは"));
    QCOMPARE(contentItem->text(), QString::fromUtf8("こんにちは"));
}

void tst_translation::stackView()
{
    QQuickControlsApplicationHelper helper(this, "stackView.qml");
    QVERIFY2(helper.ready, helper.failureMessage());

    QQuickWindow *window = helper.window;
    QVERIFY(window->setProperty("engine", QVariant::fromValue(&helper.engine)));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));

    QSignalSpy calledTranslateSpy(window, SIGNAL(calledTranslate()));
    QVERIFY(calledTranslateSpy.isValid());

    QQuickAbstractButton *button = window->findChild<QQuickAbstractButton*>("button");
    QVERIFY(button);
    // Shouldn't crash when calling retranslate.
    QVERIFY(clickButton(button));
    QTRY_COMPARE(calledTranslateSpy.count(), 1);
}

QTEST_MAIN(tst_translation)

#include "tst_translation.moc"
