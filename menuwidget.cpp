#include "menuwidget.h"

menuWidget::menuWidget()
    : QQuickWidget(QUrl(QStringLiteral("qrc:///qml/main.qml")))
{
}

menuWidget::menuWidget(QWidget *parent)
    : QQuickWidget(QUrl(QStringLiteral("qrc:///qml/main.qml")),parent)
{
}
