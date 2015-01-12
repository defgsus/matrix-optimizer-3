RESOURCES += \
    icons.qrc \
    help.qrc

HEADERS += \
    src/gui/mainwindow.h \
    src/gui/timeline1drulerview.h \
    src/gui/basic3dview.h \
    src/gui/timeline1dview.h \
    src/gui/painter/grid.h \
    src/gui/util/viewspace.h \
    src/gui/ruler.h \
    src/gui/qobjectinspector.h \
    src/gui/parameterview.h \
    src/gui/sequenceview.h \
    src/gui/sequencefloatview.h \
    src/gui/painter/valuecurve.h \
    src/gui/painter/sequenceoverpaint.h \
    src/gui/generalsequencefloatview.h \
    src/gui/objectview.h \
    src/gui/trackview.h \
    src/gui/sequencer.h \
    src/gui/widget/sequencewidget.h \
    src/gui/trackheader.h \
    src/gui/widget/trackheaderwidget.h \
    src/gui/widget/doublespinbox.h \
    src/gui/widget/equationeditor.h \
    src/gui/widget/timebar.h \
    src/gui/trackviewoverpaint.h \
    src/gui/widget/spacer.h \
    src/gui/util/objectmenu.h \
    src/gui/util/scenesettings.h \
    src/gui/audiodialog.h \
    src/gui/objectinfodialog.h \
    src/gui/splashscreen.h \
    src/gui/widget/spinbox.h \
    src/gui/widget/basic3dwidget.h \
    src/gui/widget/geometrywidget.h \
    src/gui/geometrydialog.h \
    src/gui/widget/modulatorwidget.h \
    src/gui/modulatordialog.h \
    src/gui/widget/envelopewidget.h \
    src/gui/widget/transportwidget.h \
    src/gui/widget/groupwidget.h \
    src/gui/widget/geometrymodifierwidget.h \
    src/gui/widget/doublespinboxclean.h \
    src/gui/geometryexportdialog.h \
    src/gui/texteditdialog.h \
    src/gui/sceneconvertdialog.h \
    src/gui/widget/domepreviewwidget.h \
    src/gui/projectorsetupdialog.h \
    src/gui/helpdialog.h \
    src/gui/widget/helptextbrowser.h \
    src/gui/widget/equationdisplaywidget.h \
    src/gui/equationdisplaydialog.h \
    src/gui/networkdialog.h \
    src/gui/midisettingsdialog.h \
    src/gui/saveequationdialog.h \
    src/gui/infowindow.h \
    src/gui/serverdialog.h \
    src/gui/widget/netlogwidget.h \
    src/gui/widget/overlapareaeditwidget.h \
    src/gui/widget/filterresponsewidget.h \
    src/gui/audiofilterdialog.h \
    src/gui/timelineeditdialog.h \
    src/gui/clipview.h \
    src/gui/widget/clipwidget.h \
    src/gui/widget/clipwidgetbutton.h \
    src/gui/keepmodulatordialog.h \
    src/gui/util/mainwidgetcontroller.h \
    src/gui/resolutiondialog.h \
    src/gui/objectgraphview.h \
    src/gui/item/abstractobjectitem.h \
    src/gui/util/objectgraphsettings.h \
    src/gui/item/objectgraphexpanditem.h \
    src/gui/util/objectgraphscene.h \
    src/gui/item/modulatoritem.h \
    src/gui/widget/objectlistwidget.h \
    src/gui/widget/objectlistwidgetitem.h \
    src/gui/item/audioconnectionitem.h \
    src/gui/item/objectgraphconnectitem.h \
    src/gui/widget/parameterwidget.h \
    src/gui/widget/angelscriptwidget.h \
    src/gui/widget/abstractscriptwidget.h \
    src/gui/widget/glslwidget.h \
    src/gui/widget/texteditwidget.h \
    src/gui/util/appicons.h

