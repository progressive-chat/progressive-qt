import QtQuick 2.6

LoginForm {
    loginButton.onClicked: doLogin()

    Component.onCompleted: {
        serverField.accepted.connect(doLogin)
        usernameField.accepted.connect(doLogin)
        passwordField.accepted.connect(doLogin)
        controller.connectionAdded.connect(function(conn) {
            stackView.pop()
            accountListView.currentConnection = conn
        })
        // FORK-ONLY: re-enable the button when login fails, otherwise it
        // stays stuck on "Logging in..." forever.
        controller.errorOccured.connect(resetLoginButton)
    }

    function resetLoginButton() {
        loginButton.text = "LOGIN"
        loginButton.enabled = true
    }

    function doLogin() {
        loginError.visible = false
        // NOTE: Qt 5.6 V4 has no String.startsWith/includes — use indexOf.
        if (!(serverField.text.indexOf("http") === 0 && serverField.text.indexOf("://") !== -1)) {
            loginError.text = "Server address should start with http(s)://"
            loginError.visible = true
            return
        }

        loginButton.text = "Logging in..."
        loginButton.enabled = false
        controller.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)
    }
}
