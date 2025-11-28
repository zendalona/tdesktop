//dialogs_accessible_inner_widget.cpp
#include "dialogs/dialogs_accessible_inner_widget.h"
#include "dialogs/dialogs_inner_widget.h"
#include "dialogs/dialogs_accessible_row.h"


namespace Dialogs {

AccessibleInnerWidget::AccessibleInnerWidget(InnerWidget *widget)
: QAccessibleWidget(widget, QAccessible::List)
, _inner(widget) {
}

QAccessibleInterface *AccessibleInnerWidget::child(int index) const {
	if (!_inner || index < 0 || index >= childCount()) {
		return nullptr;
	}

	return new AccessibleRow(_inner, index);
}
int AccessibleInnerWidget::childCount() const {
	return _inner ? _inner->getAccessibleChildCount() : 0;
}

int AccessibleInnerWidget::indexOfChild(const QAccessibleInterface *child) const {
	if (!child || !_inner) return -1;

	if (child->role() == QAccessible::ListItem) {
		const auto row = static_cast<const AccessibleRow*>(child);
		return row->index();
	}
	return -1;
}

QAccessibleInterface *AccessibleInnerWidget::childAt(int x, int y) const {
	if (!_inner) return nullptr;
	QPoint local = _inner->mapFromGlobal(QPoint(x, y));
	int index = _inner->getAccessibleIndexAt(local.y());
	if (index >= 0) {
		return child(index);
	}
	return nullptr;
}

QAccessibleInterface *AccessibleInnerWidget::focusChild() const {
	if (!_inner) return nullptr;
	if (_inner->hasFocus()) {
		int index = _inner->currentAccessibleIndex();
		if (index >= 0) {
			return child(index);
		}
	}
	return nullptr;
}

QAccessible::Role AccessibleInnerWidget::role() const {
	return QAccessible::List;
}

QAccessible::State AccessibleInnerWidget::state() const {
	auto s = QAccessibleWidget::state();
	s.focusable = false; 
	s.multiSelectable = false;
	return s;
}

} // namespace Dialogs