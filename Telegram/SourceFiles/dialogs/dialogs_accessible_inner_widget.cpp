// dialogs_accessible_inner_widget.cpp
#include "dialogs_accessible_inner_widget.h"
#include "dialogs_accessible_row.h"
#include "dialogs_inner_widget.h"
#include "dialogs_row.h"
#include <iostream>
#include "dialogs_indexed_list.h"
#include "styles/style_dialogs.h"
#include "styles/style_chat_helpers.h"

namespace Dialogs
{

    const bool registered = []
    {
        QAccessible::installFactory([](const QString &classname, QObject *object) -> QAccessibleInterface *
                                    {
            if (classname == "Dialogs::InnerWidget") {
                return new AccessibleInnerWidget(qobject_cast<InnerWidget *>(object));
            }
            return nullptr; });
        return true;
    }();

    AccessibleInnerWidget::AccessibleInnerWidget(InnerWidget *widget)
        : QAccessibleWidget(widget, QAccessible::List) {}

    int AccessibleInnerWidget::childCount() const
    {
        const auto *widget = static_cast<const InnerWidget *>(object());
        if (!widget)
            return 0;

        if (widget->state() == WidgetState::Filtered)
        {
            return widget->hashtagResults().size() + widget->filterResults().size() + widget->peerSearchResults().size() + widget->previewResults().size() + widget->searchResults().size();
        }
        return widget->shownList()->size();
    }

    QAccessibleInterface *AccessibleInnerWidget::child(int index) const
    {
        const auto *widget = static_cast<InnerWidget *>(object());
        if (!widget || index < 0)
            return nullptr;

        const int rowWidth = widget->width();

        if (widget->state() == WidgetState::Filtered)
        {
            int offset = 0;

            const auto &hashtags = widget->hashtagResults();
            if (index < offset + hashtags.size())
            {
                const auto i = index - offset;
                const auto &item = hashtags[i];
                return new AccessibleBasicRow(
                    &item->row,
                    "Hashtag: " + item->tag,
                    rowWidth,
                    st::mentionHeight,
                    widget->hashtagsOffset() + (i * st::mentionHeight),
                    widget);
            }
            offset += hashtags.size();

            const auto &filters = widget->filterResults();
            if (index < offset + filters.size())
            {
                return new AccessibleRow(
                    filters[index - offset].row.get(), rowWidth, widget);
            }
            offset += filters.size();

            const auto &peers = widget->peerSearchResults();
            if (index < offset + peers.size())
            {
                const auto i = index - offset;
                const auto &item = peers[i];
                return new AccessibleBasicRow(
                    &item->row,
                    item->peer->name(),
                    rowWidth,
                    st::dialogsRowHeight,
                    widget->peerSearchOffset() + (i * st::dialogsRowHeight),
                    widget);
            }
            offset += peers.size();

            const auto &previews = widget->previewResults();
            if (index < offset + previews.size())
            {
                return new AccessibleFakeRow(
                    previews[index - offset].get(), rowWidth, widget);
            }
            offset += previews.size();

            const auto &searches = widget->searchResults();
            if (index < offset + searches.size())
            {
                return new AccessibleFakeRow(
                    searches[index - offset].get(), rowWidth, widget);
            }
        }
        else
        { // Default State
            const auto &shown = widget->shownList()->all();
            if (index < shown.size())
            {
                const auto it = std::next(shown.cbegin(), index);
                return new AccessibleRow((*it).get(), rowWidth, widget);
            }
        }
        return nullptr;
    }

    int AccessibleInnerWidget::indexOfChild(const QAccessibleInterface *child) const
    {
        if (!child)
            return -1;
        const auto *widget = static_cast<const InnerWidget *>(object());
        if (!widget)
            return -1;

        if (widget->state() == WidgetState::Filtered)
        {
            int offset = 0;

            if (const auto *const basic = dynamic_cast<const AccessibleBasicRow *>(child))
            {
                const auto &hashtags = widget->hashtagResults();
                for (size_t i = 0; i < hashtags.size(); ++i)
                {
                    if (&hashtags[i]->row == basic->row())
                    {
                        return offset + i;
                    }
                }

                offset += hashtags.size() + widget->filterResults().size();
                const auto &peers = widget->peerSearchResults();
                for (size_t i = 0; i < peers.size(); ++i)
                {
                    if (&peers[i]->row == basic->row())
                    {
                        return offset + i;
                    }
                }
            }

            offset = widget->hashtagResults().size();
            if (const auto *const row = dynamic_cast<const AccessibleRow *>(child))
            {
                const auto &filters = widget->filterResults();
                for (size_t i = 0; i < filters.size(); ++i)
                {
                    if (filters[i].row.get() == row->row())
                    {
                        return offset + i;
                    }
                }
            }

            offset = widget->hashtagResults().size() + widget->filterResults().size() + widget->peerSearchResults().size();
            if (const auto *const fake = dynamic_cast<const AccessibleFakeRow *>(child))
            {
                const auto &previews = widget->previewResults();
                for (size_t i = 0; i < previews.size(); ++i)
                {
                    if (previews[i].get() == fake->row())
                    {
                        return offset + i;
                    }
                }
                offset += previews.size();

                const auto &searches = widget->searchResults();
                for (size_t i = 0; i < searches.size(); ++i)
                {
                    if (searches[i].get() == fake->row())
                    {
                        return offset + i;
                    }
                }
            }
        }
        else
        { // Default state
            if (const auto *const row = dynamic_cast<const AccessibleRow *>(child))
            {
                const auto &shown = widget->shownList()->all();
                for (auto it = shown.cbegin(); it != shown.cend(); ++it)
                {
                    if ((*it).get() == row->row())
                    {
                        return std::distance(shown.cbegin(), it);
                    }
                }
            }
        }
        return -1;
    }

    QAccessibleInterface *AccessibleInnerWidget::childAt(int, int) const
    {
        return nullptr;
    }

    QAccessibleInterface *AccessibleInnerWidget::focusChild() const
    {
        auto *widget = static_cast<InnerWidget *>(object());
        if (!widget)
            return nullptr;

        auto *focused = widget->focusedRow();
        if (!focused)
            return nullptr;

        auto *child = new AccessibleRow(focused, widget->width(), widget);
        std::cout << "focusChild Name: " << child->text(QAccessible::Name).toStdString() << std::endl;
        return child;
    }

    void AccessibleInnerWidget::setText(QAccessible::Text, const QString &)
    {
        // No-op
    }

    QString AccessibleInnerWidget::text(QAccessible::Text t) const
    {
        if (t != QAccessible::Name)
        {
            return QString();
        }

        const auto *widget = static_cast<const InnerWidget *>(object());
        if (!widget)
        {
            return QString();
        }

        if (widget->state() == WidgetState::Filtered)
        {
            return QStringLiteral("Search Results");
        }

        return QStringLiteral("Chat List");
    }

    QAccessible::Role AccessibleInnerWidget::role() const
    {
        return QAccessible::List;
    }

    QAccessible::State AccessibleInnerWidget::state() const
    {
        QAccessible::State s;
        s.focusable = false;
        s.multiSelectable = true;
        return s;
    }

} // namespace Dialogs