SOURCES += \
    src/gui/objectview.cpp \
    src/gui/qobjectinspector.cpp \
    src/gui/mainwindow.cpp \
    src/gui/basic3dview.cpp \
    src/gui/timeline1dview.cpp \
    src/gui/painter/grid.cpp \
    src/gui/ruler.cpp \
    src/gui/timeline1drulerview.cpp \
    src/gui/util/viewspace.cpp \
    src/gui/parameterview.cpp \
    src/gui/sequenceview.cpp \
    src/gui/sequencefloatview.cpp \
    src/gui/painter/valuecurve.cpp \
    src/gui/painter/sequenceoverpaint.cpp \
    src/gui/generalsequencefloatview.cpp \
    src/gui/trackview.cpp \
    src/gui/sequencer.cpp \
    src/gui/widget/sequencewidget.cpp \
    src/gui/trackheader.cpp \
    src/gui/widget/trackheaderwidget.cpp \
    src/gui/widget/doublespinbox.cpp \
    src/gui/widget/equationeditor.cpp \
    src/gui/widget/timebar.cpp \
    src/gui/trackviewoverpaint.cpp \
    src/gui/widget/spacer.cpp \
    src/gui/util/objectmenu.cpp \
    src/gui/util/scenesettings.cpp \
    src/gui/audiodialog.cpp \
    src/gui/objectinfodialog.cpp \
    src/gui/splashscreen.cpp \
    src/gui/widget/spinbox.cpp \
    src/gui/widget/basic3dwidget.cpp \
    src/gui/widget/geometrywidget.cpp \
    src/gui/geometrydialog.cpp \
    src/gui/widget/modulatorwidget.cpp \
    src/gui/modulatordialog.cpp \
    src/gui/widget/envelopewidget.cpp \
    src/gui/widget/transportwidget.cpp \
    src/gui/widget/groupwidget.cpp \
    src/gui/widget/geometrymodifierwidget.cpp \
    src/gui/widget/doublespinboxclean.cpp \
    src/gui/geometryexportdialog.cpp \
    src/gui/texteditdialog.cpp \
    src/gui/sceneconvertdialog.cpp \
    src/gui/widget/domepreviewwidget.cpp \
    src/gui/projectorsetupdialog.cpp \
    src/gui/helpdialog.cpp \
    src/gui/widget/helptextbrowser.cpp \
    src/gui/widget/equationdisplaywidget.cpp \
    src/gui/equationdisplaydialog.cpp \
    src/gui/networkdialog.cpp \
    src/gui/midisettingsdialog.cpp \
    src/gui/saveequationdialog.cpp \
    src/gui/infowindow.cpp \
    src/gui/serverdialog.cpp \
    src/gui/widget/netlogwidget.cpp \
    src/gui/widget/overlapareaeditwidget.cpp \
    src/gui/widget/filterresponsewidget.cpp \
    src/gui/audiofilterdialog.cpp \
    src/gui/timelineeditdialog.cpp \
    src/gui/clipview.cpp \
    src/gui/widget/clipwidget.cpp \
    src/gui/widget/clipwidgetbutton.cpp \
    src/gui/keepmodulatordialog.cpp \
    src/gui/util/mainwidgetcontroller.cpp \
    src/gui/resolutiondialog.cpp \
    src/gui/objectgraphview.cpp \
    src/gui/item/abstractobjectitem.cpp \
    src/gui/util/objectgraphsettings.cpp \
    src/gui/item/objectgraphexpanditem.cpp \
    src/gui/util/objectgraphscene.cpp \
    src/gui/item/modulatoritem.cpp \
    src/gui/widget/objectlistwidget.cpp \
    src/gui/widget/objectlistwidgetitem.cpp \
    src/gui/item/audioconnectionitem.cpp \
    src/gui/item/objectgraphconnectitem.cpp \
    src/gui/widget/parameterwidget.cpp \
    src/gui/widget/angelscriptwidget.cpp \
    src/gui/widget/abstractscriptwidget.cpp \
    src/gui/widget/glslwidget.cpp \
    src/gui/widget/texteditwidget.cpp \
    src/gui/util/appicons.cpp
