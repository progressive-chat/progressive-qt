.pragma library

function pushToStack(stack, page) {
    if(page && stack.currentItem !== page) {
        // FORK-ONLY: push (not replace) so the stack Back button can
        // pop back to the previous page. Same-object repeats are harmless:
        // PScreenStack toggles visibility and pop() walks back.
        stack.push(page)
    }
}
