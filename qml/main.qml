import QtQuick 2.9
import QtQuick.Controls 2.3

ApplicationWindow {
        menuBar: MenuBar {

            Menu {
            title: "File"
            Action {
                text: "Power On"
                onTriggered: mymainWindow.on_actionPower_ON_triggered();
            }
            Action {
                text: "New"
                onTriggered: mymainWindow.on_actionNew_triggered();
            }
            Action {
                text: "Open"
                onTriggered: mymainWindow.on_actionOpen_triggered();
            }
            Action {
                text: "Save"
                onTriggered: mymainWindow.on_actionSave_triggered();
            }
            Action {
                text: "Save As"
                onTriggered: mymainWindow.on_actionSaveAs_triggered();
            }
            MenuSeparator {

            }
            Action {
                text: "Exit"
                onTriggered: mymainWindow.on_actionExit_triggered();
            }
        }
        Menu {
            title: "Stack"
            Action {
                text: "Copy Level 1"
                onTriggered: mymainWindow.on_actionCopy_Level_1_triggered();
            }
            Action {
                text: "Cut Level 1"
                onTriggered: mymainWindow.on_actionCut_Level_1_triggered();
            }
            Action {
                text: "Paste to Level 1"
                onTriggered: mymainWindow.on_actionPaste_to_Level_1_triggered();
            }
            Action {
                text: "Save Level 1 As..."
                onTriggered: mymainWindow.on_actionSave_Level_1_As_triggered();
            }
            Action {
                text: "Open file to Level 1"
                onTriggered: mymainWindow.on_actionOpen_file_to_Level_1_triggered();
            }



        }

        Menu {
            title: "Hardware"
            Action {
                text: "Connect to calc"
                onTriggered: mymainWindow.on_actionConnect_to_calc_triggered();
            }
            Action {
                text: "USB Remote ARCHIVE to File"
                onTriggered: mymainWindow.on_actionUSB_Remote_ARCHIVE_to_file_triggered();
            }
            Action {
                text: "Remote USBRESTORE from file"
                onTriggered: mymainWindow.on_actionRemote_USBRESTORE_from_file_triggered();
            }
            MenuSeparator {

            }

            Action {
                text: "Insert SD Card image"
                onTriggered: mymainWindow.on_actionInsert_SD_Card_Image_triggered();
            }
            Action {
                text: "Eject SD Card image"
                onTriggered: mymainWindow.on_actionEject_SD_Card_Image_triggered();
            }

            MenuSeparator {

            }

            Action {
                text: "Simulate alarm"
                onTriggered: mymainWindow.on_actionInsert_SD_Card_Image_triggered();
            }
            Action {
                text: "Take Screenshot"
                onTriggered: mymainWindow.on_actionTake_Screenshot_triggered();
            }
            MenuSeparator {

            }

            Action {
                text: "Show LCD grid"
                checkable: true
                checked: false
                onCheckedChanged: mymainWindow.on_actionShow_LCD_grid_toggled(checked);
            }


        }
    }
    }
