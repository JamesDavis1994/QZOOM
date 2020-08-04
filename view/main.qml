import QtQuick.Window 2.12
import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import "../components" as C

ApplicationWindow {
    id: rootWindow
    visible: true
    width: 1280
    height: 784
    minimumHeight: 600
    minimumWidth: 600
    title: qsTr("qZoom")

    Settings {
        id: settings
    }

    Dialog {
        id: errorMessage
        visible: false
        width: 400
        height: 300
        //modal: true
        title: "Error"
        //anchors.centerIn: parent
        standardButtons: Dialog.Close | Dialog.Help

        property alias text : errorText.text

        Rectangle{
            anchors.centerIn: parent
            Text {
                id: errorText
                anchors.centerIn: parent
            }
        }


        onRejected: console.log("Close clicked")
        onHelp: {
            console.log("Help clicked, opening browser..");
            Qt.openUrlExternally("https://github.com/Feqzz/qZoom-Client/wiki");

        }
        Connections {
            target: errorHandler
            function onShowError(error) {
                showErrorMessage(error);
            }
        }
    }



    StackView {
        id: stackView
        focus: true
        anchors.fill: parent

        replaceEnter: Transition {

            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 0
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                duration: 0
                from: 1
                to: 0

            }
        }
    }

    function showErrorMessage(errorString){
        errorMessage.text = errorString
        errorMessage.visible = true;
    }

    // After loading show initial Login Page
    Component.onCompleted: {
        stackView.push("qrc:/view/host.qml");

    }

    function pushPage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.push(url);
    }

    function popToPage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.pop(url);
    }

    function changePage(page) {
        var template = "qrc:/view/%1.qml";
        var url = template.arg(page);

        stackView.replace(url);
    }

    function setTitle(title) {
        rootWindow.title = title;
    }

    function showSettings() {
        settings.loadSettings();
        console.log("Loaded Settings");
        settings.open();
    }
}
