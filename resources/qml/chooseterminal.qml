import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1

Rectangle {
    id: terminals

    signal terminalSelected(int index)
    signal dialogOK()
    width: 300
    height: 215

    Rectangle {
        Layout.alignment: Qt.AlignTop
        anchors.fill: parent

        TableView {
            id: list
            anchors.fill: parent
            model: terminalModel
            visible: true

            TableViewColumn{
                role: "terminal"
                title: "Terminal"
                width: 150
                resizable: false
                movable: false
            }

            /*headerDelegate: Item {
                Text {
                    text: styleData.value
                    anchors.centerIn:parent
                    color:"#333"
                }
                }*/

            itemDelegate: Item {
                Text {
                    renderType: Text.NativeRendering
                    text: "  " + styleData.value
                }
            }

            backgroundVisible: true
            alternatingRowColors: true
            headerVisible: true
            focus: true

            onActivated: {
                terminals.terminalSelected(row)
            }

            onClicked: {
                terminals.terminalSelected(row)
            }

            onDoubleClicked: {
                terminals.terminalSelected(row)
                terminals.dialogOK()
            }
        }
    }

    function resetIndex(initialTerminalIndex){
        list.selection.clear()
        list.selection.select(initialTerminalIndex)
        terminals.terminalSelected(initialTerminalIndex)
    }
}
