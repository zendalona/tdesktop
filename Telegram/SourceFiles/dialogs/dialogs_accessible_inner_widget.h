//dialogs_accessible_inner_widget.h
#pragma once

#include <QAccessibleWidget>
#include <map>

namespace Dialogs {

class InnerWidget;
class AccessibleRow;

class AccessibleInnerWidget : public QAccessibleWidget {
public:
	AccessibleInnerWidget(InnerWidget *widget);
	// ~AccessibleInnerWidget();

	QAccessibleInterface *child(int index) const override;
	int childCount() const override;
	int indexOfChild(const QAccessibleInterface *child) const override;
	
	QAccessibleInterface *focusChild() const override;
	QAccessibleInterface *childAt(int x, int y) const override;
	
	QAccessible::Role role() const override;
	QAccessible::State state() const override;

private:
	InnerWidget *_inner = nullptr;
};

} // namespace Dialogs