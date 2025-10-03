// dialogs_accessible_row.cpp
#include "dialogs_accessible_row.h"
#include "dialogs_row.h"
#include "dialogs_entry.h"
#include "dialogs_list.h"
#include "history/history.h"
#include "history/history_item.h"
#include "data/data_user.h"
#include "dialogs_indexed_list.h"
#include "styles/style_dialogs.h"
#include "data/data_folder.h"

namespace Dialogs {

    QString GenerateAccessibleDescription(
        not_null<Dialogs::Entry*> entry,
        HistoryItem *item) {
    // Call expensive or repeated methods once at the top for efficiency.
    const auto history = entry->asHistory();
    const auto name = entry->chatListName();

    QString result;

    // 1. Type (Chat type: Bot, Group, Channel, etc.)
    if (history) {
        if (const auto peer = history->peer) {
            if (peer->isUser()) {
                const auto user = peer->asUser();
                if (user && user->isBot()) {
                    result += "Bot. ";
                }
            } else if (peer->isMegagroup()) {
                result += "Group. ";
            } else if (peer->isBroadcast()) {
                result += "Channel. ";
            }
        }
    }

    // 2. Name (Contact or group name) - Always add the name.
    result += name;

    // Add details that require a valid history object.
    if (history) {
        // 3. Unread Message Count
        const int unread = history->unreadCount();
        if (unread > 0) {
            result += QString(". You have %1 unread message%2")
                .arg(unread)
                .arg(unread > 1 ? "s" : "");
        }

        // 4. Muted or not
        if (history->muted()) {
            result += ". Muted.";
        }
    }

    // Add details from the last message item, if it exists.
    if (item) {
        // 5. Sender name
        const auto from = item->from();
        const auto fromName = (from && !from->name().isEmpty())
            ? from->name()
            : QString();
        const bool isFromMyself = from && from->isSelf();
        const bool isSameAsChat = (fromName == name);

        if (!isFromMyself && !isSameAsChat && !fromName.isEmpty()) {
            result += ". Message from " + fromName;
        }

        // 6. Last Message
        const auto message = item->originalText().text;
        if (!message.isEmpty()) {
            result += ". " + message;
        }

        // 7. Received at Time (with formatting)
        const auto timestamp = item->date();
        const QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
        const QDateTime now = QDateTime::currentDateTime();

        const bool isToday = dt.date() == now.date();
        const QString timeStr = dt.time().toString("h:mm AP");

        if (isToday) {
            result += ". Received today at " + timeStr;
        } else {
            const int day = dt.date().day();
            const QString daySuffix = (day == 1 || day == 21 || day == 31) ? "st"
                : (day == 2 || day == 22) ? "nd"
                : (day == 3 || day == 23) ? "rd"
                : "th";
            const QString dateStr = QString::number(day) + daySuffix;
            const QString monthStr = dt.date().toString("MMMM");
            result += ". Received at "
                + timeStr + ", "
                + dateStr + " of "
                + monthStr;
        }
    }

    return result;
}

