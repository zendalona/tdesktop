#include "history/view/accessibility/history_inner_accessible.h"
#include "history/view/accessibility/history_item_accessible.h"
#include "history/history_inner_widget.h"

namespace HistoryView::Accessibility {

namespace {
const bool Registered = [] {
    QAccessible::installFactory([](const QString &classname, QObject *object) -> QAccessibleInterface* {
        if (classname == u"HistoryInner") {
            return new InnerAccessible(qobject_cast<HistoryInner*>(object));
        }
        return nullptr;
    });
    return true;
}();
} // namespace

InnerAccessible::InnerAccessible(HistoryInner *widget)
: QAccessibleWidget(widget, QAccessible::List)
, _inner(widget) {
}

int InnerAccessible::childCount() const {
    return _inner ? _inner->getAccessibleChildCount() : 0;
}

QAccessibleInterface *InnerAccessible::child(int index) const {
    if (!_inner || index < 0 || index >= childCount()) {
        return nullptr;
    }
    return new ItemAccessible(_inner, index);
}

int InnerAccessible::indexOfChild(const QAccessibleInterface *child) const {
    if (child && child->role() == QAccessible::ListItem) {

        const auto item = static_cast<const ItemAccessible*>(child);
        return item->index();
    }
    return -1;
}

QAccessibleInterface *InnerAccessible::childAt(int x, int y) const {
    if (!_inner) return nullptr;
    QPoint local = _inner->mapFromGlobal(QPoint(x, y));
    int index = _inner->getAccessibleIndexAt(local.y());
    if (index >= 0) {
        return child(index);
    }
    return nullptr;
}

QAccessibleInterface *InnerAccessible::focusChild() const {
    if (!_inner) return nullptr;
    
    if (_inner->hasFocus()) {
        int index = _inner->currentAccessibleIndex();
        if (index >= 0) {
            return child(index);
        }
    }
    return nullptr;
}

QAccessible::Role InnerAccessible::role() const {
    return QAccessible::List;
}

QAccessible::State InnerAccessible::state() const {
    auto s = QAccessibleWidget::state();
    s.focusable = false;
    return s;
}

} // namespace HistoryView::Accessibility