import QtQuick 2.0

Rectangle {
    id: terminals
    width: 180; height: 200

    ListView {
        id: list
        anchors.fill: parent
        model: terminalModel
        delegate: Component {
            id: contactDelegate
            Item {
                width: 180; height: 40
                Column {
                    Text { text: '<b>Terminal:</b> ' + modelData }
                }

                MouseArea{
                    id: itemMouseArea
                    anchors.fill: parent
                    onClicked: {
                        list.currentIndex = index
                    }
                }
            }
        }        

        highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        highlightRangeMode: "StrictlyEnforceRange"
        highlightFollowsCurrentItem: true
        focus: true
    }
}
