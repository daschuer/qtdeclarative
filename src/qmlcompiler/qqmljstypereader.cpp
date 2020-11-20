/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#include "qqmljstypereader_p.h"
#include "qqmljsimportvisitor_p.h"

#include <QtQml/private/qqmljsast_p.h>
#include <QtQml/private/qqmljsengine_p.h>
#include <QtQml/private/qqmljslexer_p.h>
#include <QtQml/private/qqmljsparser_p.h>
#include <QtQml/private/qqmlimportresolver_p.h>

#include <QtCore/qfileinfo.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

static QQmlJSScope::Ptr parseModule(QQmlJS::AST::ESModule *module, const QString &name)
{
    using namespace QQmlJS::AST;
    QQmlJSScope::Ptr result = QQmlJSScope::create(QQmlJSScope::JSLexicalScope);
    result->setInternalName(name);
    for (auto *statement = module->body; statement; statement = statement->next) {
        if (auto *exp = cast<ExportDeclaration *>(statement->statement)) {
            // TODO: There are a number of other things we could find here
            if (auto *function = cast<FunctionDeclaration *>(exp->variableStatementOrDeclaration)) {
                QQmlJSMetaMethod method(function->name.toString());
                method.setMethodType(QQmlJSMetaMethod::Method);
                for (auto *argument = function->formals; argument; argument = argument->next)
                    method.addParameter(argument->element->bindingIdentifier.toString(), QString());
                result->addOwnMethod(method);
            }
        }
    }
    return result;
}

static QQmlJSScope::Ptr parseProgram(QQmlJS::AST::Program *program, const QString &name)
{
    using namespace QQmlJS::AST;
    QQmlJSScope::Ptr result = QQmlJSScope::create(QQmlJSScope::JSLexicalScope);
    result->setInternalName(name);
    for (auto *statement = program->statements; statement; statement = statement->next) {
        if (auto *function = cast<FunctionDeclaration *>(statement->statement)) {
            QQmlJSMetaMethod method(function->name.toString());
            method.setMethodType(QQmlJSMetaMethod::Method);
            for (auto *parameters = function->formals; parameters; parameters = parameters->next)
                method.addParameter(parameters->element->bindingIdentifier.toString(), QString());
            result->addOwnMethod(method);
        }
    }
    return result;
}

QQmlJSScope::Ptr QQmlJSTypeReader::operator()()
{
    using namespace QQmlJS::AST;
    const QFileInfo info { m_file };
    QString baseName = info.baseName();
    const QString scopeName = baseName.endsWith(QStringLiteral(".ui")) ? baseName.chopped(3)
                                                                       : baseName;

    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    const QString lowerSuffix = info.suffix().toLower();
    const bool isESModule = lowerSuffix == QLatin1String("mjs");
    const bool isJavaScript = isESModule || lowerSuffix == QLatin1String("js");

    QFile file(m_file);
    if (!file.open(QFile::ReadOnly)) {
        QQmlJSScope::Ptr result = QQmlJSScope::create(
                    isJavaScript ? QQmlJSScope::JSLexicalScope : QQmlJSScope::QMLScope);
        result->setInternalName(scopeName);
        return result;
    }

    QString code = QString::fromUtf8(file.readAll());
    file.close();

    lexer.setCode(code, /*line = */ 1, /*qmlMode=*/ !isJavaScript);
    QQmlJS::Parser parser(&engine);

    const bool success = isJavaScript ? (isESModule ? parser.parseModule()
                                                    : parser.parseProgram())
                                      : parser.parse();
    if (!success) {
        QQmlJSScope::Ptr result = QQmlJSScope::create(
                    isJavaScript ? QQmlJSScope::JSLexicalScope : QQmlJSScope::QMLScope);
        result->setInternalName(scopeName);
        return result;
    }

    if (!isJavaScript) {
        QQmlJS::AST::UiProgram *program = parser.ast();
        QQmlJSImportVisitor membersVisitor(m_importer, QFileInfo(m_file).canonicalPath(),
                                           m_qmltypesFiles);
        program->accept(&membersVisitor);
        m_errors = membersVisitor.errors();
        return membersVisitor.result();
    }


    if (isESModule)
        return parseModule(cast<ESModule *>(parser.rootNode()), scopeName);
    else
        return parseProgram(cast<Program *>(parser.rootNode()), scopeName);
}

QT_END_NAMESPACE
