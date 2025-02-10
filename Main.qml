import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import BallsModel 0.1

Window {
	width: 600
	height: 900
	visible: true
	title: qsTr("Color Lines")
	Rectangle {
		Popup {
			id: gameOverPopup
			visible: false
			dim: true
			modal: true
			Overlay.modal: Rectangle {
				color: "#EE444444"
			}
			background: Rectangle {
				color: "transparent"
			}
			closePolicy: Popup.NoAutoClose
			implicitWidth: parent.width / 3 * 2
			implicitHeight: parent.height / 3 * 2
			anchors.centerIn: parent
			ColumnLayout {
				anchors.fill: parent

				Text {
					Layout.preferredHeight: parent.height / 3
					Layout.fillWidth: true
					text: "GAME OVER"
					color: "#00ADA9"
					font.pixelSize:  parent.height / 3 / 4
					font.bold: true
					verticalAlignment: Qt.AlignVCenter
					horizontalAlignment: Qt.AlignHCenter
				}
				Label {
					Layout.preferredHeight: parent.height / 5
					Layout.fillWidth: true
					verticalAlignment: Qt.AlignVCenter
					horizontalAlignment: Qt.AlignHCenter
					text: "NEW GAME"
					font.pixelSize: parent.height / 5 / 3
					font.bold: true
					color: "white"
					background: Rectangle {
						radius: 20
						color: "#00ADA9"
					}

					width: height * 2
					MouseArea {
						anchors.fill: parent
						onClicked: {

							mod.newGame(false)
							gameOverPopup.close()
						}
					}
				}
			}
		}
		id: back
		anchors.fill: parent
		color: "#474143"
		ColumnLayout {
			id: lay
			anchors.fill: parent
			spacing: 0
			Rectangle {
				id: upper
				Layout.preferredHeight: back.height * 0.1
				Layout.fillWidth: true
				color: Qt.lighter("#474143")
				Text {
					id: score
					anchors.centerIn: parent
					font.pixelSize: Math.floor(upper.height / 2)

					font.bold: true
					color: "#00ADA9"
					Timer {
						id: scoreTimer
						interval: 100
						repeat: true
						onTriggered: {
							if (mod.score == 0) {
								score.text = mod.score
							} else if (Number(score.text) < mod.score)
								score.text = Number(score.text) + 1
							else
								stop()
						}
					}

					Component.onCompleted: {
						score.text = mod.score
					}
				}
				Label {
					id: newGameBtn
					anchors.right: parent.right
					anchors.margins: 10
					anchors.verticalCenter: parent.verticalCenter
					height: parent.height / 2
					verticalAlignment: Qt.AlignVCenter
					horizontalAlignment: Qt.AlignHCenter
					text: "NEW GAME"
					font.pixelSize: height / 3
					color: "#00ADA9"
					background: Rectangle {
						radius: 20
						color: "#474143"
					}

					width: height * 2
					MouseArea {
						anchors.fill: parent
						onClicked: mod.newGame(false)
					}
				}
			}
			Rectangle {
				id: body
				Layout.preferredHeight: Math.min(lay.height * 0.8,
												 lay.width - 10)
				Layout.preferredWidth: Math.min(lay.height * 0.8,
												lay.width - 10)
				Layout.margins: 5
				Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
				color: "#474143"
				radius: 10
				TableView {
					id: view
					anchors.fill: parent
					anchors.centerIn: parent
					anchors.margins: 5
					transformOrigin: Item.Center
					columnSpacing: 5
					rowSpacing: 5
					model: BallsModel {
						id: mod
						onMoveBall: function (color, x, y, nX, nY) {
							fake.oX = x
							fake.oY = y
							fake.nX = nX
							fake.nY = nY
							fake.color = color
							fake.visible = true
						}
						onScoreChanged: {
							scoreTimer.start()
						}
						onGameOver: {
							gameOverPopup.open()
						}
					}
					Rectangle {
						property int oX
						property int oY
						property int nX
						property int nY
						z: 2
						id: fake
						visible: false
						x: oX
						y: oY
						implicitHeight: (view.width - view.columnSpacing * 8) / 9 - 2
						implicitWidth: implicitHeight
						anchors.margins: 1
						radius: height
						Timer {
							id: fakeTimer
							interval: 200
							onTriggered: fake.visible = false
						}

						NumberAnimation on x {
							running: fake.visible
							from: fake.oX
							to: fake.nX + 1
							duration: 200
							easing.type: Easing.InOutCubic
							onStopped: {
								mod.setBallVisible()
								fakeTimer.start()
							}
						}
						NumberAnimation on y {

							running: fake.visible
							from: fake.oY
							to: fake.nY + 1
							duration: 200
							easing.type: Easing.InOutCubic
							onStopped: {
								mod.setBallVisible()
								fakeTimer.start()
							}
						}
					}

					delegate: Item {
						required property int bInd
						required property bool bSel
						required property bool bVis
						required property color bColor

						id: del
						implicitHeight: (view.width - view.columnSpacing * 8) / 9
						implicitWidth: implicitHeight
						Rectangle {
							id: empty
							visible: full.scale == 0
							anchors.fill: parent
							anchors.margins: 1
							radius: height
							color: "white"
							Rectangle {
								anchors.centerIn: parent
								anchors.fill: parent
								anchors.margins: 1
								radius: height
								color: "#474143"
							}
						}

						Rectangle {
							id: full
							anchors.fill: parent
							anchors.margins: 1
							radius: height
							scale: del.bVis ? 1 : 0
							color: del.bColor
							Behavior on scale {
								enabled: !(fake.visible && fake.x == del.x)
								NumberAnimation {
									duration: 200
									easing.type: Easing.InOutQuad
								}
							}

							Rectangle {
								visible: del.bSel
								z: -1
								anchors.fill: parent
								anchors.margins: -3
								radius: height
								onVisibleChanged: {
									if (visible)
										selAnim.start()
									else
										selAnim.stop()
								}

								ColorAnimation on color {
									id: selAnim

									loops: Animation.Infinite
									from: "white"
									to: "darkgray"
									duration: 1000
									easing.type: Easing.InOutCubic
								}
							}
						}
						MouseArea {
							anchors.fill: parent
							onClicked: {
								mod.checkTurn(del.bInd, del.x, del.y,
											  del.bColor)
							}
						}
					}
				}
			}
		}
	}
}
