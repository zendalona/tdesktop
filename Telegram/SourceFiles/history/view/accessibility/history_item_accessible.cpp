/*
This file is part of Telegram Desktop.
*/
#include "history/view/accessibility/history_item_accessible.h"

#include "history/history_inner_widget.h"
#include "history/view/history_view_element.h"
#include "history/history_item.h"
#include "history/history_item_helpers.h"
#include "history/history_item_components.h"
#include "data/data_user.h"
#include "data/data_media_types.h"
#include "data/data_document.h"

#include "history/history_item_text.h"      
#include "ui/text/format_values.h"  

#include <QtGui/QScreen>
#include <QLocale>
#include <algorithm>
#include <iostream>
#include <QDebug>

namespace HistoryView::Accessibility {

ItemAccessible::ItemAccessible(
    not_null<Element*> element,
    not_null<const HistoryInner*> parentWidget) // <-- Added const
: _element(element)
, _parentWidget(parentWidget) {
}

bool ItemAccessible::isValid() const {
    return (_parentWidget->viewByItem(_element->data()) == _element);
}

QAccessibleInterface *ItemAccessible::parent() const {
    // We need to cast away const here because queryAccessibleInterface
    // expects a non-const QObject*, even though it doesn't modify it.
    return QAccessible::queryAccessibleInterface(const_cast<HistoryInner*>(_parentWidget.get()));
}

int ItemAccessible::childCount() const {
    return 0;
}

QAccessibleInterface *ItemAccessible::child(int index) const {
    return nullptr;
}

int ItemAccessible::indexOfChild(const QAccessibleInterface *child) const {
    return -1;
}

QObject *ItemAccessible::object() const {
    return nullptr;
}

QAccessible::Role ItemAccessible::role() const {
    return QAccessible::ListItem;
}

QAccessible::State ItemAccessible::state() const {
    QAccessible::State s;
    s.focusable = true;
    s.selectable = true;
    if (_parentWidget->hasSelectedItems()) {
        const auto selected = _parentWidget->getSelectedItems();
        const auto id = _element->data()->fullId();
        if (std::find(selected.begin(), selected.end(), id) != selected.end()) {
            s.selected = true;
        }
    }
    return s;
}

QString ItemAccessible::text(QAccessible::Text t) const {
    if (t != QAccessible::Name) {
        return QString();
    }

    const auto item = _element->data();
    QStringList parts;

    // 1. If this is the first message of a new day, add the date first.
    if (_element->displayDate()) {
        const auto date = ItemDateTime(item).date();
        parts.append(QLocale().toString(date, QLocale::LongFormat));
    }

    // 2. Now handle the specific message type.
    if (item->isService()) {
        // This is the corrected line for service messages.
        parts.append(_element->text().toString());
    } else {
        // For regular messages, add sender, content, and time.
        if (const auto from = item->displayFrom()) {
            parts.append(from->name());
        }

        const auto &original = item->originalText();
        if (!original.text.isEmpty()) {
            parts.append(original.text);
        } else if (const auto media = item->media()) {
            if (media->document() && media->document()->isVoiceMessage()) {
                const auto duration = media->document()->duration() / 1000.;
                const auto durationText = Ui::FormatDurationText(duration);
                parts.append(QStringLiteral("Voice Message, duration %1. Press Space to play.").arg(durationText));
            } else if (media->photo()) {
                parts.append(QStringLiteral("Photo"));
            } else if (media->document() && media->document()->sticker()) {
                parts.append(QStringLiteral("Sticker"));
            } else {
                parts.append(QStringLiteral("File Attachment"));
            }
        }
        parts.append(ItemDateTime(item).time().toString("h:mm AP"));
    }

    const auto result = parts.join(u", ");
    qDebug() << "Generated Accessible Text:" << result;
    return result;
}

QRect ItemAccessible::rect() const {
    const auto top = _parentWidget->itemTop(_element);
    if (top < 0) {
        return QRect();
    }
    const auto local = QRect(0, top, _parentWidget->width(), _element->height());
    const auto globalTopLeft = _parentWidget->mapToGlobal(local.topLeft());

    const auto screen = _parentWidget->screen()->geometry();
    return QRect(globalTopLeft, local.size()).intersected(screen);
}

void *ItemAccessible::interface_cast(QAccessible::InterfaceType t) {
    return nullptr;
}

QAccessibleInterface *ItemAccessible::childAt(int x, int y) const {
    return nullptr;
}

void ItemAccessible::setText(QAccessible::Text t, const QString &text) {
}

} // namespace HistoryView::Accessibility