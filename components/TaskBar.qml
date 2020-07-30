import QtQuick 2.0
import QtQuick.Controls 1.4
import "../components" as C

Rectangle {

    color: "#141414"
    height: 64
    width: parent.width

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 32

        /*
        C.PushButton {
            id: changeAudioInputButton
            text: "Change Audio"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                cb.visible = !cb.visible
            }

            ComboBox {
                id: cb
                width: 200
                visible: false
                model: ListModel {
                    id: audioInputModel
                    function getAudioTypes() {
                        var inputList = streamHandler.getAudioInputDevices()
                        clear()
                        for (var title in inputList) {
                            append({text: inputList[title]})
                        }
                    }
                }
                onActivated: {
                    console.log("switched Audio Input Device to: " + currentText)
                    streamHandler.changeAudioInputDevice(currentText);
                    cb.visible = false
                }
            }
        }
        */

        C.PushButton {
            id: roomInfoButton
            text: "Room Info"
            font.pixelSize: 32
            onClicked: roomInfo.open()
        }

        C.PushButton {
            id: muteAudioButton
            text: "Mute Audio"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled) {
                    state = "red"
                    sessionHandler.disableAudio()
                } else {
                    state = "normal"
                    sessionHandler.enableAudio()
                }
                toggled = !toggled
            }
        }

        C.PushButton {
            id: muteVideoButton
            text: "Mute Video"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                if (toggled) {
                    state = "red"
                    sessionHandler.disableVideo()
                } else {
                    state = "normal"
                    sessionHandler.enableVideo()
                }
                toggled = !toggled
            }
        }

        /* C.PushButton {
            id: addScreenButton
            text: "Add Screens"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                repeaterId.model = 0;
                gridId.columns = gridId.calcColumns();
                gridId.rows = gridId.calcRows();
                repeaterId.model = imageHandler.getNumberOfScreens();
            }
        }*/


        //TODO maybe this function does not need to be here?
        //Creates a Signal connection to imageHandler,
        //when the signal addScreen is emitted, this function will run
        Connections {
            target: imageHandler
            function onAddScreen() {
                repeaterId.model = 0;
                gridId.columns = gridId.calcColumns();
                gridId.rows = gridId.calcRows();
                repeaterId.model = imageHandler.getNumberOfScreens();
            }
            function onRemoveScreen() {
                repeaterId.model = 0;
                gridId.columns = gridId.calcColumns();
                gridId.rows = gridId.calcRows();
                repeaterId.model = imageHandler.getNumberOfScreens();

            }
        }

        C.PushButton {
            id: settingsButton
            text: "Settings"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                showSettings();
            }
        }

        C.PushButton {
            id: leaveSessionButton
            text: "Leave Session"
            font.pixelSize: 32
            property var toggled: true
            onClicked: {
                //streamHandler.stopRecording();
                sessionHandler.leaveSession();
                changePage("home");
            }
        }
    }

    Component.onCompleted: {
        //audioInputModel.getAudioTypes()

        if(!backendSettings.getAudioOn())
        {
            muteAudioButton.toggled = false;
            muteAudioButton.state = "red";
        }
        if(!backendSettings.getVideoOn())
        {
            muteVideoButton.toggled = false;
            muteVideoButton.state = "red";
        }
    }
}
