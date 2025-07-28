// dialogs_accessible_row.cpp
#include "dialogs_accessible_row.h"
#include "dialogs_row.h"
#include "dialogs_entry.h"
#include "dialogs_list.h"
#include "history/history.h"
#include "history/history_item.h"
#include "data/data_user.h"

namespace Dialogs {

    AccessibleRow::AccessibleRow(Row *row, int rowWidth, InnerWidget *parentWidget)
    : _row(row), _width(rowWidth), _parentWidget(parentWidget) {}


// QAccessibleInterface
QObject *AccessibleRow::object() const { return nullptr; }
bool AccessibleRow::isValid() const { return _row != nullptr; }
QAccessibleInterface *AccessibleRow::child(int) const { return nullptr; }
int AccessibleRow::childCount() const { return 0; }
int AccessibleRow::indexOfChild(const QAccessibleInterface *) const { return -1; }
QAccessibleInterface *AccessibleRow::parent() const {
    return QAccessible::queryAccessibleInterface(_parentWidget);
}

QRect AccessibleRow::rect() const {
    if (!_row || !_parentWidget) {
        return {};
    }

    const int y = _row->top();

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

	const auto entry = _row->entry();
	const auto name = entry->chatListName();
	const auto item = entry->chatListMessage();
	QString result;

	// 1. Type (Chat type: Bot, Group, Channel, etc.)
	if (const auto history = entry->asHistory()) {
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

	// 2. Name (Contact or group name)
	result += name;

	// 3. Unread Message Count
	if (const auto history = entry->asHistory()) {
		const int unread = history->unreadCount();
		if (unread > 0) {
			result += QString(". You have %1 unread message%2")
				.arg(unread)
				.arg(unread > 1 ? "s" : "");
		}
	}

	// 4. Muted or not
	if (const auto history = entry->asHistory()) {
		if (history->muted()) {
			result += ". Muted.";
		}
	}

	// 5. Sender name and Message Details
	if (item) {
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
		QDateTime dt = QDateTime::fromSecsSinceEpoch(timestamp);
		QDateTime now = QDateTime::currentDateTime();

		const bool isToday = dt.date() == now.date();
		const QString timeStr = dt.time().toString("h:mm AP");
		const int day = dt.date().day();
		const QString daySuffix = (day == 1 || day == 21 || day == 31) ? "st"
			: (day == 2 || day == 22) ? "nd"
			: (day == 3 || day == 23) ? "rd"
			: "th";
		const QString dateStr = QString::number(day) + daySuffix;
		const QString monthStr = dt.date().toString("MMMM");

		if (isToday) {
			result += ". Received today at " + timeStr;
		} else {
			result += ". Received at "
				+ timeStr + ", "
				+ dateStr + " of "
				+ monthStr;
		}
	}

	return result;
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
    return QRect(); // Optional: return bounding rect per character
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

} // namespace Dialogs
