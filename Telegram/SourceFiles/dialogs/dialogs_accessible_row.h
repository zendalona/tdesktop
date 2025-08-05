// dialogs_accessible_row.h
#pragma once

#include <QAccessible>
#include <QAccessibleInterface>
#include <QAccessibleTextInterface>
#include <QPointer>
#include <QRect>
#include "dialogs_inner_widget.h"

namespace Dialogs
{

    class Row;
    class FakeRow;
    class BasicRow;
    class InnerWidget;

    class AccessibleRow : public QAccessibleInterface, public QAccessibleTextInterface
    {
    public:
        AccessibleRow(Row *row, int rowWidth, const InnerWidget *parentWidget);

        // QAccessibleInterface
        QObject *object() const override;
        bool isValid() const override;
        QAccessibleInterface *child(int) const override;
        int childCount() const override;
        int indexOfChild(const QAccessibleInterface *) const override;
        QAccessibleInterface *parent() const override;
        QRect rect() const override;
        QAccessible::Role role() const override;
        QAccessible::State state() const override;
        QString text(QAccessible::Text t) const override;
        QAccessibleInterface *childAt(int x, int y) const override;
        void setText(QAccessible::Text t, const QString &text) override;
        void *interface_cast(QAccessible::InterfaceType t) override; // ✅ REQUIRED

        // QAccessibleTextInterface
        void scrollToSubstring(int startIndex, int endIndex) override;
        int characterCount() const override;
        QString text(int startOffset, int endOffset) const override;
        int offsetAtPoint(const QPoint &point) const override;
        void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
        int selectionCount() const override;
        void addSelection(int startOffset, int endOffset) override;
        void removeSelection(int selectionIndex) override;
        void setSelection(int selectionIndex, int startOffset, int endOffset) override;
        void setCursorPosition(int position) override;
        int cursorPosition() const override;
        QRect characterRect(int offset) const override;
        QString attributes(int offset, int *startOffset, int *endOffset) const override;

        Row *row() const;

    private:
        QString _accessibleText() const;
        const InnerWidget *_parentWidget = nullptr;

        Row *_row = nullptr;
        int _width = 0;
    };

    class AccessibleFakeRow : public QAccessibleInterface
    {
    public:
        AccessibleFakeRow(FakeRow *row, int rowWidth, const InnerWidget *parentWidget);

        ~AccessibleFakeRow(); //  destructor declaration

        //  Public getter for the row pointer
        FakeRow *row() const { return _row; }

        // QAccessibleInterface overrides
        QObject *object() const override;
        bool isValid() const override;
        QAccessibleInterface *child(int) const override;
        int childCount() const override;
        int indexOfChild(const QAccessibleInterface *) const override;
        QAccessibleInterface *parent() const override;
        QRect rect() const override;
        QAccessible::Role role() const override;
        QAccessible::State state() const override;
        QString text(QAccessible::Text t) const override;
        QAccessibleInterface *childAt(int x, int y) const override;
        void setText(QAccessible::Text t, const QString &text) override;
        void *interface_cast(QAccessible::InterfaceType t) override;

    private:
        FakeRow *_row = nullptr;
        const InnerWidget *_parentWidget = nullptr;
        int _width = 0;
    };

    class AccessibleBasicRow : public QAccessibleInterface {
        public:
            AccessibleBasicRow(
                BasicRow *row,
                QString name,
                int rowWidth,
                int rowHeight,
                int top,
                const InnerWidget *parentWidget);
            ~AccessibleBasicRow();

            BasicRow *row() const { return _row; }
        
            // QAccessibleInterface overrides
            bool isValid() const override;
            QRect rect() const override;
            QString text(QAccessible::Text t) const override;
            QAccessible::Role role() const override;
            QAccessible::State state() const override;
        
            // Boilerplate overrides
            QObject *object() const override;
            QAccessibleInterface *child(int) const override;
            int childCount() const override;
            int indexOfChild(const QAccessibleInterface *) const override;
            QAccessibleInterface *parent() const override;
            QAccessibleInterface *childAt(int x, int y) const override;
            void setText(QAccessible::Text t, const QString &text) override;
            void *interface_cast(QAccessible::InterfaceType t) override;
        
        private:
            BasicRow *_row = nullptr;
            QString _name;
            const InnerWidget *_parentWidget = nullptr;
            int _width = 0;
            int _height = 0;
            int _top = 0;
        };
} // namespace Dialogs
