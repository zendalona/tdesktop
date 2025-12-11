#pragma once

#include <QAccessibleWidget>

class HistoryInner;

namespace HistoryView::Accessibility {

class InnerAccessible : public QAccessibleWidget {
public:
    explicit InnerAccessible(HistoryInner *widget);

    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;
    
    QAccessibleInterface *focusChild() const override;
    QAccessibleInterface *childAt(int x, int y) const override;

    QAccessible::Role role() const override;
    QAccessible::State state() const override;
    
private:
    HistoryInner *_inner = nullptr;
};

} // namespace HistoryView::Accessibility