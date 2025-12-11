#pragma once

#include <QAccessibleInterface>
#include <QRect>

class HistoryInner;

namespace HistoryView::Accessibility {

class ItemAccessible : public QAccessibleInterface {
public:
    ItemAccessible(HistoryInner *parent, int index);

    // QAccessibleInterface overrides
    QObject *object() const override;
    bool isValid() const override;
    QAccessibleInterface *child(int index) const override;
    int childCount() const override;
    int indexOfChild(const QAccessibleInterface *child) const override;
    QAccessibleInterface *parent() const override;
    
    // Geometry & Hit Testing
    QRect rect() const override;
    QAccessibleInterface *childAt(int x, int y) const override;

    // Text & State
    QString text(QAccessible::Text t) const override;
    void setText(QAccessible::Text t, const QString &text) override;
    QAccessible::Role role() const override;
    QAccessible::State state() const override;
    
    void *interface_cast(QAccessible::InterfaceType t) override;

    // Direct access to index
    int index() const { return _index; }

private:
    HistoryInner *_parent = nullptr;
    int _index = -1;
};

} // namespace HistoryView::Accessibility