/*
This file is part of Telegram Desktop.
*/
#pragma once

#include <QAccessibleWidget>

namespace HistoryView {
class Element;
} // namespace HistoryView

namespace HistoryView::Accessibility {

class InnerAccessible final : public QAccessibleWidget {
public:
    explicit InnerAccessible(QWidget *widget);

    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;
    QAccessibleInterface *focusChild() const override;

    QString text(QAccessible::Text t) const override;
    QAccessible::Role role() const override;
};

} // namespace HistoryView::Accessibility