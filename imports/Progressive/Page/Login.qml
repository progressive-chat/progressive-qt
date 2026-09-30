import QtQuick 2.6

LoginForm {
    loginButton.onClicked: doLogin()

    Component.onCompleted: {
        serverField.accepted.connect(doLogin)
        usernameField.accepted.connect(doLogin)
        passwordField.accepted.connect(doLogin)
    }

    function doLogin() {
        loginError.visible = false
        if (!(serverField.text.startsWith("http") && serverField.text.includes("://"))) {
            loginError.text = "Server address should start with http(s)://"
            loginError.visible = true
            return
        }

        loginButton.text = "Logging in..."
        loginButton.enabled = false
        controller.loginWithCredentials(serverField.text, usernameField.text, passwordField.text)

        controller.connectionAdded.connect(function(conn) {
            stackView.pop()
            accountListView.currentConnection = conn
        })
    }
}
