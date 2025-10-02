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
#include "data/data_audio_msg_id.h"               // <-- NEEDED for AudioMsgId
#include "media/player/media_player_instance.h"   // <-- NEEDED for ::instance()
#include "media/audio/media_audio.h"// <-- NEEDED for TrackState
#include "history/history.h"

#include <QtGui/QScreen>
#include <QLocale>
#include <algorithm>


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
    const auto item = _element->data();
    if (const auto media = item->media()) {
        if (media->document() && media->document()->isVoiceMessage()) {
            return 1; // Voice notes have one child: the play button.
        }
    }
    return 0;
}

QAccessibleInterface *ItemAccessible::child(int index) const {
    if (index == 0 && childCount() == 1) {
        return new VoiceNoteButtonAccessible(_element, const_cast<ItemAccessible*>(this));
    }
    return nullptr;
}

int ItemAccessible::indexOfChild(const QAccessibleInterface *child) const {
    if (!child || childCount() == 0) {
        return -1;
    }

    // Since a voice note only has one possible child (the button at index 0),
    // we just need to check if the given child's parent is this item.
    if (child->parent() == this) {
        return 0;
    }

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

    if (_element->displayDate()) {
        parts.append(QLocale().toString(ItemDateTime(item).date(), QLocale::LongFormat));
    }

    if (item->isService()) {
        parts.append(_element->text().toString());
    } else {
        if (item->out()) {
            parts.append(QStringLiteral("You"));
        } else if (const auto from = item->displayFrom()) {
            parts.append(from->name());
        }

        
        const auto media = item->media();
        const auto &text = item->originalText();
        const auto hasMedia = (media != nullptr);
        const auto hasText = !text.text.isEmpty();

        QString mediaDescription;
        if (hasMedia) {
            if (const auto document = media->document(); document && document->isVoiceMessage()) {
                const auto mediaId = AudioMsgId(document, item->fullId());
                const auto isPlaying = (::Media::Player::instance()->getState(mediaId.type()).state == ::Media::Player::State::Playing);
                const auto durationText = Ui::FormatDurationText(document->duration() / 1000.);
                
                parts.append(QStringLiteral("Voice Message, duration %1").arg(durationText));
            } else if (media->photo()) {
                mediaDescription = QStringLiteral("Photo");
            } else if (document && document->isVideoFile()) { 
                mediaDescription = QStringLiteral("Video");
            } else if (media->document() && media->document()->sticker()) {
                mediaDescription = QStringLiteral("Sticker");
            } else {
                mediaDescription = QStringLiteral("File Attachment");
            }
        }

        if (hasMedia && hasText) {
            parts.append(mediaDescription + QStringLiteral(" with caption: ") + text.text);
        } else if (hasMedia) {
            parts.append(mediaDescription);
        } else if (hasText) {
            parts.append(text.text);
        }
        

        parts.append(ItemDateTime(item).time().toString("h:mm AP"));
    }

    if (_parentWidget->hasSelectedItems()) {
        const auto &selected = _parentWidget->getSelectedItems();
        if (std::find(selected.begin(), selected.end(), item->fullId()) != selected.end()) {
            parts.append(QStringLiteral("Selected"));
        }
    }

    if (!item->isService() && item->out()) {
        if (item->history()->outboxReadTillId() >= item->id) {
            parts.append(QStringLiteral("Seen"));
        } else {
            parts.append(QStringLiteral("Not seen"));
        }
    }

    return parts.join(u", ");
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
ItemAccessible::VoiceNoteButtonAccessible::VoiceNoteButtonAccessible(
    not_null<Element*> element,
    not_null<ItemAccessible*> parent)
: _element(element)
, _parent(parent) {
}

bool ItemAccessible::VoiceNoteButtonAccessible::isValid() const { return _parent->isValid(); }
QAccessibleInterface *ItemAccessible::VoiceNoteButtonAccessible::parent() const { return _parent; }
int ItemAccessible::VoiceNoteButtonAccessible::childCount() const { return 0; }
QAccessibleInterface *ItemAccessible::VoiceNoteButtonAccessible::child(int) const { return nullptr; }
int ItemAccessible::VoiceNoteButtonAccessible::indexOfChild(const QAccessibleInterface *) const { return -1; }
QObject *ItemAccessible::VoiceNoteButtonAccessible::object() const { return nullptr; }

QAccessible::Role ItemAccessible::VoiceNoteButtonAccessible::role() const {
    return QAccessible::Button;
}

QAccessible::State ItemAccessible::VoiceNoteButtonAccessible::state() const {
    QAccessible::State s;
    s.focusable = true;
    return s;
}

QString ItemAccessible::VoiceNoteButtonAccessible::text(QAccessible::Text t) const {
    if (t != QAccessible::Name || !_element) {
        return QString();
    }
    const auto item = _element->data();
    if (const auto media = item->media()) {
        if (const auto document = media->document(); document && document->isVoiceMessage()) {
            const auto mediaId = AudioMsgId(document, item->fullId());
            const auto isPlaying = (::Media::Player::instance()->getState(mediaId.type()).state == ::Media::Player::State::Playing);
            return isPlaying ? QStringLiteral("Pause voice") : QStringLiteral("Play voice ");
        }
    }
    return QStringLiteral("Play"); // Fallback
}
// need to fix Qrec
QRect ItemAccessible::VoiceNoteButtonAccessible::rect() const {
    const auto parentRect = _parent->rect();
    if (parentRect.isEmpty()) { return QRect(); }
    return QRect(parentRect.topLeft(), QSize(parentRect.height(), parentRect.height()));
}

void *ItemAccessible::VoiceNoteButtonAccessible::interface_cast(QAccessible::InterfaceType) { return nullptr; }
QAccessibleInterface *ItemAccessible::VoiceNoteButtonAccessible::childAt(int, int) const { return nullptr; }
void ItemAccessible::VoiceNoteButtonAccessible::setText(QAccessible::Text, const QString &) {}

} // namespace HistoryView::Accessibility