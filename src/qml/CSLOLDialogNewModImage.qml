//新建皮肤图片对话框
import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.15
import Qt.labs.platform 1.0

FileDialog {
    visible: false
    title: qsTr("Select image file")
    fileMode: FileDialog.OpenFile
    nameFilters: "PNG Image file (*.png)"
}
