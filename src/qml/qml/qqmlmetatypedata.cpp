/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
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

#include "qqmlmetatypedata_p.h"

#include <private/qqmltype_p_p.h>
#include <private/qqmltypemodule_p.h>
#include <private/qqmlpropertycache_p.h>

QT_BEGIN_NAMESPACE

QQmlMetaTypeData::QQmlMetaTypeData()
{
}

QQmlMetaTypeData::~QQmlMetaTypeData()
{
    for (auto iter = compositeTypes.cbegin(), end = compositeTypes.cend(); iter != end; ++iter)
        iter.value()->isRegistered = false;

    propertyCaches.clear();
    // Do this before the attached properties disappear.
    types.clear();
    undeletableTypes.clear();
    qDeleteAll(metaTypeToValueType);
}

// This expects a "fresh" QQmlTypePrivate and adopts its reference.
void QQmlMetaTypeData::registerType(QQmlTypePrivate *priv)
{
    for (int i = 0; i < types.count(); ++i) {
        if (!types.at(i).isValid()) {
            types[i] = QQmlType(priv);
            priv->index = i;
            priv->release();
            return;
        }
    }
    types.append(QQmlType(priv));
    priv->index = types.count() - 1;
    priv->release();
}

QQmlMetaTypeData::VersionedUri::VersionedUri(const std::unique_ptr<QQmlTypeModule> &module)
    : uri(module->module()), majorVersion(module->majorVersion())
{
}

QQmlTypeModule *QQmlMetaTypeData::findTypeModule(const QString &module, QTypeRevision version)
{
    const auto qqtm = std::lower_bound(
                uriToModule.begin(), uriToModule.end(), VersionedUri(module, version),
                std::less<QQmlMetaTypeData::VersionedUri>());
    if (qqtm == uriToModule.end())
        return nullptr;

    QQmlTypeModule *candidate = qqtm->get();
    return (candidate->module() == module && candidate->majorVersion() == version.majorVersion())
            ? candidate
            : nullptr;
}

QQmlTypeModule *QQmlMetaTypeData::addTypeModule(std::unique_ptr<QQmlTypeModule> module)
{
    QQmlTypeModule *ret = module.get();
    uriToModule.emplace_back(std::move(module));
    std::sort(uriToModule.begin(), uriToModule.end(),
              [](const std::unique_ptr<QQmlTypeModule> &a,
                 const std::unique_ptr<QQmlTypeModule> &b) {
        const int diff = a->module().compare(b->module());
        return diff < 0 || (diff == 0 && a->majorVersion() < b->majorVersion());
    });
    return ret;
}

bool QQmlMetaTypeData::registerModuleTypes(const QString &uri)
{
    auto function = moduleTypeRegistrationFunctions.constFind(uri);
    if (function != moduleTypeRegistrationFunctions.constEnd()) {
        (*function)();
        return true;
    }
    return false;
}

QQmlRefPointer<QQmlPropertyCache> QQmlMetaTypeData::propertyCacheForVersion(
        int index, QTypeRevision version) const
{
    return (index < typePropertyCaches.length())
            ? typePropertyCaches.at(index).value(version)
            : QQmlRefPointer<QQmlPropertyCache>();
}

void QQmlMetaTypeData::setPropertyCacheForVersion(int index, QTypeRevision version,
                                                  const QQmlRefPointer<QQmlPropertyCache> &cache)
{
    if (index >= typePropertyCaches.length())
        typePropertyCaches.resize(index + 1);
    typePropertyCaches[index][version] = cache;
}

void QQmlMetaTypeData::clearPropertyCachesForVersion(int index)
{
    if (index < typePropertyCaches.length())
        typePropertyCaches[index].clear();
}

QQmlRefPointer<QQmlPropertyCache> QQmlMetaTypeData::propertyCache(
        const QMetaObject *metaObject, QTypeRevision version)
{
    if (QQmlRefPointer<QQmlPropertyCache> rv = propertyCaches.value(metaObject))
        return rv;

    QQmlRefPointer<QQmlPropertyCache> rv;
    if (const QMetaObject *superMeta = metaObject->superClass())
        rv = propertyCache(superMeta, version)->copyAndAppend(metaObject, version);
    else
        rv.adopt(new QQmlPropertyCache(metaObject));

    const auto *mop = reinterpret_cast<const QMetaObjectPrivate *>(metaObject->d.data);
    if (!(mop->flags & DynamicMetaObject))
        propertyCaches.insert(metaObject, rv);

    return rv;
}

