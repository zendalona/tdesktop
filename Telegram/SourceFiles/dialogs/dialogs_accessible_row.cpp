//dialogs_accessible_row.cpp
#include "dialogs/dialogs_accessible_row.h"
#include "dialogs/dialogs_inner_widget.h"

#include <QWindow>

namespace Dialogs {

AccessibleRow::AccessibleRow(InnerWidget *parent, int index)
: _parent(parent)
, _index(index) {
}

QObject *AccessibleRow::object() const {
	return nullptr;
}

bool AccessibleRow::isValid() const {
    // Ensure parent exists
    if (!_parent) return false;
    
    // Ensure index is positive
    if (_index < 0) return false;

    // Ensure index is within the TOTAL count of children
    // If this returns false for index 1, navigation stops.
    return _index < _parent->getAccessibleChildCount();
}

QWindow *AccessibleRow::window() const {
	return _parent ? _parent->window()->windowHandle() : nullptr;
}

QAccessibleInterface *AccessibleRow::parent() const {
	return QAccessible::queryAccessibleInterface(_parent);
}

QAccessibleInterface *AccessibleRow::child(int index) const {
	return nullptr;
}

QAccessibleInterface *AccessibleRow::childAt(int x, int y) const {
	return nullptr;
}

int AccessibleRow::childCount() const {
	return 0;
}

int AccessibleRow::indexOfChild(const QAccessibleInterface *child) const {
	return -1;
}

QString AccessibleRow::text(QAccessible::Text t) const {
	if (!_parent) return QString();
	
	if (t == QAccessible::Name) {
		return _parent->getAccessibleName(_index);
	} else if (t == QAccessible::Description || t == QAccessible::Value) {
		return _parent->getAccessibleDescription(_index);
	}
	return QString();
}

void AccessibleRow::setText(QAccessible::Text t, const QString &text) {
}

QRect AccessibleRow::rect() const {
	if (!_parent) return QRect();
	
	// Use the helper on InnerWidget to calculate rect based on index
	// This keeps the logic centralized.
	QRect local = _parent->getAccessibleRect(_index);
	
	// If the item is logically selected but physically scrolled out of view,
	// we must return a valid on-screen rect (e.g. at the edge) or the 
	// screen reader will refuse to focus it.
	if (local.isEmpty()) {
		// Fallback: Use parent's top-left so it's at least "somewhere"
		return QRect(_parent->mapToGlobal(QPoint(0,0)), QSize(1,1));
	}
	
	QPoint globalTopLeft = _parent->mapToGlobal(local.topLeft());
	return QRect(globalTopLeft, local.size());
}

QAccessible::Role AccessibleRow::role() const {
	return QAccessible::ListItem;
}

QAccessible::State AccessibleRow::state() const {
	QAccessible::State s;
	s.selectable = true;
	s.focusable = true;
	// s.enabled = true; // REMOVED (Caused error)
	// Qt items are enabled by default unless s.disabled = true;
	
	if (_parent && _parent->isAccessibleRowSelected(_index)) {
		s.selected = true;
		s.focused = true; // Force focused state
	}
	return s;
}

} // namespace Dialogs