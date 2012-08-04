import QtQuick 1.0

Rectangle {
    id: window
    objectName: "base"
    width: 300
    height: 600
    color: "#000000"
    opacity: 0.6

    Component {
        id: feeddelegate
        
        

        Item {
            id: delegate
            height: column.height + 40
            width: delegate.ListView.view.width
            
            

            MouseArea {
                anchors.fill: parent
                onClicked:{
                    console.log("item clicked is ")
                    console.log(index);
                    categories.guidesClicked(index);
                    categories.opacity=0;
                    slidesView.opacity=1;
                    backbutton.opacity=1;
                    addnewguides.opacity=0;
                }
            }

            Row {
                id:drow
                width: parent.width
                height: parent.height

                Image {
                    id: dimage
                    width: 50
                    height: 50
                    source: model.thumbimage
                }


                Column {
                    id: column
                    x: 20; y: 20
                    width: parent.width - 40

                    Text {
                        id: title
                        text: model.title; width: parent.width; wrapMode: Text.WordWrap
                        font { bold: true; family: "Helvetica"; pointSize: 10 }
                    }

                }
            }


            Rectangle {
                width: parent.width; height: 2; color: "#000000"
                anchors.bottom: feeddelegate.bottom
            }

        }

    }

    Component{
        id:slidesdelegate

        Item {
            id: sdelegate
            height: scolumn.height +10
            width: sdelegate.ListView.view.width

            Column{
                id:scolumn
                width: parent.width-40;
                
                Text {
                    id: description
                    text: model.title
                }

                Rectangle {
                    id:viewimages
                    height: 20
                    width: 70
                    color: "#000000"

                    Text {
                        id: backtext
                        text: "View Images"
                        color: "#ffffff"
                    }

                    MouseArea {
                        id:view
                        anchors.fill: parent
                        onClicked: {
                            slidesView.viewImagesClicked(index);
                        }
                    }

                }


            }

            Rectangle {
                width: parent.width; height: 2; color: "#000000"
                anchors.bottom: slidesdelegate.bottom
            }

        }
    }

    ListModel {
             id: appModel
             ListElement { name: "Music"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Movies"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Camera"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Calendar"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Messaging"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Todo List"; icon: "/home/rmr/images.jpeg" }
             ListElement { name: "Contacts"; icon: "/home/rmr/images.jpeg" }
         }

    Component {
             id: appDelegate

             Item {
                 width: 100; height: 100

                 Image {
                     id: myIcon
                     y: 20; anchors.horizontalCenter: parent.horizontalCenter
                     source: icon
                 }
                 Text {
                     anchors { top: myIcon.bottom; horizontalCenter: parent.horizontalCenter }
                     text: name
                 }
             }
         }
    ListView {

        id: categories
        objectName: "guidesList"
        y:40
        width: parent.width
        height: parent.height-40
        model: feedModel
        delegate: feeddelegate
        clip: true
        opacity: 1
        
        signal guidesClicked(int index);

    }



    Rectangle {
        id:backbutton
        height: 20
        width: 40
        color: "#000000"
        x:10
        y:10
        opacity: 0

        Text {
            id: backtext
            text: "Back"
            color: "#ffffff"
        }

        MouseArea {
            id:backbuttonclick
            anchors.fill: parent
            onClicked: {
                slidesView.backButtonClicked();
                slidesView.opacity=0;
                categories.opacity=1;
                backbutton.opacity=0;
                addnewguides.opacity=1;
            }
        }
    }

    Rectangle {
        id:addnewguides
        height: 20
        width: 40
        color: "#000000"
        x:10
        y:10
        opacity: 1

        Text {
            id: addnew
            text: "New"
            color: "#ffffff"
        }

        MouseArea {
            id:addnewbutton
            anchors.fill: parent
            onClicked: {

            }
        }
    }

    ListView {

        id: slidesView
        objectName: "slideslist"

        y:50
        height: parent.height-50
        width: parent.width
        model: slidesmodel
        delegate: slidesdelegate
        clip: true
        opacity: 0

        signal backButtonClicked();
        signal viewImagesClicked(int sindex);
    }
}



