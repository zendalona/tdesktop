// dialogs_accessible_inner_widget.cpp
#include "dialogs_accessible_inner_widget.h"
#include "dialogs_accessible_row.h"
#include "dialogs_inner_widget.h"
#include "dialogs_row.h"
#include <iostream>
namespace Dialogs {

    const bool registered = [] {
        QAccessible::installFactory([](const QString &classname, QObject *object) -> QAccessibleInterface * {
            if (classname == "Dialogs::InnerWidget") {
                return new AccessibleInnerWidget(qobject_cast<InnerWidget *>(object));
            }
            return nullptr;
        });
        return true;
    }();
    
AccessibleInnerWidget::AccessibleInnerWidget(InnerWidget *widget)
    : QAccessibleWidget(widget, QAccessible::List) {}

int AccessibleInnerWidget::childCount() const {
    const auto *widget = static_cast<InnerWidget*>(object());
    if (!widget) return 0;
    return static_cast<int>(widget->accessibleRows().size());
}

QAccessibleInterface *AccessibleInnerWidget::child(int index) const {
    const auto *widget = static_cast<InnerWidget*>(object());
    if (!widget) return nullptr;

    const auto &rows = widget->accessibleRows();
    if (index < 0 || index >= static_cast<int>(rows.size())) return nullptr;

    int rowWidth = widget->width();
    return new AccessibleRow(rows.at(index), rowWidth, static_cast<InnerWidget*>(object()));  // or static_cast<InnerWidget*>(object())

}


int AccessibleInnerWidget::indexOfChild(const QAccessibleInterface *child) const {
    const auto *widget = static_cast<InnerWidget*>(object());
    if (!widget || !child) return -1;

    const auto &rows = widget->accessibleRows();
    const auto *accessibleRow = dynamic_cast<const AccessibleRow *>(child);
    if (!accessibleRow) return -1;

    auto row = accessibleRow->row();
    auto it = std::find(rows.begin(), rows.end(), row);
    return (it != rows.end()) ? static_cast<int>(std::distance(rows.begin(), it)) : -1;
}

QAccessibleInterface *AccessibleInnerWidget::childAt(int, int) const {
    return nullptr;
}

QAccessibleInterface *AccessibleInnerWidget::focusChild() const {
	auto *widget = static_cast<InnerWidget *>(object());
	if (!widget) return nullptr;

	auto *focused = widget->focusedRow();
	if (!focused) return nullptr;

	auto *child = new AccessibleRow(focused, widget->width(), widget);
	std::cout << "focusChild Name: " << child->text(QAccessible::Name).toStdString() << std::endl;
	return child;
}




void AccessibleInnerWidget::setText(QAccessible::Text, const QString &) {
    // No-op
}

QString AccessibleInnerWidget::text(QAccessible::Text t) const {
    if (t == QAccessible::Name) {
        return QStringLiteral("Chat List");
    }
    return QString();
}

QAccessible::Role AccessibleInnerWidget::role() const {
    return QAccessible::List;
}

QAccessible::State AccessibleInnerWidget::state() const {
    QAccessible::State s;
    s.focusable = false;
    s.multiSelectable = true;
    return s;

}



} // namespace Dialogs