    AccessibleRow::AccessibleRow(Row *row, int rowWidth, const InnerWidget *parentWidget)
    : _row(row), _width(rowWidth), _parentWidget(parentWidget) {}


// QAccessibleInterface
QObject *AccessibleRow::object() const { return nullptr; }
bool AccessibleRow::isValid() const { return _row != nullptr; }
QAccessibleInterface *AccessibleRow::child(int) const { return nullptr; }
int AccessibleRow::childCount() const { return 0; }
int AccessibleRow::indexOfChild(const QAccessibleInterface *) const { return -1; }
QAccessibleInterface *AccessibleRow::parent() const {
    return QAccessible::queryAccessibleInterface(const_cast<InnerWidget*>(_parentWidget));
}

QRect AccessibleRow::rect() const {
    if (!_row || !_parentWidget) {
        return {};
    }

    const auto *inner = qobject_cast<const InnerWidget*>(_parentWidget);
    if (!inner) {
        return {};
    }

    int y = 0;

    // First, handle the special case for the collapsed archive row.
    if (!inner->collapsedRows().empty()
        && _row->folder()
        && _row->folder()->id() == Data::Folder::kId) {
        const QPoint globalTopLeft = _parentWidget->mapToGlobal(QPoint(0, 0));
        const QSize size(_width, st::dialogsImportantBarHeight);
        return QRect(globalTopLeft, size);
    }

    // For all other rows, figure out their position based on the state.
    if (inner->state() == WidgetState::Filtered) {
        const auto &results = inner->filterResults();
        const auto it = std::find_if(
            results.begin(),
            results.end(),
            [=](const auto &result) { return (result.row.get() == _row); });

        if (it != results.end()) {
            const int relativeTop = it->top;
            y = inner->filteredOffset() + relativeTop;
        } else {
            return {}; 
        }
    } else { // WidgetState::Default
        y = inner->dialogsOffset() + _row->top();
    }

    const QPoint globalTopLeft = _parentWidget->mapToGlobal(QPoint(0, y));
    const QSize size(_width, _row->height());
    return QRect(globalTopLeft, size);
}
QAccessible::Role AccessibleRow::role() const { return QAccessible::ListItem; }

QAccessible::State AccessibleRow::state() const {
    QAccessible::State s;
    s.focusable = true;
    s.selectable = true;
    return s;
}

QString AccessibleRow::text(QAccessible::Text t) const {
    if (t != QAccessible::Name || !_row || !_row->entry()) {
        return QString();
    }

    if (_parentWidget->state() == WidgetState::Filtered) {
        const QString baseName = _row->entry()->chatListName();
        const auto *inner = qobject_cast<const InnerWidget*>(_parentWidget);
        if (inner) {
            const auto rows = inner->accessibleRows();
            const auto it = std::find(rows.begin(), rows.end(), _row);

            if (it != rows.end()) {
                const int index = std::distance(rows.begin(), it);
                const int total = rows.size();
                return QString("Search Result %1 of %2: %3")
                    .arg(index + 1)
                    .arg(total)
                    .arg(baseName);
            }
        }
        return "[Search Result] " + baseName;
    }

    return GenerateAccessibleDescription(
        _row->entry(),
        _row->entry()->chatListMessage());
}



QAccessibleInterface *AccessibleRow::childAt(int, int) const { return nullptr; }

void AccessibleRow::setText(QAccessible::Text, const QString &) {}

void *AccessibleRow::interface_cast(QAccessible::InterfaceType t) {
    if (t == QAccessible::TextInterface)
        return static_cast<QAccessibleTextInterface *>(this);
    return nullptr;
}

// QAccessibleTextInterface
QString AccessibleRow::text(int startOffset, int endOffset) const {
    const QString text = _accessibleText();
    if (startOffset < 0 || endOffset > text.size() || startOffset >= endOffset) {
        return QString();
    }
    return text.mid(startOffset, endOffset - startOffset);
}

int AccessibleRow::characterCount() const {
    return _accessibleText().size();
}

QRect AccessibleRow::characterRect(int) const {
    return QRect(); 
}

int AccessibleRow::offsetAtPoint(const QPoint &) const {
    return 0;
}

void AccessibleRow::selection(int, int *start, int *end) const {
    *start = *end = 0;
}

int AccessibleRow::selectionCount() const {
    return 0;
}

void AccessibleRow::addSelection(int, int) {}
void AccessibleRow::removeSelection(int) {}
void AccessibleRow::setSelection(int, int, int) {}

void AccessibleRow::scrollToSubstring(int, int) {}

void AccessibleRow::setCursorPosition(int) {}
int AccessibleRow::cursorPosition() const {
    return 0;
}

QString AccessibleRow::attributes(int, int *startOffset, int *endOffset) const {
    *startOffset = 0;
    *endOffset = characterCount();
    return "readonly:true";
}

Row *AccessibleRow::row() const {
    return _row;
}

QString AccessibleRow::_accessibleText() const {
    if (!_row || !_row->entry()) return {};
    return _row->entry()->chatListName();
    
}

AccessibleRow::~AccessibleRow() = default;

AccessibleFakeRow::AccessibleFakeRow(FakeRow *row, int rowWidth, const InnerWidget *parentWidget)
    : _row(row), _parentWidget(parentWidget), _width(rowWidth) {}


QObject *AccessibleFakeRow::object() const { return nullptr; }
bool AccessibleFakeRow::isValid() const { return _row != nullptr; }
QAccessibleInterface *AccessibleFakeRow::child(int) const { return nullptr; }
int AccessibleFakeRow::childCount() const { return 0; }
int AccessibleFakeRow::indexOfChild(const QAccessibleInterface *) const { return -1; }

QAccessibleInterface *AccessibleFakeRow::parent() const {
    return QAccessible::queryAccessibleInterface(const_cast<InnerWidget*>(_parentWidget));
}

QRect AccessibleFakeRow::rect() const {
    if (!_row || !_parentWidget) {
        return {};
    }

    const auto *inner = qobject_cast<const InnerWidget*>(_parentWidget);
    if (!inner) {
        return {};
    }

    int y = -1;

    // First, search for the row in the preview results list.
    const auto &previews = inner->previewResults();
    const auto previewIt = std::find_if(previews.begin(), previews.end(),
        [=](const auto &p) { return p.get() == _row; });

    if (previewIt != previews.end()) {
        const int index = std::distance(previews.begin(), previewIt);
        y = inner->previewOffset() + (index * inner->st()->height);
    } else {
        // If not found, search in the main search results list.
        const auto &searches = inner->searchResults();
        const auto searchIt = std::find_if(searches.begin(), searches.end(),
            [=](const auto &p) { return p.get() == _row; });

        if (searchIt != searches.end()) {
            const int index = std::distance(searches.begin(), searchIt);
            y = inner->searchedOffset() + (index * inner->st()->height);
        }
    }

    // If we found the row and calculated its position, create the rectangle.
    if (y != -1) {
        const QPoint globalTopLeft = _parentWidget->mapToGlobal(QPoint(0, y));
        const QSize size(_width, inner->st()->height);
        return QRect(globalTopLeft, size);
    }

    return {};
}
QAccessible::Role AccessibleFakeRow::role() const {
    return QAccessible::ListItem;
}

QAccessible::State AccessibleFakeRow::state() const {
    QAccessible::State result;
    result.selectable = true;
    result.focusable = true;
    return result;
}

QString AccessibleFakeRow::text(QAccessible::Text t) const {
    if (t != QAccessible::Name || !_row || !_parentWidget) {
        return QString();
    }

    const auto *inner = qobject_cast<const InnerWidget*>(_parentWidget);
    if (!inner) {
        return QString(); 
    }

    const QString details = GenerateAccessibleDescription(
        _row->item()->history(),
        _row->item());

    const auto &previews = inner->previewResults();
    const auto previewIt = std::find_if(previews.begin(), previews.end(),
        [=](const auto &p) { return p.get() == _row; });

    if (previewIt != previews.end()) {
        const int index = std::distance(previews.begin(), previewIt);
        const int total = previews.size();
        return QString("Public Post %1 of %2: %3")
            .arg(index + 1)
            .arg(total)
            .arg(details);
    }

    const auto &searches = inner->searchResults();
    const auto searchIt = std::find_if(searches.begin(), searches.end(),
        [=](const auto &p) { return p.get() == _row; });

    if (searchIt != searches.end()) {
        const int index = std::distance(searches.begin(), searchIt);
        const int total = searches.size();
        return QString("Message Search result  %1 of %2: %3")
            .arg(index + 1)
            .arg(total)
            .arg(details);
    }

    return details; // Fallback
}
QAccessibleInterface *AccessibleFakeRow::childAt(int, int) const {
    return nullptr;
}

void AccessibleFakeRow::setText(QAccessible::Text, const QString &) {}

void *AccessibleFakeRow::interface_cast(QAccessible::InterfaceType) {
    return nullptr;
}
AccessibleFakeRow::~AccessibleFakeRow() = default;


AccessibleBasicRow::AccessibleBasicRow(
    BasicRow *row,
    QString name,
    int rowWidth,
    int rowHeight,
    int top,
    const InnerWidget *parentWidget)
: _row(row)
, _name(std::move(name))
, _parentWidget(parentWidget)
, _width(rowWidth)
, _height(rowHeight)
, _top(top) {
}

AccessibleBasicRow::~AccessibleBasicRow() = default;

bool AccessibleBasicRow::isValid() const { return _row != nullptr; }

QRect AccessibleBasicRow::rect() const {
    if (!_row || !_parentWidget) {
        return {};
    }
    const QPoint globalTopLeft = _parentWidget->mapToGlobal(QPoint(0, _top));
    return QRect(globalTopLeft, QSize(_width, _height));
}

QString AccessibleBasicRow::text(QAccessible::Text t) const {
    if (t != QAccessible::Name || !_row || !_parentWidget) {
        return QString();
    }

    const auto *inner = qobject_cast<const InnerWidget*>(_parentWidget);
    if (!inner) {
        return _name; 
    }

    const auto &hashtags = inner->hashtagResults();
    const auto hashtagIt = std::find_if(
        hashtags.begin(),
        hashtags.end(),
        [=](const auto &p) { return &p->row == _row; });

    if (hashtagIt != hashtags.end()) {
        return _name;
    }

    const auto &peers = inner->peerSearchResults();
    const auto peerIt = std::find_if(
        peers.begin(),
        peers.end(),
        [=](const auto &p) { return &p->row == _row; });

    if (peerIt != peers.end()) {
        const int index = std::distance(peers.begin(), peerIt);
        const int total = peers.size();
        return QString("Global Search Result %1 of %2: %3")
            .arg(index + 1)
            .arg(total)
            .arg(_name);
    }

    return _name; // Fallback
}

QAccessible::Role AccessibleBasicRow::role() const {
    return QAccessible::ListItem;
}

QAccessible::State AccessibleBasicRow::state() const {
    QAccessible::State result;
    result.selectable = true;
    result.focusable = true;
    return result;
}

// Boilerplate implementations
QObject *AccessibleBasicRow::object() const { return nullptr; }
QAccessibleInterface *AccessibleBasicRow::child(int) const { return nullptr; }
int AccessibleBasicRow::childCount() const { return 0; }
int AccessibleBasicRow::indexOfChild(const QAccessibleInterface *) const { return -1; }
QAccessibleInterface *AccessibleBasicRow::parent() const {
    return QAccessible::queryAccessibleInterface(
        const_cast<InnerWidget*>(_parentWidget));
}
QAccessibleInterface *AccessibleBasicRow::childAt(int, int) const { return nullptr; }
void AccessibleBasicRow::setText(QAccessible::Text, const QString &) {}
void *AccessibleBasicRow::interface_cast(QAccessible::InterfaceType) { return nullptr; }

} // namespace Dialogs