/*
This file is part of Telegram Desktop.
*/
#pragma once

#include <QAccessibleInterface>

class HistoryInner;

namespace HistoryView {
class Element;
} // namespace HistoryView

namespace HistoryView::Accessibility {

class ItemAccessible final : public QAccessibleInterface {
public:
    ItemAccessible(
        not_null<Element*> element,
        not_null<const HistoryInner*> parentWidget); // <-- Added const

    // QAccessibleInterface required overrides
    bool isValid() const override;
    QAccessibleInterface *parent() const override;
    int childCount() const override;
    QAccessibleInterface *child(int index) const override;
    int indexOfChild(const QAccessibleInterface *child) const override;
    QObject *object() const override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;
    QString text(QAccessible::Text t) const override;
    QRect rect() const override;
    void *interface_cast(QAccessible::InterfaceType t) override;
    QAccessibleInterface *childAt(int x, int y) const override;
    void setText(QAccessible::Text t, const QString &text) override;

    not_null<Element*> element() const {
        return _element;
    }

private:
    const not_null<Element*> _element;
    const not_null<const HistoryInner*> _parentWidget; // <-- Added const

};

} // namespace HistoryView::Accessibility