QQmlRefPointer<QQmlPropertyCache> QQmlMetaTypeData::propertyCache(
        const QQmlType &type, QTypeRevision version)
{
    Q_ASSERT(type.isValid());

    if (auto pc = propertyCacheForVersion(type.index(), version))
        return pc;

    QVector<QQmlType> types;

    quint8 maxMinorVersion = 0;

    const QMetaObject *metaObject = type.metaObject();

    const QTypeRevision combinedVersion = version.hasMajorVersion()
            ? version
            : (version.hasMinorVersion()
               ? QTypeRevision::fromVersion(type.version().majorVersion(),
                                            version.minorVersion())
               : QTypeRevision::fromMajorVersion(type.version().majorVersion()));

    while (metaObject) {
        QQmlType t = QQmlMetaType::qmlType(metaObject, type.module(), combinedVersion);
        if (t.isValid()) {
            maxMinorVersion = qMax(maxMinorVersion, t.version().minorVersion());
            types << t;
        } else {
            types << QQmlType();
        }

        metaObject = metaObject->superClass();
    }

    const QTypeRevision maxVersion = QTypeRevision::fromVersion(combinedVersion.majorVersion(),
                                                                maxMinorVersion);
    if (auto pc = propertyCacheForVersion(type.index(), maxVersion)) {
        setPropertyCacheForVersion(type.index(), maxVersion, pc);
        return pc;
    }

    QQmlRefPointer<QQmlPropertyCache> raw = propertyCache(type.metaObject(), combinedVersion);

    bool hasCopied = false;

    for (int ii = 0; ii < types.count(); ++ii) {
        const QQmlType &currentType = types.at(ii);
        if (!currentType.isValid())
            continue;

        QTypeRevision rev = currentType.metaObjectRevision();
        int moIndex = types.count() - 1 - ii;

        if (raw->allowedRevision(moIndex) != rev) {
            if (!hasCopied) {
                // TODO: The copy should be mutable, and the original should be const
                //       Considering this, the setAllowedRevision() below does not violate
                //       the immutability of already published property caches.
                raw = raw->copy();
                hasCopied = true;
            }
            raw->setAllowedRevision(moIndex, rev);
        }
    }

    // Test revision compatibility - the basic rule is:
    //    * Anything that is excluded, cannot overload something that is not excluded *

    // Signals override:
    //    * other signals and methods of the same name.
    //    * properties named on<Signal Name>
    //    * automatic <property name>Changed notify signals

    // Methods override:
    //    * other methods of the same name

    // Properties override:
    //    * other elements of the same name

#if 0
    bool overloadError = false;
    QString overloadName;

    for (QQmlPropertyCache::StringCache::ConstIterator iter = raw->stringCache.begin();
         !overloadError && iter != raw->stringCache.end();
         ++iter) {

        QQmlPropertyData *d = *iter;
        if (raw->isAllowedInRevision(d))
            continue; // Not excluded - no problems

        // check that a regular "name" overload isn't happening
        QQmlPropertyData *current = d;
        while (!overloadError && current) {
            current = d->overrideData(current);
            if (current && raw->isAllowedInRevision(current))
                overloadError = true;
        }
    }

    if (overloadError) {
        if (hasCopied) raw->release();

        error.setDescription(QLatin1String("Type ") + type.qmlTypeName() + QLatin1Char(' ') + QString::number(type.majorVersion()) + QLatin1Char('.') + QString::number(minorVersion) + QLatin1String(" contains an illegal property \"") + overloadName + QLatin1String("\".  This is an error in the type's implementation."));
        return 0;
    }
#endif

    setPropertyCacheForVersion(type.index(), version, raw);

    if (version != maxVersion)
        setPropertyCacheForVersion(type.index(), maxVersion, raw);

    return raw;
}

static QQmlRefPointer<QQmlPropertyCache> propertyCacheForPotentialInlineComponentType(
        QMetaType t,
        const QHash<const QtPrivate::QMetaTypeInterface *,
                    QV4::ExecutableCompilationUnit *>::const_iterator &iter) {
    if (t != (*iter)->typeIds.id) {
        // this is an inline component, and what we have in the iterator is currently the parent compilation unit
        for (auto &&icDatum: (*iter)->inlineComponentData)
            if (icDatum.typeIds.id == t)
                return (*iter)->propertyCaches.at(icDatum.objectIndex);
    }
    return (*iter)->rootPropertyCache();
}

QQmlRefPointer<QQmlPropertyCache> QQmlMetaTypeData::findPropertyCacheInCompositeTypes(
        QMetaType t) const
{
    auto iter = compositeTypes.constFind(t.iface());
    return (iter == compositeTypes.constEnd())
            ? QQmlRefPointer<QQmlPropertyCache>()
            : propertyCacheForPotentialInlineComponentType(t, iter);
}

QT_END_NAMESPACE
