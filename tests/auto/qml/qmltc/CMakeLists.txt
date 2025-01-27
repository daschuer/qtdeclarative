set(cpp_sources
    tst_qmltc.h tst_qmltc.cpp

    # test files:
    nameconflict.h nameconflict.cpp
    # attached types:
    cpptypes/testattachedtype.h cpptypes/testattachedtype.cpp
    # grouped types:
    cpptypes/testgroupedtype.h cpptypes/testgroupedtype.cpp
    # private properties:
    cpptypes/testprivateproperty.h cpptypes/testprivateproperty.cpp
    cpptypes/private/testprivateproperty_p.h

    cpptypes/typewithproperties.h cpptypes/typewithproperties.cpp
)

set(qml_sources
    data/HelloWorld.qml
    data/NameConflict.qml
    data/simpleQtQuickTypes.qml
    data/typeWithEnums.qml
    data/methods.qml
    data/properties.qml
    data/ObjectWithId.qml
    data/documentWithIds.qml
    data/importNamespace.qml
    data/ComponentType.qml
    data/componentTypes.qml

    data/signalHandlers.qml
    data/javaScriptFunctions.qml
    data/changingBindings.qml
    data/propertyAlias.qml
    data/propertyAlias_external.qml
    data/propertyChangeHandler.qml
    data/NestedHelloWorld.qml
    data/ComponentHelloWorld.qml
    data/listProperty.qml
    data/listPropertySameName.qml
    data/defaultProperty.qml
    data/defaultPropertyCorrectSelection.qml
    # data/defaultAlias.qml
    data/propertyReturningFunction.qml
    data/attachedProperty.qml
    data/groupedProperty.qml
    data/groupedProperty_qquicktext.qml
    data/localImport.qml
    data/localImport_explicit.qml
    data/newPropertyBoundToOld.qml
    data/oldPropertyBoundToNew.qml
    data/nonLocalQmlPropertyBoundToAny.qml
    data/localDerived.qml
    data/justAnimation.qml
    data/justAnimationOnAlias.qml
    data/behaviorAndAnimation.qml
    data/behaviorAndAnimationOnAlias.qml
    data/singletonUser.qml
    data/bindingsThroughIds.qml
    data/localImport_context.qml
    data/neighbors_context.qml
    data/delegate_context.qml
    data/nontrivial_context.qml
    data/javascriptCaller.qml
    data/listView.qml
    data/bindingOnValueType.qml
    data/keyEvents.qml
    data/complexAliases.qml
    data/PrivateProperty.qml
    data/privatePropertySubclass.qml
    data/calqlatrBits.qml
    data/propertyChangeAndSignalHandlers.qml

    # support types:
    data/DefaultPropertySingleChild.qml
    data/DefaultPropertyManyChildren.qml
    data/LocallyImported.qml
    data/LocalWithOnCompleted.qml
    data/LocallyImported_context.qml
    data/SingletonThing.qml
)

set(js_sources
    data/subfolder/code.js
)

set(common_libraries
    Qt::Core
    Qt::QmlPrivate
    Qt::QuickPrivate
    Qt::TestPrivate
    Qt::Gui # QColor, QMatrix4x4, ...
)

set_source_files_properties(data/NameConflict.qml PROPERTIES
    QT_QMLTC_FILE_BASENAME ResolvedNameConflict)

qt_internal_add_test(tst_qmltc_diskcache
    SOURCES
        ${cpp_sources}
    LIBRARIES
        ${common_libraries}
    INCLUDE_DIRECTORIES
        cpptypes/
)
qt6_add_qml_module(tst_qmltc_diskcache
    VERSION 1.0
    URI QmltcTests
    QML_FILES
        ${qml_sources}
        ${js_sources}
    OUTPUT_DIRECTORY diskcache/QmltcTests
)
target_compile_definitions(tst_qmltc_diskcache PRIVATE
    QMLTC_TESTS_DISABLE_CACHE=0
)
qt_internal_target_compile_qml_to_cpp(tst_qmltc_diskcache
    NAMESPACE QmltcTest
    FILES
        ${qml_sources}
)

qt_internal_add_test(tst_qmltc_nodiskcache
    SOURCES
        ${cpp_sources}
    LIBRARIES
        ${common_libraries}
    INCLUDE_DIRECTORIES
        cpptypes/
)
qt6_add_qml_module(tst_qmltc_nodiskcache
    VERSION 1.0
    URI QmltcTests
    QML_FILES
        ${qml_sources}
        ${js_sources}
    OUTPUT_DIRECTORY nodiskcache/QmltcTests
)
target_compile_definitions(tst_qmltc_nodiskcache PRIVATE
    QMLTC_TESTS_DISABLE_CACHE=1
)
qt_internal_target_compile_qml_to_cpp(tst_qmltc_nodiskcache
    # NAMESPACE QmltcTest # use QT_NAMESPACE instead
    FILES
        ${qml_sources}
)

# Add qmltc documentation example to the tests. This is not beautiful but allows
# to nicely test the documentation snippets automatically (and so making sure
# the code there is up-to-date)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../src/qml/doc/snippets/qmltc snippets)
