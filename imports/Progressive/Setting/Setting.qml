pragma Singleton
// SPDX-License-Identifier: GPL-3.0-only
// Progressive Chat settings — Qt 5.6 compatible.
//
// Upstream used `Qt.labs.settings 1.0` (Qt >= 5.8 only). This port stores
// the same four keys in QtQuick.LocalStorage so the client runs on
// Qt 5.6 / Android 4.0 down to 2.3 without the labs module.
import QtQuick 2.6
import QtQuick.LocalStorage 2.0

Item {
    property bool pressAndHold: false
    property bool showTray: true
    property bool confirmOnExit: true
    property bool darkTheme: false
    property bool richText: true

    property bool _ready: false

    function _db() {
        return LocalStorage.openDatabaseSync("progressive-chat", "1.0",
                                             "Progressive Chat settings", 65536);
    }

    function load() {
        var db = _db();
        db.transaction(function(tx) {
            tx.executeSql("CREATE TABLE IF NOT EXISTS settings(k TEXT UNIQUE, v TEXT)");
            var rs = tx.executeSql("SELECT k, v FROM settings");
            for (var i = 0; i < rs.rows.length; i++) {
                var k = rs.rows.item(i).k, v = rs.rows.item(i).v;
                if (k === "pressAndHold") pressAndHold = v === "1";
                else if (k === "showTray") showTray = v === "1";
                else if (k === "confirmOnExit") confirmOnExit = v === "1";
                else if (k === "darkTheme") darkTheme = v === "1";
                else if (k === "richText") richText = v === "1";
            }
        });
    }

    function save() {
        if (!_ready) return;
        var db = _db();
        db.transaction(function(tx) {
            tx.executeSql("INSERT OR REPLACE INTO settings(k, v) VALUES(?, ?)", ["pressAndHold", pressAndHold ? "1" : "0"]);
            tx.executeSql("INSERT OR REPLACE INTO settings(k, v) VALUES(?, ?)", ["showTray", showTray ? "1" : "0"]);
            tx.executeSql("INSERT OR REPLACE INTO settings(k, v) VALUES(?, ?)", ["confirmOnExit", confirmOnExit ? "1" : "0"]);
            tx.executeSql("INSERT OR REPLACE INTO settings(k, v) VALUES(?, ?)", ["darkTheme", darkTheme ? "1" : "0"]);
            tx.executeSql("INSERT OR REPLACE INTO settings(k, v) VALUES(?, ?)", ["richText", richText ? "1" : "0"]);
        });
    }

    onPressAndHoldChanged: save()
    onShowTrayChanged: save()
    onConfirmOnExitChanged: save()
    onDarkThemeChanged: save()
    onRichTextChanged: save()

    Component.onCompleted: { load(); _ready = true; }
}
