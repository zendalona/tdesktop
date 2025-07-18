//dialogs_accessible_inner_widget.h
#pragma once

#include <QAccessibleWidget>

namespace Dialogs {

class InnerWidget;

class AccessibleInnerWidget : public QAccessibleWidget {
public:
    explicit AccessibleInnerWidget(InnerWidget *widget);

    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;
    QAccessibleInterface *focusChild() const override;

    QAccessibleInterface *childAt(int x, int y) const override;
    void setText(QAccessible::Text t, const QString &text) override;
    QString text(QAccessible::Text t) const override;

    QAccessible::Role role() const override;
    QAccessible::State state() const override;
};

}  // namespace Dialogs
