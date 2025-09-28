/*
This file is part of Telegram Desktop.
*/
#include "history/view/accessibility/history_inner_accessible.h"
#include "history/view/accessibility/history_item_accessible.h"
#include "history/history_inner_widget.h"
#include "history/history.h"

#include <QAccessible>

namespace HistoryView::Accessibility {

namespace {

// Self-registering factory, like in the dialogs example.
const bool Registered = [] {
    QAccessible::installFactory([](const QString &classname, QObject *object) -> QAccessibleInterface* {
        if (classname == u"HistoryInner") {
            return new InnerAccessible(qobject_cast<QWidget*>(object));
        }
        return nullptr;
    });
    return true;
}();

} // namespace

InnerAccessible::InnerAccessible(QWidget *widget)
: QAccessibleWidget(widget, QAccessible::List) {
}

int InnerAccessible::childCount() const {
    if (const auto *widget = qobject_cast<const HistoryInner*>(object())) {
        return widget->accessibleElements().size();
    }
    return 0;
}

QAccessibleInterface *InnerAccessible::child(int index) const {
    const auto *widget = qobject_cast<HistoryInner*>(object());
    if (!widget || index < 0) {
        return nullptr;
    }

    const auto elements = widget->accessibleElements();
    if (index >= elements.size()) {
        return nullptr;
    }

    return new ItemAccessible(elements[index], widget);
}

int InnerAccessible::indexOfChild(const QAccessibleInterface *child) const {
    if (!child) {
        return -1;
    }
    const auto *widget = qobject_cast<const HistoryInner*>(object());
    const auto *item = dynamic_cast<const ItemAccessible*>(child);
    if (!widget || !item) {
        return -1;
    }

    const auto elements = widget->accessibleElements();
    const auto it = std::find(elements.cbegin(), elements.cend(), item->element());
    if (it != elements.cend()) {
        return std::distance(elements.cbegin(), it);
    }
    return -1;
}

QAccessibleInterface *InnerAccessible::focusChild() const {
    const auto *widget = qobject_cast<const HistoryInner*>(object());
    if (!widget) {
        return nullptr;
    }
    const auto element = widget->keyNavElement();
    if (!element) {
        return nullptr;
    }
    const auto elements = widget->accessibleElements();
    const auto it = std::find(elements.cbegin(), elements.cend(), element);
    if (it == elements.cend()) {
        return nullptr;
    }
    return child(std::distance(elements.cbegin(), it));
}

QString InnerAccessible::text(QAccessible::Text t) const {
    if (t == QAccessible::Name) {
        return QStringLiteral("Message History");
    }
    return QString();
}

QAccessible::Role InnerAccessible::role() const {
    return QAccessible::List;
}

} // namespace HistoryView::Accessibility