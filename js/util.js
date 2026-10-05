.pragma library

// FORK-ONLY: turn the rich text a timeline event arrives as back into the
// plain text it will actually render. Used to measure a message with
// TextMetrics: Qt 5.6's TextMetrics has no textFormat property, so it can
// only measure plain text, and measuring the markup would give the width of
// the tags rather than of the words.
function stripMarkup(html) {
    if(html === null || html === undefined)
        return ""

    return String(html)
        .replace(/<[^>]*>/g, "")
        .replace(/&nbsp;/g, " ")
        .replace(/&lt;/g, "<")
        .replace(/&gt;/g, ">")
        .replace(/&quot;/g, "\"")
        .replace(/&#39;/g, "'")
        .replace(/&amp;/g, "&")
}

// FORK-ONLY: true for the state events a room is created with, which are
// administrative and carry nothing a reader wants in a timeline.
//
// Synapse creates a room with m.room.create, m.room.power_levels,
// m.room.join_rules, m.room.history_visibility and m.room.guest_access.
// libQMatrixClient 2019 has no typed class for any of them, so
// utils::eventToString fell through to a literal "Unknown Event" and every new
// room opened with five grey bubbles saying that. RoomPanelForm collapses the
// rows that match; this list is what it matches on, and utils.h gained text for
// them as a safety net.
//
// NB: `m.room.member` is deliberately absent - joins and leaves are worth
// seeing, and libQMatrixClient does model that one.
function administrativeState(matrixType) {
    switch (matrixType) {
    case "m.room.create":
    case "m.room.power_levels":
    case "m.room.join_rules":
    case "m.room.history_visibility":
    case "m.room.guest_access":
    case "m.room.server_acl":
        return true
    default:
        return false
    }
}

function pushToStack(stack, page) {
    if(page && stack.currentItem !== page) {
        // FORK-ONLY: push (not replace) so the stack Back button can
        // pop back to the previous page. Same-object repeats are harmless:
        // PScreenStack toggles visibility and pop() walks back.
        stack.push(page)
    }
}
