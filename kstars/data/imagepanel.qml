import QtQuick 1.0

Rectangle {
    id:baserect
    width: 300
    height: 300



    Rectangle {
        id: closebutton
        objectName: "closebutton"
        x:250
        y:5
        height: 30
        width: 50
        color: "#000000"

        signal closeButtonClicked();

        Text {
            id: close
            text: "Close"
            color: "#ffffff"
        }
        MouseArea {
            id:closearea
            anchors.fill: parent
            onClicked: {
                console.log("close clicked..")
                closebutton.closeButtonClicked();
            }
        }

    }

    Image {
        id: simage
        anchors.centerIn: parent
        source: trialImage
    }

}
