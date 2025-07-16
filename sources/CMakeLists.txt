set(SCILATEX_SOURCES
    main.cpp
    mainwindow.cpp mainwindow.h
    keyutils.h
    navigation.cpp navigation.h
    matrixmode.cpp matrixmode.h
    bracketchooserdialog.cpp bracketchooserdialog.h
    operatordialog.cpp operatordialog.h
    preferencesmanager.cpp preferencesmanager.h
    preferencesdialog.cpp preferencesdialog.h
    matlabutils.cpp matlabutils.h
    fontdialog.cpp fontdialog.h
    mathnamedialog.cpp mathnamedialog.h
    labeldialog.cpp labeldialog.h
    decoratordialog.cpp decoratordialog.h
    spacingdialog.cpp spacingdialog.h
    previewwindow.cpp previewwindow.h
    recentfilesdialog.cpp recentfilesdialog.h
)

qt_add_executable(SciLatex
    MANUAL_FINALIZATION
    ${SCILATEX_SOURCES}
)

target_include_directories(SciLatex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/third_party
    ${CURL_INCLUDE_DIR}
)

target_link_libraries(SciLatex PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
    Qt6::Pdf
    Qt6::PdfWidgets
    Qt6::Core
    libcurl
)

set_target_properties(SciLatex PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(SciLatex)
