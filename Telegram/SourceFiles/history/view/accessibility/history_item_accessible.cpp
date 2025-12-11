#include "history/view/accessibility/history_item_accessible.h"
#include "history/history_inner_widget.h"
#include <QWindow>

namespace HistoryView::Accessibility {

ItemAccessible::ItemAccessible(HistoryInner *parent, int index)
: _parent(parent)
, _index(index) {
}

QObject *ItemAccessible::object() const {
    return nullptr;
}

bool ItemAccessible::isValid() const {
    return _parent && _index >= 0 && _index < _parent->getAccessibleChildCount();
}

QAccessibleInterface *ItemAccessible::child(int index) const {
    return nullptr; 
}

int ItemAccessible::childCount() const {
    return 0;
}

int ItemAccessible::indexOfChild(const QAccessibleInterface *child) const {
    return -1;
}

QAccessibleInterface *ItemAccessible::parent() const {
    if (!_parent) return nullptr;
    return QAccessible::queryAccessibleInterface(_parent);
}

QRect ItemAccessible::rect() const {
    if (!_parent) return QRect();
    
    QRect local = _parent->getAccessibleRect(_index);
    if (local.isEmpty()) {
        return QRect(_parent->mapToGlobal(QPoint(0,0)), QSize(1,1));
    }
    
    QPoint globalTopLeft = _parent->mapToGlobal(local.topLeft());
    return QRect(globalTopLeft, local.size());
}


QAccessibleInterface *ItemAccessible::childAt(int x, int y) const {
    return nullptr;
}


QString ItemAccessible::text(QAccessible::Text t) const {
    if (!_parent) return QString();

    if (t == QAccessible::Name) {
        return _parent->getAccessibleName(_index);
    } else if (t == QAccessible::Description || t == QAccessible::Value) {
        return _parent->getAccessibleDescription(_index);
    }
    return QString();
}

void ItemAccessible::setText(QAccessible::Text t, const QString &text) {
}

QAccessible::Role ItemAccessible::role() const {
    return QAccessible::ListItem;
}

QAccessible::State ItemAccessible::state() const {
    QAccessible::State s;
    s.selectable = true;
    s.focusable = true;
    
    if (_parent && _parent->isAccessibleItemSelected(_index)) {
        s.selected = true;
        s.focused = true;
    }
    return s;
}

void *ItemAccessible::interface_cast(QAccessible::InterfaceType t) {
    return nullptr; 
}

} // namespace HistoryView::Accessibility