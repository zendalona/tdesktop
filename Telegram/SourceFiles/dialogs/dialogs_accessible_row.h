//dialog_accessible_row.h
#pragma once

#include <QAccessibleInterface>
#include <QRect>

namespace Dialogs {

class InnerWidget;

class AccessibleRow : public QAccessibleInterface {
public:
	AccessibleRow(InnerWidget *parent, int index);

	QObject *object() const override;
	bool isValid() const override;
	
	QWindow *window() const override;
	QAccessibleInterface *parent() const override;
	QAccessibleInterface *child(int index) const override;
	QAccessibleInterface *childAt(int x, int y) const override; 
    int index() const { return _index; }
	int childCount() const override;
	int indexOfChild(const QAccessibleInterface *child) const override;
	
	QString text(QAccessible::Text t) const override;
	void setText(QAccessible::Text t, const QString &text) override; 
	
	QRect rect() const override;
	QAccessible::Role role() const override;
	QAccessible::State state() const override;

private:
	InnerWidget *_parent = nullptr;
	int _index = -1;
};

} // namespace Dialogs