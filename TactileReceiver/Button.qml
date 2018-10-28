import QtQuick 2.0

Item {
    Rectangle {
        id: rectangle
        x: 0
        y: 0
        width: 180
        height: 70
        color: "#89d474"
        radius: 20

        Text {
            id: text1
            x: 28
            y: 11
            width: 124
            height: 48
            color: "#ffffff"
            text: qsTr("START")
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 32
        }
    }

}
