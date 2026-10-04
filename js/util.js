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

function pushToStack(stack, page) {
    if(page && stack.currentItem !== page) {
        // FORK-ONLY: push (not replace) so the stack Back button can
        // pop back to the previous page. Same-object repeats are harmless:
        // PScreenStack toggles visibility and pop() walks back.
        stack.push(page)
    }
}
